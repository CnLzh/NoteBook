// Copyright (C) 2023
//
// File:     thread_pool.cpp
// Brief:    Definition the ThreadPool class.
// Author:   CnLzh

#include "thread_pool.h"

ThreadPool::ThreadPool(
	const unsigned int &kCorePoolSize,
	const unsigned int &kMaxPoolSize,
	const unsigned int &kMaxTaskSize,
	const unsigned int &kIdleTimeOutSecond
) noexcept
	: kCorePoolSize_(kCorePoolSize),
	  kMaxPoolSize_(kMaxPoolSize),
	  kMaxTaskSize_(kMaxTaskSize),
	  idle_thread_num_(0),
	  now_thread_num_(0),
	  kIdleTimeOutSecond_(kIdleTimeOutSecond),
	  thread_pool_status_(TPS_RUNNING) {
  InitThreadMonitor();
  AddThread(kCorePoolSize_);
}

ThreadPool::~ThreadPool() noexcept {
  task_cv_.notify_all();
  for (auto &it : thread_pool_) {
	if (it.joinable())
	  it.join();
  }
}

void ThreadPool::AddThread(const unsigned int &size) {
  // 线程池不处于运行状态
  if (thread_pool_status_ != ThreadPoolStatus::TPS_RUNNING)
	throw std::runtime_error("Grow on ThreadPool is stopped.");
  unsigned int grow_size = size;
  std::lock_guard tp_lock(thread_pool_mutex_);
  for (; now_thread_num_ < kMaxPoolSize_ && grow_size > 0; --grow_size) {
	// 创建一个线程并加入到线程池
	thread_pool_.emplace_back(
		[this] {
		  for (;;) {
			Task task;
			{
			  std::unique_lock task_lock(task_mutex_);
			  // 等待被唤醒或超时
			  while (!task_cv_.wait_for(
				  task_lock,
				  std::chrono::seconds(kIdleTimeOutSecond_),
				  [this] {
					return thread_pool_status_ == ThreadPoolStatus::TPS_TERMINATED || !tasks_.empty();
				  })) {
				// 若等待超时且存活线程数大于核心线程数，删除非核心线程
				if (now_thread_num_ > kCorePoolSize_) {
				  --idle_thread_num_;
				  --now_thread_num_;
				  std::lock_guard monitor_lock(monitor_mutex_);
				  monitor_thread_free_.emplace_back(std::this_thread::get_id());
				  monitor_cv_.notify_one();
				  return;
				}
			  }
			  if (thread_pool_status_ == ThreadPoolStatus::TPS_TERMINATED && tasks_.empty())
				return;
			  --idle_thread_num_;
			  task = std::move(tasks_.front());
			  tasks_.pop();
			  if (thread_pool_status_ == ThreadPoolStatus::TPS_SHUTDOWN && tasks_.empty())
				thread_pool_status_ = ThreadPoolStatus::TPS_TERMINATED;
			}
			task();
			++idle_thread_num_;
		  }
		});
	++idle_thread_num_;
	++now_thread_num_;
  }
}

unsigned int ThreadPool::GetIdleThreadCount() const {
  return idle_thread_num_;
}

size_t ThreadPool::GetThreadCount() {
  return now_thread_num_;
}

void ThreadPool::Shutdown() {
  thread_pool_status_ = ThreadPoolStatus::TPS_SHUTDOWN;
}

void ThreadPool::InitThreadMonitor() {
  std::thread monitor([this] {
	for (;;) {
	  std::unique_lock monitor_lock(monitor_mutex_);
	  monitor_cv_.wait(monitor_lock);
	  std::lock_guard tp_lock(thread_pool_mutex_);
	  auto it = monitor_thread_free_.begin();
	  while (it != monitor_thread_free_.end()) {
		auto id = *it;
		auto ret = std::find_if(
			thread_pool_.begin(),
			thread_pool_.end(),
			[=](std::thread &t) {
			  return t.get_id() == id;
			});
		if (ret != thread_pool_.end()) {
		  ret->join();
		  thread_pool_.erase(ret);
		}
		it = monitor_thread_free_.erase(it);
	  }
	}
  });
  monitor.detach();
}
