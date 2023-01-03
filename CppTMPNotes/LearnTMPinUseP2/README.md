# 5. Learn TMP in Use Part 2

## 5.1 Example 4: SFINAE
本部分展示两个基于SFINAE的例子，enable_if和void_t。

### 5.1.1 enable_if
假设我们想要这样的函数模板重载，对于整型的参数，匹配第一个模板，实现针对整型参数的逻辑；对于浮点型的参数，匹配第二个模板，实现针对浮点型的参数。于是定义了下面两个函数模板重载:

```cpp
template <typename INT> void foo(INT) {};

template <typename FLT> void foo(FLT) {};
```

显然，上面两个模板实际上是一模一样的，编译器会报告重定义错误，因为它们的签名完全相同。我们无法写一个针对参数类型的函数模板重载，唯一的办法是对每个类型都写一个不同函数重载，但这样要定义十几个相似的函数，完全失去了泛型编程的支持。这时，就需要`enable_if`解决该问题:

```cpp
template <bool, typename T = void>
struct enable_if : type_identity<T> {};

template <typename T>
struct enable_if<false, T> {};

template <typename T> enable_if_t<is_integral_v<T>> foo(T) {};  // #1

template <typename T> enable_if_t<is_floating_point_v<T>> foo(T) {};  //#2

foo(1);
foo(1.0f);
```

我们先来看看`enable_if`是如何定义的。首先它是一个类模板，主模板有两个形参，第一个形参接受一个bool类型，第二个形参接受一个类型T且有一个默认值void，主模板就返回T本身。另外有一个偏特化，偏特化接受一个类型形参T，匹配bool为false的情况.但注意，它内部不存在"type"的定义，当我们调用`enable_if<bool, T>::type`时，如果bool为true，那么enable_if就返回T；如果bool为false，那么不能返回任何东西。"type"不存在，对它的调用就构成了一个非良构的表达式，只要这个表达式位于立即上下文中，那么就会触发SFINAE机制。

所以，对于`foo(1)`，在函数模板#2中，T被推导为int，`is_floating_point_v<T>`为false，所以`enable_if<is_floating_point_v<T>>`产生了非良构的实例化表达式。它位于函数模板foo的返回值处，是一个立即上下文，所以这是要给替换失败，函数模板#2从重载集中剔除，避免了重定义错误。`foo(1.0f)`也是同理。

所以，通过`enable_if`(准确的说是通过SFINAE)，我们拥有了基于逻辑来控制函数重载集的能力。

来看看第二个例子，假设我们想通过类模板的形参来控制类模板成员函数的重载:

```cpp
template <typename T> struct S{
    template <typename U> static enable_if_t<is_same_v<T, int>> foo(U) {};  // #1
    template <typename U> static enable_if_t<!is_same_v<T, int>> foo(U) {};   // #2
};

S<int>::foo(1);
```

在这里，foo是S的静态成员函数模板，我们希望当S的实参是int时，匹配第一个foo；当S的实参不是int时，匹配第二个foo。也就是说，S<int>::foo(1)应该调用#1。但实际上，编译这段代码时，编译器会报错。在实例化`S<int>`的时候，`enable_if<false>`里面没有"type"类型，这是我们预期的替换失败，但不会发生SFINAE，而是报告了一个错误。因为enable_if不在类模板S的立即上下文中。注意，foo的返回值属于立即上下文，是对foo来说的。也就是说在实例化成员函数模板foo时，foo的返回值区域位于立即上下文中。而我们现在正在实例化S，只有在S自己的立即上下文内才能使用SFINAE。

所以我们要推迟`enable_if`的实例化到foo的立即上下文中执行。让`enable_if`的实例化发生在foo实例化的时候，而不是S实例化的时候:

```cpp
template <typename... Args>
struct always_true : true_type {};

template <typename T> struct S{
    template <typename U> static enable_if<always_true_v<U> && is_same_v<T, int>> foo(U) {};
    template <typename U> static enable_if<always_true_v<U> && !is_same_v<T, int>> foo(U) {};
};

S<int>::foo(1);
```

我们实现一个无意义的Metafunction，`always_true`接受任意参数，返回true。然后将`always_true_v<U>`放到enable_if的实参表达式里，因为`always_true`永远都是true，所以表达式逻辑不变。但问题解决了。

