# 2. Template Basics

## 2.1 Template Declaration and Definition 
在C++中，一共可以声明5种不同的模板，分别是:类模板(class template)，函数模板(function template)，变量模板(variable template)，别名模板(alias template)，概念(concept)。

```cpp
template <typenmae T> struct class_t;
template <typename T> void function_t(T);
template <typename T> T variable_t;
template <typename T> using alias_t = T;
template <typename T> concept no_constraint = true;
```

其中，前三种模板都可以拥有定义，后两种模板不需要提供定义，因为它们不产生运行时的实体。

```cpp
template <typenmae T> struct class_t {};
template <typename T> void function_t(T) {}
template <typename T> T variable_t = T(123);
```

对于类模板，函数模板，变量模板，他们的声明和定义与普通的类，函数，变量一致，区别仅在于开头多了template关键字。template关键字表明这事一个模板，尖括号种声明了模板的参数。

## 2.2 Template Parameters and Arguments
在模板中，可以声明三种类型的形参(Parameters)，分别是: 非类型模板形参(Non-type Template Parameters)，类型模板形参(Type Template Parameters)和模板模板形参(Template Template Parameters):

```cpp
template<int n> struct NontypeTemplateParameter {};
template<typename T> struct TypeTemplateParameter {};
template<template <typenmae T> typename Tmpl> TemplateTemplateParameter {};
```

其中，非类型的形参接收一个确定类型的常量作为实参(Arguments)。一般情况下，非类型模板形参必须是结构化类型(structural type)的，主要包括:
- 整形
- enum类型
- 指针和引用类型
- 浮点数类型和字面量类型

需要注意的是，非类型模板实参必须是常量，因为模板是在编译期被展开的，在这个阶段只有常量，没有变量。

```cpp
template <float &f>
void foo() {std::cout << f << std::endl; }

template <int i>
void bar() {std::cout << i << std::endl; }

int main(){
    static float f1 = 0.1f;
    float f2 = 0.2f;
    for<f1>();  // output: 0.1
    foo<f2>();  // error: 不是常量

    int i1 = 1;
    int const i2 = 2;
    bar<i1>();  // error: 不是常量
    bar<i2>();  // output: 2
}

```

对于类型模板形参(Type Template Parameters)，我们使用`typename`关键字声明它是一个类型。对于模板模板形参(Template Template Parameters)，和类模板的声明类似，也是在类型的前面加上`template <...>`。模板模板形参只接受类模板或类的别名模板作为实参，并且实参模板的形参列表必须要与形参模板的形参列表匹配。

```cpp
template <template <typename T> typename Tmpl>
struct S {};

template <typename T> void foo() {}
template <typename T> struct Bar1 {};
template <typename T, typename U> struct Bar2 {};

S<foo>();   // error: 不是类模板
S<Bar1>();  // ok: 
S<Bar2>();  // error: 形参列表不匹配

```

一个模板可以声明多个形参，一般情况下，可以声明一个变长的形参列表，称为"template parameter pack"，这个变长形参列表可以接收0个或多个非类型常量，类型，模板作为模板实参。变长形参列表必须出现在所有模板形参的最后。

```cpp
template <typename T, typename U> struct TemplateWithTwoParameters {};

template <int... Args> struct VariadicTemplate1 {};
template <int, typename... Args> struct VariadicTemplate2 {};
template <template <typename T> typename... Args> strcut VariadicTemplate3 {};

VariadicTemplate1<1,2,3>();
VariadicTemplate2<1,int>();
VariadicTemplate3<>();
```

模板可以声明默认实参，与函数的默认实参类似。只有主模板(Primary Template)才可以声明默认实参，模板特化(Template Specialization)不可以。
```cpp
template <typename T = int> struct TemplateWithDefaultArguments {};
```

## 2.3 Template Instantiation
模板的实例化(Instantiation)是指由泛型的模板定义生成具体的类型，函数，变量的过程。模板在实例化时，模板形参被替换为实参，从而生成具体的实例。模板的实例化分为两种: 隐式实例化和显式实例化，其中隐式实例化式平常最常用的实例化方式。隐式实例化是指当我们要用这个模板生成实体的时候，要创建具体对象的时候，才做实例化。而显式实例化是告诉编译器去实例化一个模板，但现在还不用它创建对象，将来再用。需要注意的是，隐式实例化和显式实例化不是根据是否隐式传参而区分的。

