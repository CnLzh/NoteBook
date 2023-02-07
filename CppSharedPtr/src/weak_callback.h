//
// Created by cnlzh on 2023/2/8.
//

#ifndef SRC_WEAK_CALLBACK_H_
#define SRC_WEAK_CALLBACK_H_

#include <memory>
#include <functional>

template<typename T, typename ...Args>
class WeakCallBack {
 public:
  WeakCallBack(const std::weak_ptr<T> &object,
			   const std::function<void(T *, Args...)> &function)
	  : object_(object), function_(function) {}

  void operator()(Args &&... args) const {
	std::shared_ptr<T> ptr(object_.lock());
	if (ptr)
	  function_(ptr.get(), std::forward<Args>(args)...);
  }
 private:
  std::weak_ptr<T> object_;
  std::function<void(T *, Args...)> function_;
};

template<typename T, typename ...Args>
WeakCallBack<T, Args...> MakeWeakCallback(const std::shared_ptr<T> &object,
										  void (T::*function)(Args...)) {
  return WeakCallBack<T, Args...>(object, function);
}

template<typename T, typename ...Args>
WeakCallBack<T, Args...> MakeWeakCallback(const std::shared_ptr<T> &object,
										  void (T::*function)(Args...) const) {
  return WeakCallBack<T, Args...>(object, function);
}

#endif //SRC_WEAK_CALLBACK_H_