我们来解释一下原理，在实例化`S<int>`时，编译器用int替换T，第二个foo中的`!<is_same_v<T, int>`就变成了false。但对于这个表达式的剩余部分`always_true_v<U>`，其值依赖于模板foo的形参U，而这个U直到foo实例化的时候编译器才能确定它的值，所以编译器无法继续对enable_if_t求值，也就没有任何非良构的表达式产生。直到`foo(1)`开始实例化，U被推导为int，`always_true_v<U>`返回true。这时`enable_if_t<true && false>`就构成了非良构，但因为我们现在位于foo的立即上下文中，可以使用SFINAE。

### 5.1.2 void_t

void_t的定义如下:

```cpp
template <typename...> using void_t = void;
```

看起来是一个很简单的东西，就是一个别名模板，接受任意的类型参数，无论传给它什么，都返回一个void。

假设我们现在想实现这样一个功能，要检查一个Metafunction是否遵守了Metafunction Convention。也就是说，给一个任意的类型，我们检查它内部是否定义了一个"type":

```cpp
template <typename, typename = void> struct has_type_member : false_type {};

template <typename T> struct has_type_member<T, void_t<typename T::type>> " true_type {};

std::cout << has_type_member_v<int> << std::endl;std::cout << has_type_member_v<true_type> << std::endl;
std::cout << has_type_member_v<type_identity<int>> << std::endl;
```

我们定义了一个Metafunction叫做has_type_member，它的主模板接受两个参数，第一个参数是一个类型，第二个参数也是一个类型，但默认值设为void。另有一个偏特化，偏特化保留了第一个参数T，第二个参数期望匹配的是一个void类型，但没有直接写void，而是`void_t<typename T::type>`。

第一个关键点，先理解一下偏序规则是如何发挥作用的，什么情况下会匹配has_type_member的偏特化。对于第二个模板参数，当我们传入void时，显然这时会匹配偏特化；当我们给第二个参数传入非void时，由于偏特化不匹配非void类型，所以这时会匹配主模板。而重点是，当不传递第二个参数时，那么编译器会从该形参的默认实参来确定实参，而默认实参是void。也就是说，当不显式指定第二个实参时，模板的偏序规则会优先匹配偏特化。

第二个关键点，在偏特化的第二个特化形参里，并不是直接指定了void，而是指定了一个void_t的实例化表达式，这个表达式可能是非良构的。具体的说，当第一个形参T中包含名为"type"的成员，且"type"是一个类型时，表达式`typename T::type`良构；而当第一个形参T中不包含"type"或"type"不是一个类型时，表达式`typename T::type`非良构。而这个非良构发生在形参列表里，属于立即上下文，因此SFINAE发挥作用。也就是说，当T中不包含"type"类型时，偏特化被剔除，这次实例化只能匹配主模板。

所以，我们使用`has_type_member`时，只指定一个参数，不指定第二个参数。效果就是，当第一个参数里包含"type"成员时，匹配特化，结果为true；当第一个参数里不包含"type"成员时，匹配主模板，结果为false。

最后需要说明的是，`void_t`与`has_type_member`的实现与void没有直接关系，void不重要，重要的是第二个参数的默认实参要和`void_t`的返回类型匹配，从而触发偏序关系选择偏特化模板。只要遵守这个原理，可以把void换成其他类型，也能实现一样的效果。

## 5.2 Example 5: Unevaluated Expressions
在C++中有四个运算符，它们的操作数是不会被求值的，因为这四个运算符只是对操作数的编译期属性进行访问。这四个运算符分别是:`typeid,sizeof,noexcept,decltype`。

```cpp
std::size_t n = sizeof(std::cout << 42);
decltype(foo()) r = foo();
```

例如，第一句里的`std::cout << 42`并不会被执行，第二句中的foo也只调用了一次。也就是说这些运算符的操作数是不会在运行时生效的，甚至都不存在，在编译期就已经处理掉了。这些运算符的表达式称为不求值表达式(Unevaluated Expression)，它们的参数所处的区域称为不求值上下文(Unevaluated Context)。

### 5.2.1 declval
我们先来看不求值表达式的第一个用力，假如我们定义了两个函数模板重载:

