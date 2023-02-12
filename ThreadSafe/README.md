# Thread Safe
线程安全是多线程编程中非常重要的概念。如何设计规范的方式以确保所有线程正常运行并操作共享数据结构，是不可避免的话题。

编写线程安全的类并不复杂，用同步原语(Synchronization Primitives)保护内部状态即可。但对象的生存周期不能由其自身的mutex来保护。如何避免对象析构时可能存在的竞态条件(Race Condition)是C++多线程编程面临的基本问题。

## 线程安全的定义
一个线程安全的类，应当满足以下三个条件：
- 多个线程同时访问，表现出正确的行为。
- 无论操作系统如何调度这些线程，无论这些线程的执行顺序如何。
- 调用端代码无需额外的同步或协调动作。

根据这个定义，C++标准库里的大多数class都不是线程安全的，包括`std::string, std::vector, std::map`等，因为这些class通常需要外部加锁才能供多个线程同时访问。

## 避免死锁
假设线程A和线程B都需要两个互斥M1和M2时，若A持有M1，B持有M2，则A、B线程互相持有对方所需要的资源，互相等待对方释放资源，若线程不主动释放资源，则两个线程均无法继续执行，这就造成了死锁。因此，一个函数如果要锁住多个对象，为了保证始终按照相同顺序加锁(否则会死锁)，可以通过比较mutex对象的地址，始终先加锁地址较小的mutex。

## 对象的构造
对象构造的线程安全很简单，只要在构造期间不泄漏this指针，即：
- 不要在构造函数中注册任何回调
- 不要在构造函数中跨线程传递this给其他对象

因为构造函数执行期间，对象还没有完成初始化，如果this被泄漏给其他对象，那么别的线程可能访问这个半成品对象，会造成难以预料的后果。

即使在构造函数的最后一行也不要泄漏this，因为如果`class Foo`是一个基类，优先于派生类构造，那么执行完`Foo:Foo()`的最后一行代码后，还会继续执行派生类的构造函数，此时仍然不安全。

## 对象的析构
对象的析构，在单线程中只需要注意避免空悬指针和野指针。但在多线程环境中，存在了很多竞态条件。虽然可以通过mutex保护临界资源，但析构函数会破坏这一条件，它会把mutex成员变量销毁掉。

### 数据成员的mutex不能保护析构
假设如下：
1. 线程A执行X的析构函数，持有互斥锁，继续往下执行析构。
2. 线程B此时访问X，被阻塞。

接下来将发生不可预料的后果。因为线程A会把互斥锁销毁，那么线程B可能永远阻塞下去，也可能进入临界区然后core dump，或者发生其他更糟糕的事。

由此可见，作为数据成员的mutex只能用于同步本类的其他数据成员的读写，不能保护安全的析构。因为mutex的生命周期最多和对象一样长，不能保护整个析构过程。而且，析构过程本来也不需要被保护，因为只有别的线程都访问不到这个对象，析构才是安全的。

### 线程安全的Observer
一个动态创建的对象是否还活着，通过指针是看不出来的。指针就是指向了一块内存，如果内存上的对象已经销毁，那么根本不能访问，也就无法知道对象的状态。即使这个地址可以访问，万一是在原地址又创建了一个新的对象呢？或者这个新的对象类型异于老的对象呢？

也就是说，如果对象x注册了任何非静态成员函数回调，那么必然在某处存在持有指向x的指针，这就暴露在竞态条件的风险下。一个经典场景就是Observer模式：

设`Subject为主体，Observer为观察者`，当`Subject`通知每个`Observer`时，如何确保`Observer`对象还活着？或许可以尝试在`Observer`的析构函数中解除注册？但这并不能奏效。存在以下问题：

1. 如何确认`Subject`还活着？
2. 若析构时，`Subject`正好在访问`Observer`？

既然`Observer`对象正在析构，调用它的任何非静态成员函数都是不安全的，何况`Observer`是个基类，执行到基类的析构函数时，派生类对象已经析构掉了，此时访问该对象，core dump恐怕是最幸运的结果。

虽然这些竞态条件看起来可以通过加锁解决，但是在哪里加锁，谁持有这些锁，似乎又不是那么简单。要是存在一个活着的对象，能告诉我们要访问的对象是否还活着就好了。

另外，当存在两个指针`p1`和`p2`，且指向堆上的同一个对象`Object`时，若线程A通过`p1`指针销毁了对象，那么`p2`就变成了空悬指针。要想安全的销毁对象，最好是在其余线程都看不到的情况下做(垃圾回收的原理，就是所有人都用不到的东西一定是垃圾)。

所以，在需要暴露给其他线程时，使用指向对象的原始指针是不好的做法。`Subject`保存的不应该是原始的`Observer*`，而是别的能分辨`Observer`对象是否存活且可以安全销毁的东西。

### 使用shared_ptr/weak_ptr
显然，通过智能指针提供的引用计数功能，可以解决上述竞态条件的问题。当然，此处要警惕循环引用导致资源泄露的问题。

```cpp
class Observable{
 public:
  void Register(const std::weak_ptr<Observer>& x){
	std::lock_guard lock(mutex_);
	observers_.push_back(x);
  }

  void NotifyObservers(){
	std::lock_guard lock(mutex_);
	for(auto it = observers_.begin(); it != observers_.end();){
	  std::shared_ptr<Observer> obj(it->lock());
	  if(obj){
		obj->Update();
		++it;
	  } else{
		it = observers_.erase(it);
	  }
	}
  }

 private:
  std::mutex mutex_;
  std::vector<std::weak_ptr<Observer>> observers_;
};
```

通过将`Observer*`替换为`weak_ptr`，解决了线程安全的问题，但还有几个疑点：

- 侵入性：强制要求Observer必须以shared_ptr管理
- 不是完全线程安全：Observer的析构函数会调用`subject->unregister`解注册，万一`subject`已经不存在了呢？
- 锁争用：Observable的三个成员函数都用了互斥同步，会导致`register和unregister`等待`notify`，而后者的执行时间是无法预期的，因为它同步回调了用户提供的`Update`函数。
- 死锁：万一`Update`中调用了`(un)register`，如果`mutex_`是不可重入的，会造成死锁；如果是可重入的，会造成迭代器失效。

关于shared_ptr的技术与陷阱，可以参考[此处](https://github.com/CnLzh/NoteBook/tree/main/CppSharedPtr)。

### 解决Observer的问题
Observer模式的主要问题在于其面向对象的设计。因为Observer是基类，带来了非常强的耦合，限制了成员函数的名字、参数、返回值，还限制了成员函数的类型(必须是Observer的派生类)。而且若FOO想同时观察两个类型的事件，则需要多继承，这无疑是糟糕的。在现代C++中，我们可以通过Signal/Slots的方式绕过Observer的限制，完全解决上述遗留的疑点，其实现借助了使用`std::function和std::bind`取代虚函数的思想。

Signal/Slots本质上就是Observer模式，主体收到某个signal后，通知对应的观察者(调用对应的slots)。其完整实现代码见[此处](https://github.com/CnLzh/NoteBook/tree/main/ThreadSafe/src/SignalSlot.h)。