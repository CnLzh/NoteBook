// Copyright (C) 2023
//
// File:     thread_pool.h
// Brief:    Declares the ThreadPool class.
// Author:   CnLzh

#ifndef SRC_THREAD_POOL_H_
#define SRC_THREAD_POOL_H_

#include <vector>
#include <list>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <future>
#include <stdexcept>
#include <shared_mutex>

// 禁用拷贝和赋值构造函数
#define DISALLOW_COPY_AND_ASSIGN(ClassName) \
    ClassName (const ClassName&) = delete;      \
    ClassName operator=(const ClassName&) = delete;

class ThreadPool final {

  enum ThreadPoolStatus {
	TPS_RUNNING,  // 运行状态，接受新任务，并分配线程处理。
	TPS_SHUTDOWN,  // 关闭状态，不接受新任务，但会将任务队列中的任务执行完。待任务队列为空时，切换状态为TPS_TERMINATED。
	TPS_TERMINATED  // 终止状态：线程池彻底终止。
  };

  using Task = std::function<void()>;  // 任务模板
  const unsigned int kCorePoolSize_;  // 核心线程数
  const unsigned int kMaxPoolSize_;  // 最大线程数
  const unsigned int kMaxTaskSize_;  // 最大任务数
  std::list<std::thread> thread_pool_;  // 线程池
  std::mutex thread_pool_mutex_;  // 线程池同步锁
  std::queue<Task> tasks_;  // 任务队列
  std::mutex task_mutex_;  // 任务队列同步锁
  std::condition_variable task_cv_;  // 任务队列条件变量
  std::atomic<ThreadPoolStatus> thread_pool_status_;  // 线程池执行状态
  std::atomic<unsigned int> idle_thread_num_;  // 空闲线程数
  std::atomic<unsigned int> now_thread_num_;  // 当前线程数
  const unsigned int kIdleTimeOutSecond_;    // 空闲存活时间 (秒)
  std::list<std::thread::id> monitor_thread_free_;  // 等待释放的线程列表
  std::condition_variable monitor_cv_;  // 监控线程信号量
  std::mutex monitor_mutex_;  // 监控线程同步锁


 public:

  /**
   * @brief constructor
   * @param kCorePoolSize 核心线程数
   * @param kMaxPoolSize 最大线程数
   */
  explicit ThreadPool(
	  const unsigned int &kCorePoolSize = std::thread::hardware_concurrency() / 2,
	  const unsigned int &kMaxPoolSize = std::thread::hardware_concurrency(),
	  const unsigned int &kMaxTaskSize = 1000,
	  const unsigned int &kIdleTimeOutSecond = 60
  ) noexcept;

  /**
   * @brief destructor
   * @remark 唤醒线程池内所有线程，并等待所有线程执行结束
   */
  ~ThreadPool() noexcept;

  /**
   * @brief commit 提交一个任务
   * @tparam T 函数指针
   * @tparam Args 参数列表
   * @return
   */
  template<typename T, typename... Args>
  auto commit(T &&f, Args &&... args) -> std::future<decltype(f(args...))>;

  /**
   * @brief 获取空闲线程数
   * @return idle_thread_num_
   */
  [[nodiscard]] unsigned int GetIdleThreadCount() const;

  /**
   * @brief 获取线程数量
   * @return thread_pool_.size()
   */
  [[nodiscard]] size_t GetThreadCount();

  /**
   * @brief 不接受新任务，执行完任务队列中的所有任务后，关闭线程池
   */
  void Shutdown();

 private:

  /**
   * @brief 添加指定数量的线程
   * @param size 创建的线程数量
   */
  void AddThread(const unsigned int &size = 1);

  /**
   * @brief 守护线程，删除超过空闲存活时间的线程
   */
  void InitThreadMonitor();

  DISALLOW_COPY_AND_ASSIGN(ThreadPool)
};

template<typename F, typename ...Args>
auto ThreadPool::commit(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
  // 线程池不处于运行状态
  if (thread_pool_status_ != ThreadPoolStatus::TPS_RUNNING)
	throw std::runtime_error("Commit on ThreadPool is stopped.");

  // 推导任务返回值
  using RetType = decltype(f(args...));

  // 打包任务函数及参数
  auto task = std::make_shared<std::packaged_task<RetType()>>(
	  std::bind(std::forward<F>(f), std::forward<Args>(args)...)
  );

  // 任务结果
  std::future<RetType> result = task->get_future();

  // 将任务加入到任务队列
  {
	std::unique_lock task_lock(task_mutex_);
	if (kMaxTaskSize_ > tasks_.size()) {    // 任务数量未达到上限
	  tasks_.emplace(
		  [task]() {
			(*task)();
		  });
	} else {    // 任务数量达到上限
	  if (idle_thread_num_ < 1 && now_thread_num_ >= kMaxPoolSize_) {    // 线程数量达到上限
		throw std::runtime_error("The task was abandoned because task queue is full.");
	  } else {    // 线程数量未达到上限
		// 创建一个非核心线程
		task_lock.unlock();
		AddThread();
		task_lock.lock();
		tasks_.emplace(
			[task]() {
			  (*task)();
			});
	  }
	}
  }

  // 唤醒一个线程
  task_cv_.notify_one();

  return result;
}

#endif // SRC_THREAD_POOL_H_