```cpp
template <typename T> enable_if_t<is_integral_v<T>, int> foo(T) {}; // #1
template <typename T> enable_if_t<is_floating_point_v<T>, float> foo(T) {}; //#2
```

第一个模板匹配整型的实参T，接受一个T类型的变量作为函数参数，返回值类型为int；第二个模板匹配浮点型的实参T，接受一个T类型的变量作为函数参数，返回值类型为float。然后，我们定义一个类模板S，内部有一个成员value_，我们希望value_的类型是S的形参T对应的foo重载的返回值类型:

```cpp
template <typename T> struct S {decltype(foo<T>(??)) value_; };
```

为了得到foo的返回值类型，我们只需要写一个foo的调用表达式，然后将这个表达式传入`decltype`有运算符，就能得到foo的返回值类型。并且根据我们直到`decltype`是一个不求值运算符，foo的调用表达式不会被求值。但是问题在于，foo函数接受一个T类型的变量作为参数，我们去哪里创建一个T的变量出来呢？况且在编译期也没有变量。

虽然我们不能真正的在编译期创建一个变量，但在不求值表达式中，我们可以表示一个假想的变量出来，通过declval:

```cpp
template <typename T> add_rvalue_reference_t<T> declval() noexcept;
```

`declval`是一个函数模板，只有声明没有定义。因为不在求值上下问中，对这个模板的实例化不会被求值，我们不是真的要创建这个变量，只是要让编译器假设我们有一个这样的变量。所以`declval`是不能被用在需要求值的地方的，只能应用在不求值上下文中。另外，它的返回值类型是T的右值引用，`add_rvalue_reference`也是一个Metafunction，它接受T，返回T的右值引用。有了`declval`，我们就可以伪造一个变量传给foo了:

```cpp
template <typename T> strcut S { decltype(foo<T>(declval<T>())) value_; };

std::cout << is_same_v<int, decltype(S<char>.value_)> << std::endl;
std::cout << is_same_v<float, decltype(S<double>.value_)> << std::endl;
```

`declval<T>()`不在求值上下文中，就表示了一个T类型的变量。

### 5.2.2 add_lvalue_reference
结合不求值表达式和SFINAE，我们能实现更复杂的功能。现在实现两个Metafunction，它们给类型加上引用。add_lvalue_reference给类型加上左值引用，add_rvalue_reference给类型加上右值引用:

```cpp
template <typename T>
struct add_lvalue_reference : type_identity<T&> {};

template <typename T>
struct add_rvalue_reference : type_identity<T&&> {};
```

这两个Metafunction看起来很简单，但如果传入一个void会怎样？void是没有相应的引用类型的，如果T是void，那么会产生一个编译错误。如何解决该问题呢:

```cpp
namespace detail {
    
    template <typename T> type_identity<T&> try_add_lvalue_reference(int);
    template <typename T> type_identity<T> try_add_lvalue_reference(...);

    template <typename T> type_identity<T&> try_add_rvalue_reference(int);
    template <typename T> type_identity<T> try_add_rvalue_reference(...);
}

template <typename T>
struct add_lvalue_reference : decltype(detail::try_add_lvalue_reference<T>(0)) {};

template <typename T>
struct add_rvalue_reference : decltype(detail::try_add_rvalue_reference<T>(0)) {};

std::cout << is_same_v<char&, add_lvalue_reference<char>> << std::endl;
std::cout << is_same_v<void, add_lvalue_reference<void>> << std::endl;
```

我们来解释一下原理。对`add_lvalue_reference`，我们实现两个辅助函数模板`type_identity<T&> detail::try_add_lvalue_reference(int)`和`type_identity<T> detail::try_add_rvalue_reference(...)`。它们的返回值类型一个是`type_identity<T&>`，另一个是`type_identity<T>`。而这个返回值类型决定了`add_lvalue_reference`的结果，因为是直接继承过来的。而具体继承哪个，则取决于重载决议的结果。当T=void时，`type_identity<void&>`是一个非良构的表达式，根据SFINAE，第一个辅助模板被剔除，所以`add_lvalue_reference`实际继承的是第二个辅助函数模板的返回值，也就是`type_identity<void>`，返回`void`。而当T=char时，`type_identity<char&>`是良构表达式，两个函数模板都合法，那么根据偏序规则，由于第一个函数模板的函数形参类型特化程度更高，更匹配`detail::try_add_lvalue_reference<char>(0)`这个调用，所以第一个重载被选中，`add_lvalue_reference`返回`char&`。