当我们在代码中使用了一个模板，触发了一个实例化过程时，编译器会用模板的实参替换模板的形参，生成对应的代码。同时，编译器会根据一定规则选择一个位置，将生成的代码插入到这个位置中，这个位置被称为POI(point of instantiation)。由于要做替换才能生成具体的代码，因此C++要求模板的定义对它的POI一定式可以见的。换句话说，在同一个翻译单元中，编译器一定要能看到模板的定义，才能对其进行替换，完成实例化。因此最常见的做法是，将模板定义在头文件中，然后在源文件中通过`#include`头文件来获取该模板的定义。这就是模板编程中的包含模型(Inclusion Model)。

## 2.4 Template Arguments Deduction
为了实例化一个模板，编译器需要知道所有的模板实参，但不是每个实参都要显式指定。有时，编译器可以根据函数调用的实参来推断模板的实参，这个过程被称为模板实参推导(Template Arguments Deduction)。对每个函数实参，编译器都尝试去推导对应的模板实参，如果所有的模板实参都能被推导出来，且推导结果不产生冲突，那么模板实参推导成功。例如:

```cpp
template <typename T>
void foo(T,T) {}

foo(1,1);
fpp(1,1.0);
```

在`foo(1,1)`中，两次推导结果一致，均为int，推导成功；在`foo(1,1,0)`中，两次推导结果不一致，推导失败。C++17中引入了类模板实参推导(Class Template Arguments Deduction)，可以通过类模板的构造函数来推导模板实参: 

```cpp
template <typename T>
struct S { S(T,int) {} }；

S s(1,2);
```

## 2.5 Template Specialization
模板的特化(Template Specialization)允许我们替换一部分或全部的形参。其中，替换全部形参的特化称为全特化(Full Specialization)，替换部分形参的特化称为偏特化(Partial Specialization)，非特化的原始模板称为主模板(Primary Template)。只有类模板和变量模板可以进行偏特化，函数模板只能全特化。在实例化模板的时候，编译器会从所有的特化版本中选择最匹配的那个实现来做替换，如果没有特化匹配，那么就会主动选择柱模版进行替换操作。

```cpp
template <typenamte T, typename U> void foo(T, U) {}    // 偏特化
template <> void foo(int,float) {}  //全特化

template <typenmae T, typename U> struct S {};  // #1
template <typename T> struct S<int, T> {};      // #2
template <> struct S<int, float> {};            // #3

S<int, int>();      // choose #2
S<int, float>();    // choose #3
S<float, int>();    // choose #1
```

我们可以只声明一个特化，然后在其他地方定义它:

```cpp
template <> void foo<float, int>;
template <typenmae T> struct S<float, T>;
```

## 2.6 Function Template Overloading
函数模板虽然不能偏特化，但是可以重载(Overloading)，并且可以与普通的函数一起重载。在C++中，所有的函数和函数模板，只要他们拥有不同的签名(Signature)，就可以在程序中共存。一个函数(模板)的签名包含以下部分:

1. 函数(模板)的非限定名(Unqualified Name)
2. 名字的域(Scope)
3. 成员函数(模板)的CV限定符
4. 成员函数(模板)的引用限定符
5. 函数(模板)的形参列表类型，如果是模板，则取决于实例化前的形参列表类型
6. 函数模板的返回值类型
7. 函数模板的模板形参列表

## 2.7 Example

```cpp
template <int N>
struct binary {
  static constexpr int value = binary<N / 10>::value << 1 | N % 10;
};

template <> struct binary<0> {
  static constexpr int value = 0;
};

std::cout << binary<101>::value << std::endl;
```

我们定义了一个主模板，接受一个非类型形参。同时定义了一个在`N == 0`时的全特化，在实例化`binary<0>`时，编译器会匹配这个特化。在主模板中，定义了一个静态常量`value`，并初始化为`binary<N / 10>::value << 1 | N % 10`，由于静态常量会在编译器求值，所以编译器在实例化`binary<101>`时会尝试求值这个表达式。这个表达式中包含了对`binary`的另一个实例化，所以编译器会递归的实例化`binary`这个模板，直到`N == 0`，模板的实例化匹配到特化版本，此处定义了`value = 0`，递归到这里被终止，求值表达式层层返回，最终计算出`binary<101>::value = 5`。

通过对这个例子的分析，得到了TMP的一些线索:
1. 模板像函数一样被使用，模板的形参就像是函数的形参，模板的静态成员作为函数的返回值。
2. 通过实例化来调用模板。
3. 通过特化和重载来实现分支选择。
4. 通过递归来实现循环逻辑。
5. 所有过程发生在编译期间，由编译器驱动。

所以，已经有了函数，有了if/else，有了循环，就可以编程了。这种编程的逻辑工作在编译期间，处理的数据是常量和类型，没有运行时，也没有变量。这就是TMP。
