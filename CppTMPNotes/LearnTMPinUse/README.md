# 3. Learn TMP in Use
通过几个例子，来进一步了解TMP，以及了解什么是Metafunction Convention。这些例子大部分来自标准库`<type_traits>`。

## 3.1 Example 1: Type Manipulation

### 3.1.1 is_reference

判定一个类型是不是引用类型的模板:

```cpp
template <typename T> struct is_reference { static constexpr bool value = false; };     // #1
template <typename T> struct is_reference<T&> { static constexpr bool value = true; };  // #2
template <typename T> struct is_reference<T&&> { static constexpr bool value = true; }; // #3

std::cout<< is_reference<int>::value << std::endl;
std::cout<< is_reference<int&>::value << std::endl;
std::cout<< is_reference<int&&>::value << std::endl;
```

is_reference包含一个主模板和两个偏特化，接受一个类型T作为参数。当T传入一个右值引用类型时，编译器会选择#3这个模板特化来进行实例化；当T传入一个左值引用类型时，编译器会选择#2这个模板特化来进行实例化；当T不是引用类型时，#2和#3都不匹配，编译器选择主模板#1来进行实例化。

### 3.1.2 remove_reference
除了判定一个引用类型外，我们还可以移除一个类型的引用:

```cpp
template <typename T> struct remove_reference { using type = T; };      // #1
template <typename T> struct remove_reference<T&> { using type = T; };     // #2
template <typename T> struct remove_reference<T&&> { using type = T; };    // #3

// case 1:
int&& i = 0;
remove_reference<decltype(i)>::type j = i;

// case 2:
template <typename T>
void foo(typename remove_reference<T>::type a_copy) { a_copy += 1; }

foo<int>(i);    // passed by value
foo<int&&>(i);  // passed by value
```

同样的一个主模板和两个偏特化，同样的匹配规则。TMP工作在编译期，在编译期没有变量，只有常量和类型，这里的移除引用就是把一个引用类型变成对应的非引用类型。

另一个问题，"case 2"中为什么remove_reference的前面要加一个"typename"关键字？是因为`remove_reference<T>::type`是一个待决名(Dependent Name)，编译器在语法分析的时候还不知道这个名字到底代表什么。对于普通的名字，编译器直接通过名字查找就能直到这个名字的词性。但对于待决名，因为它是什么取决于模板的实参T，所以直到编译器在语义分析阶段对模板进行实例化之后，才能对"type"进行名字查找，直到它到底是什么东西，所以名字查找是分两个阶段的，待决名直到第二个阶段才能被查找。但是在语法分析阶段，编译器就需要判定这个语句是否合法，所以需要我们显式的告诉编译器"type"是什么。在`remove_reference<T>::type`这个语法中，type有三种可能，一是静态成员变量或函数，二是一个类型，三是一个成员模板。编译器要求对类型要用`typename`关键字修饰，对于模板要用`template`关键字修饰，以便完成语法分析的工作。

## 3.2 Metafunction Convention
通过这两个例子总结一些元编程的通用原则，称之为Metafunction Convention。

### 3.2.1 Metafunction always return a "type"
程序是逻辑和数据的集合。`is_reference`和`remove_reference`是两个类模板，但是在TMP中，他们接受实参，返回结果，像函数一样被使用。我们称这种在编译器"调用"的特殊"函数"为Metafunction，它代表了TMP中的"逻辑"。Metafcuntion接受常量和类型作为参数，返回常量或类型作为结果，我们称这些常量和类型为Metadata，它代表了TMP中的"数据"。进一步的，我们称常量为Non-type Metadata(or Numerical Metadata)，称类型为Type Metadata。

但在上面的例子中看到，is_reference的返回值名为"value"，remove_reference的返回值名为"type"，为了形式化上的一致性，Metafunction Convention规定，所有的Metafunction都以"type"作为唯一的返回值，对于原本以"value"指代的那些常量，使用一个类模板将它们封装起来，Metafunction返回这个类模板的相应实例。例如:

