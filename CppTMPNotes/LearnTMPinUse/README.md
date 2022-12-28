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
template <typename T&> struct remove_reference { using type = T; };     // #2
template <typename T&&> struct remove_reference { using type = T; };    // #3

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