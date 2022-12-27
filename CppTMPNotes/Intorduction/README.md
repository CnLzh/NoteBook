# 1. Intorduction

## 1.1 元编程
Meta这个词有着"更高一层抽象"的意思。例如在数据结构中提到的MetaData通常指"描述数据的数据"。

元编程(MetaProgramming)就是编写一类特殊的程序，这类程序将其他程序或自身视为输入的数据来进行处理，称之为元程序。

## 1.2 C++模板元编程
C++模板元编程(C++ Template MetaProgramming, TMP)是指在编程过程中，逻辑代码和元程序自身的代码是写在一起的，元程序通过编译生成逻辑代码，并与其他逻辑代码合并到一起。即TMP在编译期执行，而普通的C++代码在运行期执行。

C++ Template MetaProgramming(TMP)借助编译器处理模板的能力，在编译期生成代码，最终和普通的C++代码一起编译执行。C++标准也在C++11之后对TMP进行了大幅度支持，包括一些核心的语言特性和标准库组件。这里列举了其中比较重要的部分:
- C++11: `type traits, auto, variadic templates, decltype, declval, alias template`
- C++14: `variable template, generic lambda, generalized constepr, return type decuction`
- C++17: `class template argument deduction, if constexpr, constexpr lambda`
- C++20: `constraints and concepts, abbreviated function templateconsteval, constinit`

## 1.3 TMP的作用
- TMP可以做编译器的计算
这使得我们能将一些运行时的工作迁移到编译期，从而提升程序的运行效率。当然代价就是编译的时间变长，以及目标文件增大。但因为程序使一次编译多次运行的，所以有时候以编译期的代价换取运行时的效率是非常划算的。
- TMP可以对类型做计算
TMP对类型做计算的能力是无可代替的。C++缺乏有效的自省机制，在运行时根据变量的类型去编写逻辑是不可能的。但在编译期，类型可以作为模板的实参(Arguments)参与逻辑运行。
- TMP可以使代码变得简洁优美
TMP自身的代码是晦涩难懂的，但有时它可以使使用它的外部代码变得简洁优雅。
- TMP的广泛应用
TMP已经是一个广泛使用且不可或缺的技术了。很多程序库，特别是泛型程序库，几乎都不可避免的使用了TMP。即使没有直接写过TMP的代码，也一定间接的从TMP中受益。
