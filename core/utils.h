#ifndef UTILS_H
#define UTILS_H

#include "cxxopts.h"
#include "get_time.h"
#include "quick_sort.h"
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <limits.h>
#include <mutex>

#define intV int32_t
#define uintV int32_t
#define UINTV_MAX INT_MAX

#define intE int32_t
#define uintE int32_t

#define DEFAULT_NUMBER_OF_THREADS "1"
#define DEFAULT_MAX_ITER "10"
#define DEFAULT_STRATEGY "1"
#define DEFAULT_SOURCE_VERTEX "0"
#define DEFAULT_GRANULARITY "1"
#define TIME_PRECISION 5
#define VAL_PRECISION 14
#define THREAD_LOGS 0
// #define ADDITIONAL_TIMER_LOGS 0

struct CustomBarrier {
  int num_of_threads_;
  int current_waiting_;
  int barrier_call_;
  std::mutex my_mutex_;
  std::condition_variable my_cv_;

  CustomBarrier(int t_num_of_threads)
      : num_of_threads_(t_num_of_threads), current_waiting_(0),
        barrier_call_(0) {}

  void wait() {
    std::unique_lock<std::mutex> u_lock(my_mutex_);
    int c = barrier_call_;
    current_waiting_++;
    if (current_waiting_ == num_of_threads_) {
      current_waiting_ = 0;
      // unlock and send signal to wake up
      barrier_call_++;
      u_lock.unlock();
      my_cv_.notify_all();
      return;
    }
    my_cv_.wait(u_lock, [&] { return (c != barrier_call_); });
    //  Condition has been reached. return
  }

  void open() {
    barrier_call_=-1;
    my_cv_.notify_all();
  }
};

// CAS operation
template <class ET>
inline bool CAS(ET *ptr, ET oldv, ET newv)
{
  if (sizeof(ET) == 1)
  {
    return __sync_bool_compare_and_swap((bool *)ptr, *((bool *)&oldv), *((bool *)&newv));
  }
  else if (sizeof(ET) == 4)
  {
    return __sync_bool_compare_and_swap((int *)ptr, *((int *)&oldv), *((int *)&newv));
  }
  else if (sizeof(ET) == 8)
  {
    return __sync_bool_compare_and_swap((long *)ptr, *((long *)&oldv), *((long *)&newv));
  }
  else
  {
    std::cout << "CAS bad length : " << sizeof(ET) << std::endl;
    abort();
  }
}

template <class T>
bool checkEqual(T *array1, T *array2, long n)
{
  bool flag = true;
  quickSort(array1, n, [](uintV val1, uintV val2) {
      return val1 < val2;
  });

  quickSort(array2, n, [](uintV val1, uintV val2) {
      return val1 < val2;
  });


  for(long i = 0; i < n; i++)
  {
    if (array1[i] != array2[i])
    {
      flag = false;
      break;
    }
  }
  return flag;
}

#endif
