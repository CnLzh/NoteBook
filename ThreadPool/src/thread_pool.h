// Copyright (C) 2023
//
// File:     thread_pool.h
// Brief:    Declares the ThreadPool class.
// Author:   CnLzh
//

#ifndef SRC_THREAD_POOL_H_
#define SRC_THREAD_POOL_H_

#define DISALLOW_COPY_AND_ASSIGN(ClassName) \
    ClassName (const ClassName&) = delete;      \
    ClassName operator=(const ClassName&) = delete;

class ThreadPool {
 public:
  explicit ThreadPool();
  ~ThreadPool();

 private:
  DISALLOW_COPY_AND_ASSIGN(ThreadPool)
};

#endif // SRC_THREAD_POOL_H_
