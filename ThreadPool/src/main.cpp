#include "thread_pool.h"
#include <iostream>
#include <synchapi.h>

void func() {
  // do something
}

int main() {
  ThreadPool tp;
  tp.commit(func);
  return 0;
}