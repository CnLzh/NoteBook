# shared_ptr

## 循环引用
循环引用指两个使用shared_ptr管理的对象，互相使用对象内部的shared_ptr指向对方，导致引用计数器错误，从而造成内存泄漏。

分析以下用例：
```cpp
class A {
 public:
  std::shared_ptr<B> a_;
  ~A() {
	std::cout << "class A destruction " << std::endl;
  }
};

class B {
 public:
  std::shared_ptr<A> b_;
  ~B() {
	std::cout << "class B destruction " << std::endl;
  }
};

int main() {
  std::shared_ptr<A> a_ptr = std::make_shared<A>();
  std::shared_ptr<B> b_ptr = std::make_shared<B>();
  std::cout << "a_ptr use count:" << a_ptr.use_count() << std::endl;
  std::cout << "b_ptr use count:" << b_ptr.use_count() << std::endl;

  a_ptr->a_ = b_ptr;
  b_ptr->b_ = a_ptr;
  std::cout << "a_ptr use count:" << a_ptr.use_count() << std::endl;
  std::cout << "b_ptr use count:" << b_ptr.use_count() << std::endl;

  return 0;
}
```

运行结果为：

```
a_ptr use count:1
b_ptr use count:1
a_ptr use count:2
b_ptr use count:2
```

当分别使用a_ptr指向一个A类型对象，b_ptr指向一个B类型对象时，a_ptr与b_ptr的引用计数器均为一，即对应的内存空间被一个shared_ptr所指向。

随后将a_ptr中的a_指向b_ptr，b_ptr中的b_指向a_ptr，此时a_ptr与b_ptr的引用计数器均为二，即a_ptr指向的内存空间，也被b_ptr中的_b指向；b_ptr指向的内存空间，也被a_ptr中的_a指向，故其引用计数器均为二。

此时程序运行结束，a_ptr与b_ptr离开作用域，其引用计数器减一后不为零，故认为还有指针指向对应内存，因此没有执行class A与class B的析构函数，造成内存泄漏。

### 解决循环引用
我们可以通过weak_ptr解决该问题，weak_ptr是弱引用，不控制对象的生命周期，但它知道对象是否还活着。如果对象还活着，那么它可以提升为有效的shared_ptr；如果对象已经死了，提升失败，返回一个空的shared_ptr。

分析以下用例：

```cpp
class A {
 public:
  std::weak_ptr<B> _a;
  ~A() {
	std::cout << "class A destruction " << std::endl;
  }
};

class B {
 public:
  std::weak_ptr<A> _b;
  ~B() {
	std::cout << "class B destruction " << std::endl;
  }
};
```

运行结果为：
```
a_ptr use count:1
b_ptr use count:1
a_ptr use count:1
b_ptr use count:1
class B destruction
class A destruction
```

使用weak_ptr指向内存空间，不会引起引用计数器的变化。故在程序运行结束后，a_ptr与b_ptr的引用计数器均为零，正确执行了析构函数，解决了循环引用问题。

## 线程安全
shared_ptr的引用计数器是原子操作，但对象的读写不是。因为shared_ptr有两个数据成员，读写操作不能原子化。shared_ptr的线程安全级别和标准库容器、内建类型是一样的，即：
- 一个shared_ptr对象可以被多线程同时读取。
- 一个shared_ptr对象同时被多线程读写，需要加锁。

注：以上是shared_ptr对象本身的线程安全级别，不是管理的对象的线程安全级别。

为了性能考虑，通常的做法是在栈上创建一个shared_ptr，并仅对读写动作加锁，尽可能缩小临界区。例如：

```cpp
shared_ptr<Foo> global_ptr;

void read() {
    shared_ptr<Foo> local_ptr;
    {
        lock_guard lock(mutex);
        local_ptr = global_ptr;
    }
    // use loacl_ptr since here.
}
```

当然，这只能保证shared_ptr对象本身是线程安全的，若通过shared_ptr对其管理的对象进行读写，则要考虑其管理的对象的线程安全问题。

## 技术与陷阱

### 意外延长对象的生命周期
shared_ptr是强引用，只要有一个指向对象的shared_ptr存在，该对象就不会析构。而shared_ptr是允许拷贝构造和赋值的，如果不小心遗留了一个拷贝，那么对象就永世长存了。例如`std::bind`，因其会将实参拷贝一份，如果参数是shared_ptr，那么对象的生命周期就不会短于`std::function`对象，例如：

```cpp
shared_ptr<Foo> p(new Foo);
std::function<void()> func = std::bind(&Foo::do_something, p);
```

这里func对象持有了`shared_ptr<Foo>`的一份拷贝，有可能会在不经意间延长了创建的Foo对象的生命周期。

### 函数参数
因为要修改引用计数器，而且多线程的拷贝通常要加锁，shared_ptr的拷贝开销要比原始指针高，但需要拷贝的时候并不多。多数情况下，可以用const reference方式传递，一个线程只需要在最外层有一个实体对象，之后都可以用const reference来使用这个shared_ptr。依照这个规则，基本上不会遇到反复拷贝shared_ptr导致的性能开销。且因最外层的实体对象是在栈上，不可能被其他线程看到，始终是线程安全的。

### 析构动作在创建时被捕获
这意味着虚析构函数不再是必须的，即使基类的析构函数不是虚函数，使用shared_ptr管理的派生类也能正确的调用析构函数并释放资源。所以shared_ptr可以持有任何对象并确保能安全的释放。例如：

```cpp
class Base {
 public:
  ~Base() {
	std::cout << "Base destructor" << std::endl;
  }
};

class Derived : public Base {
 public:
  ~Derived() {
	std::cout << "Derived destructor" << std::endl;
  }
};

int main() {

  Base *b = new Derived;
  delete b;
  
  std::cout << "---------------" << std::endl;

  std::shared_ptr<Base> p(new Derived);

  return 0;
}
```

运行结果为：

```
Base destructor
---------------
Derived destructor
Base destructor
```

可以看到，若基类不是虚析构函数，shared_ptr也可以正确完成派生类的析构；但原始指针不能正确完成派生类的析构。

其实现原理是通过泛型编程与面向对象编程相结合，在shared_ptr创建时，保存了所创建的派生类的指针，并在最终析构时调用。以下模拟其析构的实现原理：