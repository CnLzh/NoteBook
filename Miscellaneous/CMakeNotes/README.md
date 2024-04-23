# CMake

CMake是最流行的跨平台构建和理C/C++项目的工具，适用于不同编译器和操作系统。使用CMake，可以令开发者灵活的构建和维护各类项目。其学习曲线非常陡峭，但功能及其强大。

## 构建项目

### 构建最简单的可执行文件

CMkae通过`CMakeLists.txt`配置项目的构建系统，一个最简单的`CMakeLists.txt`如下：

```
# 指定CMAKE最低版本号
cmake_minimum_required (VERSION 3.28.4)

# 项目名称
project(HelloCmake)

message(STATUS "PROJECT_NAME = ${PROJECT_NAME}")

# 添加可执行文件
add_executable(HelloCmake main.cc)

```

这样就构建了一个名为`HelloCmake`的可执行文件。CMake可以使用`message`输出信息，这里可以看到输出`PROJECT_NAME = HelloCmake`。其中，`${PROJECT_NAME}`是CMake定义的变量，表示项目名。

我们可以使用如下方式生成可执行文件：

```
mkdir build
cd build
cmake ..
make
```

这样就编译得到了`HelloCmake`可执行文件。

### 源文件

一个项目中显然不可能只有一个源文件，如果多个源文件每个都手动添加，显然比较麻烦，因此可以使用如下方式：

```
# 指定CMAKE最低版本号
cmake_minimum_required (VERSION 3.28.4)

# 项目名称
project(HelloCmake)

message(STATUS "PROJECT_NAME = ${PROJECT_NAME}")

# 将src中的所有源文件保存到SRCS中
aux_source_directory(src SRCS)

# 添加可执行文件
add_executable(HelloCmake ${SRCS})

```

使用`aux_source_directory(<dir> <var>)`命令，可以把`src`中所有的源文件名称保存到`SRCS`中，这样生成可执行文件时就可以使用`SRCS`而不用手动添加所有源文件了。

### 头文件

一个项目中除了源文件外，肯定还包含头文件，如果头文件和源文件在同级路径中，在源文件的`#include "xxx.h"`是可以直接找到对应的头文件的。

但如果源文件在`src`文件夹中，头文件在`include`文件夹中，虽然可以通过`#include "../include/xxx.h"`的方式找到头文件，但在多级目录情况下，显然比较麻烦，因此可以使用如下方式：

```
# 指定CMAKE最低版本号
cmake_minimum_required (VERSION 3.28.4)

# 项目名称
project(HelloCmake)

message(STATUS "PROJECT_NAME = ${PROJECT_NAME}")

# 添加指定目录到搜索头文件路径中
include_directories(include)

# 将src中的所有源文件保存到SRCS中
aux_source_directory(src SRCS)

# 添加可执行文件
add_executable(HelloCmake ${SRCS})
```

这样，项目构建时就会把`include`文件夹加入到搜索路径中，直接使用`#include "xxx.h"`就可以找到对应的头文件了。

当然，一个工程有时会包括多个目标文件，但`include_directories`会把对应的目录全局性的加入到所有目标文件的搜索路径中，这可能是不好的行为，因为可能导致不必要的目录包含。

所以，还有另一个方式`target_include_directories`可以把对应的目录加入到执行目标文件的搜索路径中，例如：

```
# 指定CMAKE最低版本号
cmake_minimum_required (VERSION 3.28.4)

# 项目名称
project(HelloCmake)

message(STATUS "PROJECT_NAME = ${PROJECT_NAME}")


# 添加可执行文件1
add_executable(HelloCmake_1 main1.cc)
add_executable(HelloCmake_2 main2.cc)

# 添加指定目录到指定目标文件的搜索路径中
target_include_directories(HelloCmake_1 PUBLIC ${PROJECT_SOURCE_DIR}/include_1)
target_include_directories(HelloCmake_2 PUBLIC ${PROJECT_SOURCE_DIR}/include_2)
```

这样就可以实现每个不同的目标文件有不同的搜索路径，其中`${PROJECT_SOURECE_DIR}`是指这个项目的源目录，就是`CMakeLists.txt`所在的目录。

另外，这里还有一个关键字`PUBLIC`，用来控制依赖项的使用范围，一共分为`PRIVATE, INTERFACE, PUBLIC`三种。但对于生成的目标文件是可执行文件的情况，三者没有明显区别。所以，我们将在下文的生成静态库和动态库部分中，详细介绍三者的作用和区别。

### 构建最简单的静态库和动态库

目标文件除了可执行文件以外，还包括静态库和动态库。

构建动态库的方式如下：

```
# 指定CMAKE最低版本号
cmake_minimum_required (VERSION 3.28.4)

# 项目名称
project(HelloLib)

# 添加动态库
add_library(HelloLib SHARED main.cc)
```

使用`add_library`和`SHARED`关键字，就可以构建一个动态库。

构建静态库的方式如下：

```
# 指定CMAKE最低版本号
cmake_minimum_required (VERSION 3.28.4)

# 项目名称
project(HelloLib)

# 添加静态库
add_library(HelloLib STATIC main.cc)
```
将`SHARED`关键字替换为`STATIC`关键字，就可以构建一个静态库。

### 链接静态库和动态库

构建了静态库或动态库后，还需要把库链接到可执行文件中。

链接动态库和静态库的方式是一致的，使用`target_link_libraries`即可，例如：

```
# 指定CMAKE最低版本号
cmake_minimum_required (VERSION 3.28.4)

# 项目名称
project(Hello)

# 添加目标文件
add_executable(Hello mian.cpp)

# 添加动态/静态库
target_link_libraries(Hello PRIVATE HelloLib)

```

我们知道，无论是动态库还是静态库，往往除了生成的库文件以外，还有对应的头文件。其中的头文件包含了库函数的声明，而库文件包含了库函数的实现。

接下来，我们用一个完整的生成和链接动态库的例子，对`PRIVATE, INTERFACE, PUBLIC`三个关键字做详细解释。

