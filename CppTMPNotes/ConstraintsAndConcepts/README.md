# Constraints and Concepts
概念(Concepts)是对模板实参的一些约束(Constraints)的集合，是C++20引入的新特性。这些约束可以被用于选择最恰当的函数模板重载和类模板偏特化。其优势有两个: 一是语法更简单的同时功能也更强大；二是编译器产生的错误信息更容易理解。

对比一下我们就可以看到Concept的优势:

```cpp
// SFINAE:
template <typename T>
static constexpr bool is_numeric_v = std::is_integral_v<T> || std::is_floating_point_v<T>;

template <typename T>
std::enable_if_t<is_numeric_v<T>, void> foo(T);

// Concept:
template <typename T>
concept Numeric = std::is_intergral_v<T> || std::is_floating_point_v<T>;

template <Numeric T>
void foo(T);
```

Concept的声明语句形如:

```cpp
template <...>
concept _name = _constraint_expression_;
```

一个约束表达式就是一个对模板形参的逻辑运算符表达式，它指定对于模板实参的要求。例如上面看到的`std::is_integral_v<T> || std::is_floating_point_v<T>`就是一个约束表达式。与模板实例化中的逻辑运算表达式不同，约束表达式中的逻辑运算是短路求值的。约束表达式可以出现在Concept的声明中，比如`template <typename T> Numerical = std::is_integral_v<T> || std::is_floating_point_v<T>`；也可以出现在`requires`从句中。

`requires`关键字用来引入Requires从句，requires从句可以放在函数模板的签名里，用来表示约束。requires关键字后面必须跟一个常量表达式，可以是true/false，也可以是Concept表达式，Concept的合取/析取，也可以是约束表达式，还可以是requires表达式。下面这些写法都是等效的:

```cpp
// 形参列表中直接使用Concept
template <Numeric T>
void foo(T) {}

// Concept的requires从句
template <typename T> requires Numeric<T>
void foo(T) {}

template <typename T>
void foo(T) requires Numeric<T> {}

// 使用约束表达式的requires从句
template <typename T> requires::std::is_integral_v<T> || std::is_floating_point_v<T>
void foo(T) {}

template <typename T>
void foo(T) requires::std::is_integral_v<T> || std::is_floating_point_v<T> {}
```

`requires`关键字还可以用来引入一个Requires表达式。它是一个bool类型的右值表达式，描述一些模板实参的约束。若约束满足(良构)则返回true，否则返回false。下面是用requires表达式来声明Concept的例子:

```cpp
template <typename T> concept Incrementable = requires(T v) { ++v; };
template <typename T> concept Decrementable = requires(T v) { --v; };

template <typename From, typename To>
concept ConvertibleTo = std::is_convertible_v<From, To> && requires(std::add_rvalue_reference_t<From> (&f)()) {
    static_cast<To>(f());
};

template <typename T, typename U = T>
concept Swappable = requires(T&& t, U&& u) {
    swap(std::forward<T>(t), std::forward<U>(u));
    swap(std::forward<U>(u), std::forward<T>(t));
};
```

另外，有些写法是requires表达式独有的，比如下面的Hashable，它判断`a`能否传给`std::hash`，以及`std::hash`的返回值类型能否转为`std::size_t`:

```cpp
template <typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> ConvertibleTo<std::size_T>;
};
```

除了用在Concept的声明中外，requires表达式还可以直接用在requires从句中:

```cpp
template <typename T> concept Addable = requires(T x) { x + x; };

template <tpename T> requires Addable<T>
T add(T a, T b) { return a + b; }

template <typename T> requires requires(T x) { x + x; }
T add (T a, T b) { return a + b; }
```

最后，约束出现的顺序决定了编译器检查的顺序，所以下面两个函数模板虽然在逻辑上是等效的，但有不同的约束，不算是重定义:

```cpp
template <Incrementable T>
void g(T) requires Decrementable<T> {};

template <Decrementable T>
void g(T) requires Incrementable<t> {};
```

Concept虽然在C++20才被写入标准里，但它是一个历史非常悠久的东西。对Concept的讨论触及元编程的一个核心问题，为什么我们需要元编程，什么情况下需要用到元编程。

标准库中一共有6种Iteerator，分别是: `InputIterator, OutputIterator, ForwardIterator, BidirectionalIterator, RandomAccessIterator, ContiguouslIterator`。标准库种的泛型排序函数`std::sort`只接受`RandomAccessIterator`的迭代器参数:

```cpp
template <typename RandomIt>
void sort(RandomIt first, RandomIt last);
```

这里的`RandomAccessIterator`实际上就是一个概念(concept)，它描述了`std::sort`对其参数类型的要求。这样的要求在标准库种有很多，被统称为具名要求(Named Requirements)。这些东西本质上都是concept，它们在concept的语法出现之前，就已经存在了。因为对concept的需求是与泛型编程伴生的: 我们希望泛型，但又不是完全的泛型，对传入的类型仍有一定要求。比如`std::sort`就要求它的参数类型是支持随机访问的迭代器，而这个要求是源自快速排序算法的，是`std::sort`自身的要求，是一种必然的要求。

再进一步从软件设计的角度讨论这个问题。concept其实代表了我们在设计中对某一类实体的抽象。假如我们想实现一种接口与实现分离的设计，接口是统一的，而实现有多种，传统的做法是什么呢？我们会实现一个纯虚的基类，在里面定义所有纯虚的接口，然后所有的实现都继承这个基类，在派生类里提供具体实现。这带来两个问题，一是必须通过基类指针来操作接口，通过运行时多态的机制访问实现，这是有成本的，而有时候并不需要在运行时变换实现，在编译时就能确定想要用哪个实现，但仍然避免不了运行时的成本；二是这种约束太强了，不仅约束了实现的类型，还约束了所有接口的参数类型和返回值类型。但是有了concept后，我们不需要基类，只需要通过concept声明一系列对类型和接口的约束就可以了，比如我们可以约束这个类型必须包含一个名为"work"的接口，这个接口接受一个数值类型参数，返回一个数值类型参数。所有的实现不论是什么类型，只要满足这个约束，就可以拿来使用。并且这种对接口的约束可以是严格的，也可以是松散的，比如可以要求一个接口使用int类型的参数，也可以要求它接受所有数值类型的参数。

```cpp
// dynamic polymorphic
struct WorkerInterface {
    virtual int work(int) = 0;
};

struct WorkerImpl : WorkerInterface {
    int work(int) override {
        return 1;
    }
};

int do_work(WorkerInterface* w) {
    return w->work(1);
}

// static polymorphic
template <typename T>
concept worker = requires(T a) {
    { a.work(int()) } -> std::same_as<int>;
};

template <worker T>
int do_work(T&& w) {
    return w.work(1);
}
```

总结一下，简单来说，C++模板元编程: 编译时以类型，常数作为入参的函数，输出一个类型，函数或常数，可以被运行时使用。主要是通过pattern匹配实现分支，再通过递归，从而做到图灵完全。