```cpp
// non-type metadata
template <bool b>
struct bool_ { static constexpr bool value = b; };

// metafunction
template <typename T> struct is_reference { using type = bool_<false>; };
template <typename T&> struct is_reference { using type = bool_<true>; };
template <typename T&&> struct is_reference { using type = bool_<true>; };
```

我们定义了一个名为`bool_`的类模板，以封装`bool`类型的常量。在调用is_reference时，也是使用"type"这个名字，如果想访问结果中的值，使用`is_reference<T>::type::value`即可。

注意，Metafunction Convention的这种规定，并不是C++语言上的要求，而是编程指导上的要求，目的是规范元编程的代码，使其更具可读性和兼容性。便准库，boost，github上热门的TMP库都遵循这一约定，你也应该遵守。

### 3.2.2 integral_const
在真实世界的场景中，一个典型的Non-type Metadata是这样定义的:

```cpp
template <typename T, T v>
struct integral_constant {
    static constexpr T value = v;
    using value_type = T;
    using type = integral_constant;
    constexpr operator value_type() const noexcept { return value; }
    constexpr value_type operator()() const noexcept { return value; }
};
```

这是在TMP中最常用的一个Non-type Metadata，它除了像bool_那样定义了一个value外，还定义了:
- value_type指代数据的类型
- type指代自身，即integral_constant，这个成员使integral_constant变成了一个返回自己的Metafunction
- operator value_type()是到value_type的隐式类型转换，返回value的值
- value_type operator()是函数调用运算符重载，返回value的值

这些成员，特别是type，都会使TMP变得更方便，通常我们在使用时还会定义一些alias:

```cpp
// alias
template <bool B> using bool_constant = integral_constant<bool, B>;
using true_type = bool_constant<true>;
using false_type = bool_constant<false>;
```

有了这些定义，is_reference的定义就变成了:

```cpp
template <typename T> struct is_reference { using type = false_type; };
template <typename T> struct is_reference<T&> { using type = true_type; };
template <typename T> struct is_reference<T&&> { using type = true_type; };
```

对它的调用就变成了:

```cpp
std::cout << is_reference<int>::type::value;
std::cout << is_reference<int>::type();
std::cout << is_reference<int>::type()();
```

### 3.2.3 use public inheritance
当一个Metafunction使用另一个Metafunction的结果作为返回值时，不用自己定义type成员，只需要直接继承另一个Metafunction即可。例如:

```cpp
template <typename T> struct is_reference : public false_type {};
template <typename T> struct is_reference<T&> : public true_type {};
template <typename T> struct is_reference<T&&> : public true_type {};
```

由于true_type和false_type内部定义了一个名为"type"的成员，而且这个成员值得是它们自己，所以直接继承过来，is_reference内部也有了名为"type"的成员了。我们可以实现一个新的Metafunction，判定一个类型是不是int或引用类型:

```cpp
template <typename T> struct is_int_or_reference : public is_reference<T> {};
template <> struct is_int_or_reference<int> : public true_type {};

std::cout << is_int_or_reference<int>::value;std::cout << is_int_or_reference<int>();
std::cout << is_int_or_reference<int>()();
```

公有继承和直接定义"type"成员两种方式效果类似。我们在TMP中会尽可能的使用这种继承的方式，而不是每次都去定义type。因为这种方式实现的代码更简洁，也更具有一致性。

### 3.2.4 useful aliases
为了方便，我们通常还会创建两个东西来简化Metafunction的调用。

一. 对于返回非类型常量的Metafunction，我们定义一个`_v`后缀的变量模板(Variable Template)，通过它可以方便的获取Metafunction返回的value:

```cpp
template <typename T> inline constexpr bool is_reference_v = is_reference<T>::value;
```

