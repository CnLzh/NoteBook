// Copyright (C) 2023
//
// File:     thread_pool.cpp
// Brief:    Definition the ThreadPool class.
// Author:   CnLzh

#include "thread_pool.h"

ThreadPool::ThreadPool(
	const unsigned int &kCorePoolSize,
	const unsigned int &kMaxPoolSize,
	const unsigned int &kMaxTaskSize
) noexcept
	: kCorePoolSize_(kCorePoolSize),
	  kMaxPoolSize_(kMaxPoolSize),
	  kMaxTaskSize_(kMaxTaskSize),
	  idle_thread_num_(kCorePoolSize),
	  thread_pool_status_(TPS_RUNNING) {

}

ThreadPool::~ThreadPool() noexcept {
  task_cv_.notify_all();
  for (auto &it : thread_pool_) {
	if (it.joinable())
	  it.join();
  }
}
