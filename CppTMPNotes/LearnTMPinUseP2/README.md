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