二. 对于返回一个类型的Metafunction，我们声明一个`_t`后缀的别名模板(Alias Template)，通过它可以方便的获取Metafunction返回的type:

```cpp
template <typename T> using remove_reference_t = typename remove_reference<T>::type;
```

## 3.3 Example 2: Metafunction with Multiple Arguments
下面来看一看多个形参的例子。

### 3.3.1 is_same
假如我们想判定两个类型是否相等，并以此来写一些逻辑。在C++中，由于缺乏自省机制，所以普通的代码是不能实现下面这种效果的:

```cpp
int i = 0;
std::cout << is_same_v<decltype(i), int> << std::endl;std::cout << is_same_v<decltype(i), float> << std::endl;

if (is_same_v<decltype(i), int>){
    // do something...
} else{
    // do something...
}
```

这里可能会想到RTTI(Run-Time Type Information)的机制，但RTTI不同编译器的实现可能有差别，它的本意是为了实现C++内部的一些语言机制，主要是动态多态(Dynamic Polymorphism)，因此依赖RTTI的代码可能不具备可移植性。但是通过TMP，我们可以实现一个Metafunction来达到判定类型的效果，原理非常简单:

```cpp
template <typenmae T, typename U>
struct is_same : false_type {};

template <typename T>
strcut is_same<T, T> : true_type {};
```

is_same是一个类模板，有两个模板形参T和U，主模板继承了false_type，另外有一个特化继承了true_type，这个特化模板匹配T和U相同的情况。当T和U相同时，编译器将T和U带入模板特化的实参列表里，然后尝试推导特化模板的形参，因为两个参数相同，，所以推导得出一致的结果，匹配特化成功，`is_same<T, U>::value == true`。当T和U不同时，推导失败，匹配到主模板，此时`is_same<T, U>::value == false`。

### 3.3.2 is_one_of
这个例子展示变长形参，我们将is_same推广一下，给定一个类型T，和一堆类型列表，判定T是否包含在这个列表之中。

```cpp
template <typename T, typename U, typename... Rest>
struct is_one_of : bool_constant<is_one_of<T, U>::value || is_one_of<T,Rest...>::value> {};

template <typename T, typename U>
struct is_one_of<T, U> : is_same<T, U> {};

int i = 0;
std::cout << is_one_of_v<decltype(i), float, double> << std::endl;
std::cout << is_one_of_v<decltype(i), float, int, double, char> << std::endl;
```

is_one_of的主模板形参分为三个部分: 类型T，类型U，变长形参列表(Parameter Pack)Rest，另有一个特化模板，只接受两个参数T和U，这个特化的逻辑等效于is_same。主模板的递归逻辑是一个析取表达式，判定T和U是否相等，或者判定T是不是包含在剩余的参数中。这里的`Rest...`是对变长形参列表的展开，当我们要引用一个变长形参列表内的内容时，就需要这样写。这个递归会一直持续遍历所有的参数，直到只剩下T和最后一个参数，这时匹配模板特化，递归终止。

需要注意的是，这个析取表达式的求值与运行时不同，运行时的析取表达式遵循"短路求值"的规则，对`a || b`如果a为true，就不再对b求值了。但在编译期，在模板实例化的时候，析取表达式的短路求值是不生效的。

另外，有一个元编程的小技巧是，有时我们可以通过一个简单的别名来实现一个新的Metafunction。例如:

```cpp
template <typename T>
using is_integral = is_one_of<T, bool, char, short, int, long, long long>;
```

### 3.3.3 is_instantiation_of
这个例子展示模板模板形参。is_instantiation_of接受两个参数，一个类型，一个模板，它可以判定这个类型是不是这个模板的实例类型。

