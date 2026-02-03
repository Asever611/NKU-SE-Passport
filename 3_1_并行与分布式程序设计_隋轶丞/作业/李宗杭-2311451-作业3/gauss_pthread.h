// gauss_pthread.h
#ifndef GAUSS_PTHREAD_H_INCLUDED
#define GAUSS_PTHREAD_H_INCLUDED

// 初始化线性方程组
void init_gauss();

// 串行高斯消元
void serial_elimination();

// 动态线程模式下的任务函数
void* dynamic_thread_static_simple_task(void* param);
void* dynamic_thread_static_AVX_task(void* param);
void* dynamic_thread_dynamic_simple_task(void* param);
void* dynamic_thread_dynamic_AVX_task(void* param);

// 用于动态线程模式的线程创建和管理函数
void dynamic_thread(void* (*task)(void*));

// 静态线程模式下的任务函数
void* static_thread_static_simple_task(void* param);
void* static_thread_static_AVX_task(void* param);
void* static_thread_dynamic_simple_task(void* param);
void* static_thread_dynamic_AVX_task(void* param);

// 用于静态线程模式的线程创建和管理函数
void static_thread(void* (*task)(void*));

double measure_time_gauss(void (*test_func)(void* (*task)(void*)), void* (*task)(void*), bool serial, int repeat);
void test_gauss();
#endif // GAUSS_PTHREAD_H_INCLUDED
