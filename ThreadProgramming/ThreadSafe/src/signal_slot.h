// Copyright (c) 2023
// File:    signal_slot.h
// Brief:   Definition the SignalImpl, Signal, Slot class.
// Author:  cnlzh

#ifndef SRC_SIGNAL_SLOT_H_
#define SRC_SIGNAL_SLOT_H_

#include <assert.h>

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace detail {

template<typename SlotFunction>
class SlotImpl;

template<typename SlotFunction>
class SignalImpl {
 public:
  using SlotList = std::vector<std::weak_ptr<SlotImpl<SlotFunction>>>;

  SignalImpl() : slots_(new SlotList) {}

  void CopyOnWrite() {
	if (slots_.use_count() != 1) {
	  slots_.reset(new SlotList(*slots_));
	}
	assert(slots_.use_count() == 1);
  }

  void clean() {
	std::lock_guard<std::mutex> lock(mtx_);
	CopyOnWrite();
	auto it = slots_->begin();
	while (it != slots_->end()) {
	  if (it->use_count() == 0) {
		it = slots_->erase(it);
	  } else {
		++it;
	  }
	}
  }

  std::shared_ptr<SlotList> slots_;
  std::mutex mtx_;
};

template<typename SlotFunction>
class SlotImpl {
 public:
  using DT = SignalImpl<SlotFunction>;

  SlotImpl(const std::shared_ptr<DT> &data, SlotFunction &&slot_func)
	  : data_(data), tie_(), slot_func_(slot_func), tied_(false) {}

  SlotImpl(const std::shared_ptr<DT> &data, SlotFunction &&slot_func,
		   const std::shared_ptr<void> &tie)
	  : data_(data), tie_(tie), slot_func_(slot_func), tied_(true) {}

  ~SlotImpl() {
	std::shared_ptr<DT> data(data_.lock());
	if (data) {
	  data->clean();
	}
  }

  std::weak_ptr<DT> data_;
  std::weak_ptr<void> tie_;
  SlotFunction slot_func_;
  bool tied_;
};

}  // namespace detail

using Slot = std::shared_ptr<void>;

template<typename Signature>
class Signal;

template<typename Ret, typename... Args>
class Signal<Ret(Args...)> {
 public:
  using Callback = std::function<void(Args...)>;
  using SignalImpl = detail::SignalImpl<Callback>;
  using SlotImpl = detail::SlotImpl<Callback>;

  Signal() : impl_(new SignalImpl) {}
  ~Signal() {}

  Slot connect(Callback &&func) {
	std::shared_ptr<SlotImpl> slot_impl(
		new SlotImpl(impl_, std::forward<Callback>(func)));
	add(slot_impl);
	return slot_impl;
  }

  Slot connect(Callback &&func, const std::shared_ptr<void> &tie) {
	std::shared_ptr<SlotImpl> slot_impl(
		new SlotImpl(impl_, std::forward<Callback>(func), tie));
	add(slot_impl);
	return slot_impl;
  }

  void emit(Args &&... args) {

	std::shared_ptr<SignalImpl::SlotList> slots;
	{
	  std::lock_guard<std::mutex> lock(impl_->mtx_);
	  slots = impl_->slots_;
	}

	for (auto it = slots->begin(); it != slots->end(); ++it) {
	  std::shared_ptr<SlotImpl> slot_impl = it->lock();
	  if (slot_impl) {
		std::shared_ptr<void> guard;
		if (slot_impl->tied_) {
		  guard = slot_impl->tie_.lock();
		  if (guard) {
			slot_impl->slot_func_(args...);
		  }
		} else {
		  slot_impl->slot_func_(args...);
		}
	  }
	}
  }

 private:
  void add(const std::shared_ptr<SlotImpl> &slot) {

	std::lock_guard<std::mutex> lock(impl_->mtx_);
	impl_->CopyOnWrite();
	impl_->slots_->push_back(slot);

  }

 private:
  const std::shared_ptr<SignalImpl> impl_;
};

#endif //SRC_SIGNAL_SLOT_H_
