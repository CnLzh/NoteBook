// Copyright (C) 2023
//
// File:     thread_pool.cpp
// Brief:    Definition the ThreadPool class.
// Author:   CnLzh

#include "thread_pool.h"

ThreadPool::ThreadPool(const unsigned int &kCorePoolSize, const unsigned int &kMaxPoolSize) noexcept
	: kCorePoolSize_(kCorePoolSize),
	  kMaxPoolSize_(kMaxPoolSize),
	  thread_pool_status_(TPS_RUNNING) {

}

ThreadPool::~ThreadPool() noexcept {
  task_cv_.notify_all();
  for (auto &it : thread_pool_) {
	if (it.joinable())
	  it.join();
  }
}
