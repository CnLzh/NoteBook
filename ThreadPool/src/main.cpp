#include "thread_pool.h"

void func() {
  // do something
}

int main() {
  ThreadPool tp;
  tp.commit(func);
  return 0;
}