可以看到，这种实现方式的思路是，不论T能否添加引用，我们先建设能，给它添上引用，如果错了就SFINAE。所以这种实现方法是更通用的，如果除了void外，还有其他类型也不能添加引用，这个实现也能覆盖到。总之思路就是:管它行不行，先试试，不行再说。

### 5.2.3 is_copy_assignable
同样的思路，我们可以实现一个Metafunction，来判断一个类型是不是可拷贝赋值的。类型T可以拷贝赋值的意思是，对两个T类型的变量a和b，我们可以写表达式: a = b。

参照上面的思路，想知道能不能写a = b，管它能不能写，先写出来再说:

```cpp
template <typename T>
using copy_assign_t = decltype(declval<T&>() = declval<T const&>());
```

上面decltype括号里就是一个赋值表达式，只不过是借助declval写出来的。对于一个赋值表达式，等号左边是一个`T&`类型的变量，等号右边是一个`T const&`类型的变量，并且赋值表达式的返回值类型等于等号左边的变量类型，也就是`T&`。如果赋值这句良构，那么copy_assign_t就等于`T&`；如果非良构，我们只要保证它在立即上下文里就行了，就可以SFINAE。所以:

```cpp
template <typename T, typename = void>
struct is_copy_assignable : false_type {};

template <typename T>
struct is_copy_assignable<T, void_t<copy_assign_t<T>>> : true_type {};

struct S { S& operator=(S const&) = delete; };
std::cout << is_copy_assignable_v<int> << std::endl;
std::cout << is_copy_assignable_v<true_type> << std::endl;
std::cout << is_copy_assignable_v<S> << std::endl;
```

我们把`copy_assign_t<T>`放到`void_t`里，剩下的就与`has_type_member`没有区别了。

在标准库头文件中还能找到更多相似的例子，可以去探索探索。

## 5.3 Eaxmple 6: Applications is Real World

### 5.3.1 std::tuple
`std::tuple`可以存储多个不同类型的值，那么不同类型的值是如何存储到一个列表的呢？

```cpp
int i = 1;
auto t = std::tuple(0, i, '2', 3.0f, 4ll, std::string("five"));

std::cout << std::get<1>(t) << std::endl;
std::cout << std::get<5>(t) << std::endl;
```

我们尝试实现一个自己的tuple，来展示它的实现原理:

```cpp
template <typename... Args>
struct tuple{
    tuple(Args...) {}
};

template <typename T, typename... Args>
struct tuple<T, Args...> : tuple<Args...>{
    tuple(T v, Args... params) : value_(v), tuple<Args...>(params...) {}

    T value_;
};
```

tuple的主模板什么都不做，只定义了一个构造函数，这个构造函数也没有任何逻辑，定义它是为了做类模板实参推导。tuple的模板特化实现了一个递归继承，`tuple<T, Args...>`继承了`tuple<Args...>`，这个递归继承会一直继承到`tuple<>`，这时匹配主模板，递归终止。特化模板定义了一个类型为T的成员`value_`，也就是说，在递归继承的每一层都存储了一个value，每一层的value的类型都可以是不同的，这就是tuple可以存储不同类型变量的关键。

了解了tuple的结构，我们再看下怎么读取tuple的元素，我们先来看下如何获取tuple中第N个元素的类型:

```cpp
template <usigned N, typename Tpl>
struct tuple_element;

template <unsigned N, typename T, typename... Args>
struct tuple_element<N, tuple<T, Args...>> : type_identity<T> {
    using __tuple_type = tuple<T, Args...>;
};
```

tuple_element对下标N做递归，直到N=0，这时T就是第N个元素的类型。但有一点特别的是，除了`tuple_element<>::type`以外，还定义了一个`tuple_element<>::__tuple_type`，它代表的是从N之后的参数组成的tuple类型。例如，`tuple_element<1, tuple<int, float, char>>::__tuple_type`就等于`tuple<float, char>`。

然后我们定义一个get函数，它直接通过一个类型转换就可以获得tuple的第N个元素。