```cpp
std::list<int> li;
std::vector<int> vi;
std::vector<float> vf;

std::cout << is_instantiation_of_v<decltype(vi), std::vector> << std::endl;
std::cout << is_instantiation_of_v<decltype(vf), std::vector> << std::endl;
std::cout << is_instantiation_of_v<decltype(li), std::vector> << std::endl;
std::cout << is_instantiation_of_v<decltype(li), std::list> << std::endl;

template <typename Inst, template <typename...> typename Tmpl>
struct is_instantiation_of : false_type {};

template <template <typename...> typename Tmpl, typename... Args>
struct is_instantiation_of<Tmpl<Args...>, Tmpl> : true_type {};
```

is_instantiation_of也有一个主模板和一个偏特化，主模板继承false_type，它匹配当这个类型不是模板的实例时的情况；特化模板继承自true_type，它匹配当传入的类型是对应模板的实例时的情况。

### 3.3.4 conditional
我们已经看到了很多通过类模板特化来实现选择逻辑的例子，我们可以实现一个通用的选择，就像是`if`语句那样。`conditional`就是编译期的`if`: 

```cpp
template <bool B, typename T, typename F>
struct conditional : type_identity<T> {};

template <typename T, typename F>
struct conditional<false, T, F> : type_identity<F> {};
```

当B是false时，匹配模板的特化，返回F；当B是true时，匹配主模板，返回T。通过conditional，我们可以实现一个类似"短路求值"的效果。例如我们用conditional来实现is_one_of:

```cpp
template <typename T, typename U, typename... Rest>
struct is_one_of : conditional_t<is_same_v<T, U>, true_type, is_one_of<T, Rest...>> {};

template <typename T, typename U>
struct is_one_of<T, U> : conditional_t<is_same_v<T, U>, true_type, false_type> {};
```

每一次递归前，我们先判定T和剩余所有参数中的第一个U是否相等，如果相等，就直接返回true_type，不会再向下递归。这个技巧有时可以用来优化编译时长。

## 3.4 Example 3: Deal with Arrays
最后一组例子，我们来看看在TMP中式怎么处理数组类型的。

### 3.4.1 rank
rank返回数组的维度。

```cpp
std::cout << rank_v<int> << std::endl;
std::cout << rank_v<int[5]> << std::endl;
std::cout << rank_v<int[5][5]> << std::endl;
std::cout << rank_v<int[][5][6]> << std::endl;

template <typename T>
struct rank : integral_constant<std::size_t, 0> {};     // #1

template <typename T>
struct rank<T[]> : integral_constant<std:size_t, rank<T>::value + 1> {};    // #2

template <typename T, std::size_t N>
struct rank<T[N]> : integral_constant<std::size_t, rank<T>::value + 1> {};  // #3
```

rank包含一个主模板和两个偏特化。根据模板特化的匹配规则，当模板实参式数组类型时，会匹配#2或#3这两个特化，当实参式非数组类型时，匹配主模板。其中，#2匹配不定长数组；#3匹配定长数组。整个递归的过程是对维度做递归，每次递归value + 1，就可以得到总维度。

### 3.4.2 extent
extent接受两个参数，一个数组T和一个值N，返回T的第N维大小。

```cpp
std::cout << extent_v<int[3]> << std::endl;
std::cout << extent_v<int[3][4], 0> << std::endl;
std::cout << extent_v<int[3][4], 1> << std::endl;
std::cout << extent_v<int[3][4], 2> << std::endl;
std::cout << extent_v<int[]> << std::endl;

template <typename T, unsigned N = 0>
struct extent : integral_constant<std::size_t, 0> {};

template <typename T>
struct extent<T[], 0> : integral_constant<std::size_t, 0> {};

template <typename T, unsigned N>
struct extent<T[], N> : extent<T, N-1> {};

template <typename T, std::size_t I>
struct extent<T[I], 0> : integral_constant<std::size_t, I> {};

template <typename T, std::size_t I, unsigned N>
struct extent<T[I], N> : entent<T, N-1> {};
```

extent共有4个偏特化，前两个匹配不定长数组，后两个匹配定长数组，主模板匹配非数组类型。
