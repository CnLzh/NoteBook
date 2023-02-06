# shared_ptr
关于C++中shared_ptr的一些问题和思考。

## 循环引用
循环引用指两个使用shared_ptr管理的对象，互相使用对象内部的shared_ptr指向对方，导致引用计数器错误，从而造成内存泄漏。

分析以下用例：
```cpp
class A {
 public:
  std::shared_ptr<B> a_;
  ~A() {
	std::cout << "class A destructor " << std::endl;
  }
};

class B {
 public:
  std::shared_ptr<A> b_;
  ~B() {
	std::cout << "class B destructor " << std::endl;
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
	std::cout << "class A destructor " << std::endl;
  }
};

class B {
 public:
  std::weak_ptr<A> _b;
  ~B() {
	std::cout << "class B destructor " << std::endl;
  }
};
```

运行结果为：
```
a_ptr use count:1
b_ptr use count:1
a_ptr use count:1
b_ptr use count:1
class B destructor
class A destructor
```

使用weak_ptr指向内存空间，不会引起引用计数器的变化。故在程序运行结束后，a_ptr与b_ptr的引用计数器均为零，正确执行了析构函数，解决了循环引用问题。

避免循环引用的通常做法是，资源管理者持有指向资源的shared_ptr，资源持有指向其管理者的weak_ptr。

另外，当想访问资源但并不想管理资源，且在使用时需要判断资源是否存在时，可以使用weak_ptr。

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

其实现原理是通过泛型编程与面向对象编程相结合，在shared_ptr创建时，不仅存储了一个引用计数器，还存储了构造期间使用的指针类型作为删除器，并在引用计数器为零时调用删除器。在上面的例子中，shared_ptr的删除器是在创建时构造的，其始终指向一个Derived对象，并在删除时调用其析构函数，该行为与虚函数无关，因此无需虚析构函数。以下模拟其析构的实现原理：

```cpp
template<typename Base>
class simple_ptr_internal_interface {
 public:
  virtual Base *get() = 0;
  virtual void destruct() = 0;
};

template<typename Base, typename Derived>
class simple_ptr_internal : public simple_ptr_internal_interface<Base> {
 public:
  struct DefaultDeleter {
	void operator()(Base *t) {
	  delete static_cast<Derived *>(t);
	}
  };
  simple_ptr_internal(Base *p)
	  : pointer_(p) {}
  virtual Base *get() override {
	return pointer_;
  }
  virtual void destruct() override {
	deleter_(pointer_);
  }
 private:
  Base *pointer_;
  DefaultDeleter deleter_;
};

template<typename Base>
class simple_ptr {
 public:
  template<typename Derived>
  explicit simple_ptr(Derived *d)
	  : internal(new simple_ptr_internal<Base, Derived>(d)) {}

  ~simple_ptr() {
	this->destruct();
  }

 private:
  void destruct() {
	internal->destruct();
  }
  simple_ptr_internal_interface<Base> *internal;
};
```

如上所述，shared_ptr在构造时保存了原始的指针类型，并在析构时释放。

### 析构动作所在的线程
对象的析构是一个同步操作，当最后一个指向对象的shared_ptr离开其作用域后，该对象会同时在同一个线程析构。这个线程不一定是对象诞生的线程，如果对象的析构比较耗时，那么可能会拖慢关键线程的速度。我们可以用一个单独的线程专门处理析构动作，把对象的析构都转移到那个线程，从而解放关键线程。

### 定制析构功能
shared_ptr具有定制析构功能，其构造函数可以有一个额外的模板类型参数，传入一个函数指针或者仿函数func，在析构对象时执行func(ptr)，其中ptr是shared_ptr保存的对象指针。

假设存在Stock类，代表一只股票的价格，每支股票有一个唯一的字符串表示。Stock是一个主动对象，能不断获取新价格。为了节约系统资源，同一个程序里每个出现的股票只有一个Stock对象，如果多处用到同一只股票，那么Stock对象应该被共享。如果这个股票没有在任何地方用到，其对应的Stock对象应该析构，以释放资源。

根据以上要求，我们设计一个对象池StockFactory。接口很简单，根据key返回Stock对象。在多线程程序中，对象可能被销毁，那么返回shared_ptr是合理的，我们写出如下代码(错的)。

```cpp
class StockFactory {
 public:
  std::shared_ptr<Stock> Get(const std::string &key);

 private:
  StockFactory(const StockFactory &) = delete;
  StockFactory operator=(const StockFactory &) = delete;

  std::mutex mutex_;
  std::map<std::string, std::shared_ptr<Stock>> stocks_;
};
```

其中`get()`的逻辑很简单，如果在`stocks_`中找到了key，就返回`stocks_[key]`，否则新建一个Stock并存入`stocks_[key]`。

然而这里存在一个问题，Stock对象永远不会被销毁，因为map中存的是shared_ptr。那么如果使用weak_ptr呢，比如：

```cpp
std::shared_ptr<Stock> StockFactory::Get(const std::string &key) {
  std::shared_ptr<Stock> p_stock;
  std::lock_guard lock(mutex_);
  std::weak_ptr<Stock>& wk_stock = stocks_[key];
  p_stock = wk_stock.lock();
  if(!p_stock){
	p_stock.reset(new Stock(key));
	wk_stock = p_stock;
  }
  return p_stock;
}
```

