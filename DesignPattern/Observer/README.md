# Observer Pattern
Observer是由主体对象和观察者对象组成的。主体对象维护其依赖项(观察者)的列表，并通知他们主体对象的状态更改。是一种类似于订阅的关系。
例如：一个报社(主体)和很多订阅报纸的人(观察者)。人们可以通过向报社订阅的方式，获取报社的最新报纸；报社在更新了报纸后，会向所有订阅者(观察者列表)发布新的报纸。

## Observer Pattern可以解决哪些问题？
- 定义了对象之间的一对多的依赖关系，使对象松耦合。
- 当一个对象状态更改时，自动更新其依赖对象。
- 一个对象可以通知多个其他对象。

## Observer Pattern描述了什么解决方案？
- 定义了主体和观察者。
- 当主体更改时，所有观察者都会自动收到通知和更新。

也就是说，主体是唯一负责维护观察者列表的对象，并通过调用观察者的接口通知所有观察者状态改变。观察者的职责是在主体中注册和注销自己，并在收到通知时更新状态。

由此可见，主体和观察者彼此没有明确的了解，互相之间不清楚对方的实现细节。可以在运行时独立添加和删除观察者。

## Observer Pattern简单实现

```cpp
#include <iostream>
#include <string>
#include <set>

// 观察者基类
class Observer {
public:
	virtual void Update(const std::string& msg) = 0;
};

// 观察者派生类
class DerivedObserver : public Observer {
public:
	virtual void Update(const std::string& msg) {
		std::cout << "Derive OBserver read msg: " << msg << std::endl;
	}
};

// 主体
class Subject {
public:
	// 注册观察者
	void Register(Observer* ob) {
		obs_.insert(ob);
	}

	// 注销观察者
	void UnRegister(Observer* ob) {
		obs_.erase(ob);
	}

	// 通知观察者
	void Notify(const std::string& msg) {
		for (auto it : obs_) {
			it->Update(msg);
		}
	}

private:
	// 观察者列表
	std::set<Observer*> obs_;
};


int main() {
	Subject sub;  // 主体
	Observer* ob = new DerivedObserver;  // 观察者
	sub.Register(ob);  // 注册
	sub.Notify("hello");  // 通知观察者状态变化
	sub.UnRegister(ob);  // 注销
	sub.Notify("world");  // 通知观察者状态变化
	delete ob;
	return 0;
}
```

可以看到，所有`Observer`的派生类，均需要实现`Update`接口，通过`Subject`提供的`Register`和`UnRegister`接口注册或注销，即可订阅或取消接收主体状态变化后的消息通知。

当然，此种实现方式还存在一些约束：
- 需要继承，继承是一种强对象关系，不够灵活。
- 观察者用于接受通知的`Update`接口参数不能变化。

在`C++ 11`中，可以通过`std::function`来解决继承问题；通过可变参数模板解决接口参数不能改变的问题。