```cpp
template <unsigned N, typename... Args>
tuple_element_t<N, tuple<Args...>>& get(tuple<Args...>& t){
    using __tuple_type = typename tuple_element<N, tuple<Args...>>:__tuple_type;
    return static_cast<__tuple_type&>(t).value_;
}
```

因为tuple是一层层继承的，所以这里对t相当于是一个向上转型，转型后直接返回这一层的value就行了。另外get返回的是value的左值引用，也就是说tuple中的元素是可以修改的。

```cpp
int i = 1;
auto t = std::tuple(0, i, '2', 3.0f, 4ll, std::string("five"));
std::cout << get<1>(t) << std::endl;
get<1>(t) = 0;
std::cout << get<1>(t) << std::endl;
```

### 5.3.2 A Universal json::dumps
接下来我们实现一个通用的json::dumps，它支持将嵌套的STL容器序列化为json字符串。

实现原理是使用函数模板重载的递归展开，并且通过SFINAE控制模板重载。

首先，对于内置的数值类型，直接转`string`返回，这里通过`is_one_of`判定参数类型是不是内置的数值类型:

```cpp
template <typename T>
std::enable_if_t<is_one_of_v<std::decay_t<T>, int, long, long long, unsigned, unsigned long, unsigned long long, float, double, long double>, std::string> dumps(const T& value){
    return std::to_string(value);
}
```

`std::decay`也是一个Metafunction，返回T的原始类型，无论T是引用类型，还是CV类型，都能保证用原始类型和后面的参数进行比较。通过`std::enable_if`，当dumps参数的类型不是这些数值类型时，这个模板从重载集中剔除。

然后对于`std::string`和其他的内置类型，可能要做一些特殊处理:

```cpp
// string, char
template <typename T>
std::enable_if_t<is_one_of_v<std::decay_t<T>, std::string, char>, std::string>
dumps(const T &obj) {
  std::stringstream ss;
  ss << '"' << obj << '"';
  return ss.str();
}

// char *
static inline std::string dumps(const char *s) {
  return json::dumps(std::string(s));
}

// void, nullptr
template <typename T>
std::
    enable_if_t<is_one_of_v<std::decay_t<T>, void, std::nullptr_t>, std::string>
    dumps(const T &) {
  return "null";
}

// bool
template <typename T>
std::enable_if_t<is_one_of_v<std::decay_t<T>, bool>, std::string>
dumps(const T &value) {
  return value ? "true" : "false";
}
```

下面，对于STL中的容器，我们要递归调用dumps。这里用到了`is_instantiation_of`来判定函数参数是不是容器实例:

```cpp
template <template <typename...> class Tmpl, typename... Args>
std::enable_if_t<
    is_instantiation_of_v<Tmpl<Args...>, std::vector> ||
    is_instantiation_of_v<Tmpl<Args...>, std::list> ||
    is_instantiation_of_v<Tmpl<Args...>, std::deque> ||
    is_instantiation_of_v<Tmpl<Args...>, std::forward_list> ||
    is_instantiation_of_v<Tmpl<Args...>, std::set> ||
    is_instantiation_of_v<Tmpl<Args...>, std::multiset> ||
    is_instantiation_of_v<Tmpl<Args...>, std::unordered_set> ||
    is_instantiation_of_v<Tmpl<Args...>, std::unordered_multiset>,
    std::string>
    dumps(const Tmpl<Args...>& obj) {
    std::stringstream ss;
    ss << "[";
    for (auto itr = obj.begin(); itr != obj.end();) {
        ss << dumps(*itr);
        if (++itr != obj.end()) ss << ", ";
    }
    ss << "]";
    return ss.str();
}

// map, multimap, unordered_map, unordered_multimap
template <template <typename...> class Tmpl, typename... Args>
std::enable_if_t<
    is_instantiation_of_v<Tmpl<Args...>, std::map> ||
    is_instantiation_of_v<Tmpl<Args...>, std::multimap> ||
    is_instantiation_of_v<Tmpl<Args...>, std::unordered_map> ||
    is_instantiation_of_v<Tmpl<Args...>, std::unordered_multimap>,
    std::string>
    dumps(const Tmpl<Args...>& obj) {
    std::stringstream ss;
    ss << "{";
    for (auto itr = obj.begin(); itr != obj.end();) {
        ss << dumps(itr->first);
        ss << ":";
        ss << dumps(itr->second);
        if (++itr != obj.end()) ss << ", ";
    }
    ss << "}";
    return ss.str();
}

// std::pair
template <typename T, typename U>
std::string dumps(const std::pair<T, U>& obj) {
    std::stringstream ss;
    ss << "{" << dumps(obj.first) << ":" << dumps(obj.second) << "}";
    return ss.str();
}
```