虽然通过这种方式可以销毁Stock，但程序却出现了轻微的内存泄露，因为`stocks_`的大小只增不减，`stocks_.size()`是曾经存活过的Stock对象总数。面对这个问题，可以利用shared_ptr的定制析构功能。我们利用这个特性，对上述代码修改，完整代码如下：

```cpp
#define DISALLOW_COPY_AND_ASSIGN(ClassName) \
    ClassName (const ClassName&) = delete;      \
    ClassName operator=(const ClassName&) = delete;

class Stock {
 public:
  Stock(const std::string &name)
	  : name_(name) {}

  const std::string Key() const {
	return name_;
  }

 private:
  std::string name_;
};

class StockFactory {
 public:
  std::shared_ptr<Stock> Get(const std::string &key) {
	std::shared_ptr<Stock> p_stock;
	std::lock_guard lock(mutex_);
	std::weak_ptr<Stock> &wk_stock = stocks_[key];
	p_stock = wk_stock.lock();
	if (!p_stock) {
	  p_stock.reset(new Stock(key),
					[this](Stock *stock) { deleteStock(stock); });
	  wk_stock = p_stock;
	}
	return p_stock;
  }

 private:
  void DeleteStock(Stock *stock) {
	if (stock) {
	  std::lock_guard lock(mutex_);
	  // race condition
	  auto it = stocks_.find(stock->key());
	  assert(it != stocks_.end());
	  if (it->second.expired()) {
		stocks_.erase(it);
	  }
	}
	delete stock;
  }

  std::mutex mutex_;
  std::map<std::string, std::weak_ptr<Stock>> stocks_;

  DISALLOW_COPY_AND_ASSIGN(StockFactory)
};

```

这里我们向`reset()`传递了一个仿函数，让它在析构`Stock* p`时调用`StockFactory`对象的`DeleteStock`成员函数。在`// race condition`处，若直接使用如下方式，则存在race condition：

```cpp
std::lock_guard lock(mutex_);
stocks_erase(stock->key());
```

race condition发生在函数进入`DeleteStock`后，在`lock`前，有线程B调用了相同`key`的`StockFactory::Get()`，此时的weak_ptr已经无法提升了，所以会有一个新的Stock对象被创建，而`DeleteStock`的析构才刚刚开始，此时内存中存在两个具有相同key的Stock对象，`DeleteStock`要删除其对象，但不应删除map中的key，否则若再有线程C调用了相同`key`的`StockFactory::Get()`，因map中的key已经被删除，又会构造一个新的Stock对象，导致线程B和线程C分别持有两个key相同的不同对象。

另外，若修改`expired()`为`lock()`，存在造成死锁的可能：`std::muetx`是不可重入的，若线程A进入`DeleteStock`后，在`lock`前，线程B调用了相同`key`的`StockFactory::Get()`，此时`stocks_[key]`的`use_count = 1`，线程A继续执行到`weak_ptr::lock()`提升成功，此时`stocks_[key]`的`use_count = 2`，因`stocks_[key]`提升成功，故不会调用`stocks_.erase(key)`。若线程B刚好释放了shared_ptr，此时`use_count = 1`，线程A继续执行，shared_ptr离开作用域后`use_count = 0`，在本次`DeleteStock`中递归调用了`DeleteStock`，而`std::mutex`不可重入，程序无法继续执行下去，导致死锁。

当然，此处还存在另外一个问题，在`StockFactory::Get()`中，我们把一个原始的`StockFactory this`指针保存在了仿函数中，这存在线程安全问题。如果这个StockFactory对象先于Stock对象被析构，Stock对象析构时会发生core dump。我们可以通过下文中的"弱回调"技术解决该问题。

### enable_shared_from_this
前文中讲到，如果StockFactory先于Stock对象被析构，Stock析构时去调用`StockFactory::DeleteStock()`时会core dump。似乎我们应该使用shared_ptr解决对象生命周期问题，比如在`StockFactory::Get()`中获得一个指向当前对象this的`shared_ptr<StockFactory>`对象，将其传递给仿函数。这样一来，仿函数中保存了一份`shared_ptr<StockFactory>`以确保调用`StockFactory::DeleteStock()`时StockFactory对象还存活着。例如：

```cpp
class StockFactory : public std::enable_shared_from_this<StockFactory>
...
...
p_stock.reset(new Stock(key),
			[self = shared_from_this()](Stock *stock) { self->deleteStock(stock); });
```

这里需要注意两点，一是不要直接使用this指针构造shared_ptr，而应使用标准库中提供的`std::enable_shared_from_this`类模板返回指向当前对象的shared_ptr指针，因为使用原始指针创建shared_ptr无法共享(会构造新的控制模块，而不会增加引用计数)，且会造成内存重复释放(double delete)；二是`shared_from_this()`不能在构造函数中调用，因为`enable_shared_from_this`的工作原理是使用第一个shared_ptr的副本初始化一个隐藏的weak_ptr，该副本指向该对象，因此在构造函数执行期间，该对象还不存在，而若使shared_ptr指向该对象，该对象必须存在(已经构造)，所以构造期间没有`enable_shared_from_this`可使用的shared_ptr。

当然，此处还存在一个问题，因为将shared_ptr传递给了仿函数，虽然保证了回调的时候StockFactroy对象一定存在，但StockFactory的生命周期似乎被意外的延长了，使其不短于绑定了仿函数的Stock对象。

有时候我们需要，如果对象还活着，就调用它的成员函数，否则忽略的语义。称之为"弱回调"。

### 弱回调