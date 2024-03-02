# C++ "this" Pointer
`this`指针，实际上就是指向这个类自己的指针。

在C++对象模型中，类的成员函数不保存在类的地址空间，其本质上是全局函数。那么类的成员函数是如何被调用的呢？例如：

```cpp
class Data {
public:
	void DataPrintf() {
		cout << data_ << endl;
	}
private:
	int data_ = 10;
};

int main(){
    Data d;
    d.DataPrintf();
    return 0;
}

```

实际上`DataPrintf()`的函数原型是`DataPrinf(Data* const this)`，只不过`Data* const this`是隐藏的，编译器帮我们做了。调用`d.DataPrintf()`会被编译器解析为`DataPrintf(d)`。在成员函数中访问的成员变量`data_`会被编译器解析为`this->data_`。

注意，`this`是一个底层指针，即`this`的指向不可改变，但指向的值可以被改变。观察以下用例：

```cpp
class Data {
public:
	void DataPrintf(){
		cout << data_ << endl;
	}
private:
	int data_ = 10;
};

void func(const Data& d) {
	d.DataPrintf();
}
```

这段代码是无法通过编译的。因为`func`的参数`const Data& d`是一个常量引用，即`d`是一个常量。而`d.DataPrintf()`被解析成`DtatPrintf(d)`，我们无法用一个常量初始化一个底层指针`Data* const this`，因为底层指针指向的值是可变的。

我们修改为`void DataPrintf() const`即可编译。因为`void DataPrintf() const`会被编译器解析为`void DataPirntf(const Data* const this)`，即`this`是底层指针也是顶层指针，其指向和指向的值均为常量，此时用一个常量初始化`this`是被允许的。