对于数组类型，使用了前面提到的`extent`，另外`std::is_array`可以判定T是不是一个数组。对于`std::array`，可以直接通过模板参数获得数组长度:

```cpp
// array
template <typename T>
std::enable_if_t<std::is_array_v<T>, std::string> dumps(const T &arr) {
  std::stringstream ss;
  ss << "[";
  for (size_t i = 0; i < std::extent<T>::value; ++i) {
    ss << dumps(arr[i]);
    if (i != std::extent<T>::value - 1) ss << ", ";
  }
  ss << "]";
  return ss.str();
}

// std::array
template <typename T, std::size_t N>
std::string dumps(const std::array<T, N> &obj) {
  std::stringstream ss;
  ss << "[";
  for (auto itr = obj.begin(); itr != obj.end();) {
    ss << dumps(*itr);
    if (++itr != obj.end()) ss << ", ";
  }
  ss << "]";
  return ss.str();
}
```

对于`std::tuple`，由于它的实现原理比较特殊，所以逻辑与其它不同，要写一个基于tuple长度N的递归展开:

```cpp
// std::tuple
template <size_t N, typename... Args>
std::enable_if_t<N == sizeof...(Args) - 1, std::string>
dumps(const std::tuple<Args...> &obj) {
  std::stringstream ss;
  ss << dumps(std::get<N>(obj)) << "]";
  return ss.str();
}
template <size_t N, typename... Args>
std::enable_if_t<N != 0 && N != sizeof...(Args) - 1, std::string>
dumps(const std::tuple<Args...> &obj) {
  std::stringstream ss;
  ss << dumps(std::get<N>(obj)) << ", " << dumps<N + 1, Args...>(obj);
  return ss.str();
}
template <size_t N = 0, typename... Args>
std::enable_if_t<N == 0, std::string> dumps(const std::tuple<Args...> &obj) {
  std::stringstream ss;
  ss << "[" << dumps(std::get<N>(obj)) << ", " << dumps<N + 1, Args...>(obj);
  return ss.str();
}
```

对于指针类型，我们希望输出它指向的值:

```cpp
// pointer
template <typename T>
std::string dumps(const T *p) {
  return dumps(*p);
}
// shared_ptr, weak_ptr, unique_ptr
template <typename T>
std::enable_if_t<
    is_instantiation_of_v<T, std::shared_ptr> ||
        is_instantiation_of_v<T, std::weak_ptr> ||
        is_instantiation_of_v<T, std::unique_ptr>,
    std::string>
dumps(const std::shared_ptr<T> &p) {
  return dumps(*p);
}
```

我们定义了很多模板，但这时我们会遇到名字查找的问题，如果直接把上面模板的定义写到头文件里，由于这些函数模板是相互调用的，所以这些模板之间的名字查找就存在了顺序依赖。所以，我们必须先前向声明所有的dumps模板。这和函数的前向声明是同一个道理。虽然我们直接把模板的定义放在头文件里，但模板的声明在某些情况下也是必不可少的。

我们已经处理了内置类型和STL中的类型，那么对于用户的自定义类型应该如何处理？这里就要用到依赖于实参的名字查找(Argument-dependent Name Lookup,ADL)，ADL是指编译器会去函数实参类型所在的命名空间里查找函数名字。所以我们在定义`UserDefine`类型时，在同一个命名空间内提供一个针对`UserDefine`的`dumps`重载即可。

```cpp
struct UserDefine { int a; };

std::string dumps(const UserDefine& obj){
    return "UserDefine" + std::to_string(obj.a);
}
```

[点击查看完整代码。](https://github.com/CnLzh/NoteBook/blob/main/CppTMPNotes/LearnTMPinUseP2/json.h)