// gauss_pthread.cpp
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <immintrin.h>
#include <windows.h>
#include <cmath>
#include <time.h>
#include "gauss_pthread.h"
#include "utils.h"
using namespace std;

const int MAX_N = 2560;
float A[MAX_N][MAX_N];
float B[MAX_N];
float X[MAX_N];
int n = 128;                  // 方程组规模
int chunk_size = 1;       // 分块大小


// 初始化线性方程组（对角线占优）
void init_gauss() {
    srand(unsigned(time(NULL)));
    for (int i = 0; i < n; ++i) {
        float sum = 0.0f;
        for (int j = 0; j < n; ++j) {
            A[i][j] = (float)rand() / RAND_MAX;
            sum += fabs(A[i][j]);
        }
        A[i][i] = sum + 1.0f;  // 保证对角线占优
        B[i] = (float)rand() / RAND_MAX * 100;
    }
}

// 串行AVX
void serial_AVX() {
    for (int k = 0; k < n - 1; ++k) {  // 主元行
        for (int i = k + 1; i < n; ++i) {  // 待消元行
            float factor = A[i][k] / A[k][k];
            int j = k;
            __m256 vec_factor = _mm256_set1_ps(factor);
            for(; j <= n - 8; j += 8) {
                __m256 vec_A_k = _mm256_loadu_ps(A[k] + j);
                __m256 vec_A_i = _mm256_loadu_ps(A[i] + j);
                vec_A_i = _mm256_sub_ps(vec_A_i, _mm256_mul_ps(vec_factor, vec_A_k));
                _mm256_storeu_ps(A[i] + j, vec_A_i);
            }
            for (; j < n; ++j) {
                A[i][j] -= factor * A[k][j];
            }
            B[i] -= factor * B[k];
        }
    }
}

// 静态任务
void* dynamic_thread_static_simple_task(void* param) {
    threadParam_t* p = (threadParam_t*)param;
    int k = p->k;
    int t_id = p->t_id;
    int start = k + t_id * chunk_size + 1;

    for(int i = start; (i < start + chunk_size) && (i < n); i++) {      // 消元操作，只处理分配的行块
        float factor = A[i][k] / A[k][k];
        for(int j = k; j < n; j++) {
            A[i][j] -= factor * A[k][j];
        }
        B[i] -= factor * B[k];
    }

    pthread_exit(NULL);
    return NULL;
}

// 静态任务 + AVX
void* dynamic_thread_static_AVX_task(void* param) {
    threadParam_t* p = (threadParam_t*)param;
    int k = p->k;
    int t_id = p->t_id;
    int start = k + t_id * chunk_size + 1;

    for(int i = start; (i < start + chunk_size) && (i < n); i++) {      // 消元操作，只处理分配的行块
        float factor = A[i][k] / A[k][k];
        int j = k;
        __m256 vec_factor = _mm256_set1_ps(factor);
        for(; j <= n - 8; j += 8) {
            __m256 vec_A_k = _mm256_loadu_ps(A[k] + j);
            __m256 vec_A_i = _mm256_loadu_ps(A[i] + j);
            vec_A_i = _mm256_sub_ps(vec_A_i, _mm256_mul_ps(vec_factor, vec_A_k));
            _mm256_storeu_ps(A[i] + j, vec_A_i);
        }
        for (; j < n; ++j) {        // 处理剩余元素
            A[i][j] -= factor * A[k][j];
        }
        B[i] -= factor * B[k];
    }

    pthread_exit(NULL);
    return NULL;
}

// 动态任务
void* dynamic_thread_dynamic_simple_task(void* param) {
    threadParam_t* p = (threadParam_t*)param;
    int k = p->k;
    int t_id = p->t_id;
    int start;

    while (1) {
        pthread_mutex_lock(&mutex_task);
        start = k + next_row + 1;
        next_row += chunk_size;
        pthread_mutex_unlock(&mutex_task);

        for(int i = start; (i < start + chunk_size) && (i < n); i++) {
            float factor = A[i][k] / A[k][k];
            for(int j = k; j < n; j++) {
                A[i][j] -= factor * A[k][j];
            }
            B[i] -= factor * B[k];
        }
        if (start >= n) {
            break;
        }
    }

    pthread_exit(NULL);
    return NULL;
}

// 动态任务 + AVX
void* dynamic_thread_dynamic_AVX_task(void* param) {
    threadParam_t* p = (threadParam_t*)param;
    int k = p->k;
    int t_id = p->t_id;
    int start;

    while (1) {
        pthread_mutex_lock(&mutex_task);
        start = k + next_row + 1;
        next_row += chunk_size;
        pthread_mutex_unlock(&mutex_task);

        for(int i = start; (i < start + chunk_size) && (i < n); i++) {
            float factor = A[i][k] / A[k][k];
            int j = k;
            __m256 vec_factor = _mm256_set1_ps(factor);
            for(; j <= n - 8; j += 8) {
                __m256 vec_A_k = _mm256_loadu_ps(A[k] + j);
                __m256 vec_A_i = _mm256_loadu_ps(A[i] + j);
                vec_A_i = _mm256_sub_ps(vec_A_i, _mm256_mul_ps(vec_factor, vec_A_k));
                _mm256_storeu_ps(A[i] + j, vec_A_i);
            }
            for (; j < n; ++j) {
                A[i][j] -= factor * A[k][j];
            }
            B[i] -= factor * B[k];
        }
        if (start >= n) {
            break;
        }
    }

    pthread_exit(NULL);
    return NULL;
}

// 动态分配线程
void dynamic_thread(void* (*task)(void*)) {
    vector<pthread_t> thread_handles(thread_num);
    vector<threadParam_t> params(thread_num);
    pthread_mutex_init(&mutex_task, NULL);
    for (int k = 0; k < n; k++) {
        if (task == dynamic_thread_static_simple_task || task == dynamic_thread_static_AVX_task) {
            chunk_size = (n - (k + 1)) / thread_num + 1;    // 静态分配每个线程需要处理的行数
        }
        next_row = 0;
        for (int t_id = 0; t_id < thread_num; t_id++) {
            params[t_id].k = k;
            params[t_id].t_id = t_id;
        }
        for (int t_id = 0; t_id < thread_num; t_id++) {
            pthread_create(&thread_handles[t_id], NULL, task, &params[t_id]);
        }
        for (int t_id = 0; t_id < thread_num; t_id++) {
            pthread_join(thread_handles[t_id], NULL);
        }
    }
    pthread_mutex_destroy(&mutex_task);
}

// 静态任务
void* static_thread_static_simple_task(void* param) {
   threadParam_t* p = (threadParam_t*)param;
    int t_id = p->t_id;
    int start;

    for (int k = 0; k < n; k++) {
        chunk_size = (n - (k + 1)) / thread_num + 1;
        start = k + t_id * chunk_size + 1;
        for(int i = start; (i < start + chunk_size) && (i < n); i++) {
            float factor = A[i][k] / A[k][k];
            for(int j = k; j < n; j++) {
                A[i][j] -= factor * A[k][j];
            }
            B[i] -= factor * B[k];
        }

        pthread_barrier_wait(&barrier);     // 等所有线程都完成消元，再进入下一轮
    }

    pthread_exit(NULL);
    return NULL;
}

// 静态任务 + AVX
void* static_thread_static_AVX_task(void* param) {
   threadParam_t* p = (threadParam_t*)param;
    int t_id = p->t_id;
    int start;

    for (int k = 0; k < n; k++) {
        chunk_size = (n - (k + 1)) / thread_num + 1;
        start = k + t_id * chunk_size + 1;
        for(int i = start; (i < start + chunk_size) && (i < n); i++) {
            float factor = A[i][k] / A[k][k];
            int j = k;
            __m256 vec_factor = _mm256_set1_ps(factor);
            for(; j <= n - 8; j += 8) {
                __m256 vec_A_k = _mm256_loadu_ps(A[k] + j);
                __m256 vec_A_i = _mm256_loadu_ps(A[i] + j);
                vec_A_i = _mm256_sub_ps(vec_A_i, _mm256_mul_ps(vec_factor, vec_A_k));
                _mm256_storeu_ps(A[i] + j, vec_A_i);
            }
            for (; j < n; ++j) {
                A[i][j] -= factor * A[k][j];
            }
            B[i] -= factor * B[k];
        }
        pthread_barrier_wait(&barrier);
    }

    pthread_exit(NULL);
    return NULL;
}

// 动态任务
void* static_thread_dynamic_simple_task(void* param) {
   threadParam_t* p = (threadParam_t*)param;
    int t_id = p->t_id;
    int start;

    for (int k = 0; k < n; k++) {
        if (t_id == 0)
            next_row = 0;
        pthread_barrier_wait(&barrier);     // 等待重置next_arr
        while (1) {
            pthread_mutex_lock(&mutex_task);
            start = k + next_row + 1;
            next_row += chunk_size;
            pthread_mutex_unlock(&mutex_task);
            for(int i = start; (i < start + chunk_size) && (i < n); i++) {
                float factor = A[i][k] / A[k][k];
                for(int j = k; j < n; j++) {
                    A[i][j] -= factor * A[k][j];
                }
                B[i] -= factor * B[k];
            }
            if (start >= n) {
                break;
            }
        }
        pthread_barrier_wait(&barrier);     // 等所有线程都完成消元，再进入下一轮
    }

    pthread_exit(NULL);
    return NULL;
}

// 动态任务 + AVX
void* static_thread_dynamic_AVX_task(void* param) {
   threadParam_t* p = (threadParam_t*)param;
    int t_id = p->t_id;
    int start;

    for (int k = 0; k < n; k++) {
        if (t_id == 0)
            next_row = 0;
        pthread_barrier_wait(&barrier);     // 等待重置next_arr
        while (1) {
            pthread_mutex_lock(&mutex_task);
            start = k + next_row + 1;
            next_row += chunk_size;
            pthread_mutex_unlock(&mutex_task);
            for(int i = start; (i < start + chunk_size) && (i < n); i++) {
                float factor = A[i][k] / A[k][k];
                int j = k;
                __m256 vec_factor = _mm256_set1_ps(factor);
                for(; j <= n - 8; j += 8) {
                    __m256 vec_A_k = _mm256_loadu_ps(A[k] + j);
                    __m256 vec_A_i = _mm256_loadu_ps(A[i] + j);
                    vec_A_i = _mm256_sub_ps(vec_A_i, _mm256_mul_ps(vec_factor, vec_A_k));
                    _mm256_storeu_ps(A[i] + j, vec_A_i);
                }
                for (; j < n; ++j) {
                    A[i][j] -= factor * A[k][j];
                }
                B[i] -= factor * B[k];
            }
            if (start >= n) {
                break;
            }
        }
        pthread_barrier_wait(&barrier);     // 等所有线程都完成消元，再进入下一轮
    }

    pthread_exit(NULL);
    return NULL;
}

// 静态分配线程
void static_thread(void* (*task)(void*)) {
    vector<pthread_t> thread_handles(thread_num);
    vector<threadParam_t> params(thread_num);
    pthread_mutex_init(&mutex_task, NULL);
    pthread_barrier_init(&barrier, NULL, thread_num);       // 初始化屏障，指定需要等待的线程数为 thread_num
    for (int t_id = 0; t_id < thread_num; t_id++) {
        params[t_id].t_id = t_id;
    }
    for (int t_id = 0; t_id < thread_num; t_id++) {
        pthread_create(&thread_handles[t_id], NULL, task, &params[t_id]);
    }
    for (int t_id = 0; t_id < thread_num; t_id++) {
        pthread_join(thread_handles[t_id], NULL);
    }
    pthread_mutex_destroy(&mutex_task);
    pthread_barrier_destroy(&barrier);
}

// 计时函数
double measure_time_gauss(void (*test_func)(void* (*task)(void*)), void* (*task)(void*), bool serial, int repeat) {
    double sum = 0;
    for (int i = 0; i < repeat; i++) {
        init_gauss();
        sum -= get_time();
        if (serial) {
            serial_AVX();
        }
        else {
            test_func(task);
        }
        sum += get_time();
    }
    return sum / repeat * 1000.0;
}

// 测试函数
void test_gauss() {
    int repeat = 10;

    printf("动态线程    数据规模：128\n");
    printf("测试小数据规模在不同线程数情况，初步观察各方法运行情况\n");
    printf("%-10s %-12s %-12s %-12s %-12s %-12s %-12s\n",
        "线程数", "串行AVX", "静态任务", "静态AVX", "动态任务", "动态AVX", "原总用时");
    n = 128;
    for (int i = 2; i <=32 ; i += 2){
        thread_num = i;

        double t_serial = measure_time_gauss(nullptr, nullptr, true, repeat);
        double t_static = measure_time_gauss(dynamic_thread, dynamic_thread_static_simple_task, false, repeat);
        double t_static_AVX = measure_time_gauss(dynamic_thread, dynamic_thread_static_AVX_task, false, repeat);
        chunk_size =1;
        double t_dynamic = measure_time_gauss(dynamic_thread, dynamic_thread_dynamic_simple_task, false, repeat);
        double t_dynamic_AVX = measure_time_gauss(dynamic_thread, dynamic_thread_dynamic_AVX_task, false, repeat);
        double total = (t_serial + t_static + t_static_AVX + t_dynamic + t_dynamic_AVX) * repeat;

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_serial, t_static, t_static_AVX, t_dynamic, t_dynamic_AVX, total);
    }

    printf("\n动态线程    数据规模：2560\n");
    printf("测试大规模数据在不同线程数下情况，与小数据规模对比判断\n");
    printf("%-10s %-12s %-12s %-12s %-12s %-12s %-12s\n",
        "线程数", "串行AVX", "静态任务", "静态AVX", "动态任务", "动态AVX", "原总用时");
    n = 2560;
    for (int i = 2; i <=32 ; i += 2){
        thread_num = i;

        double t_serial = measure_time_gauss(nullptr, nullptr, true, repeat);
        double t_static = measure_time_gauss(dynamic_thread, dynamic_thread_static_simple_task, false, repeat);
        double t_static_AVX = measure_time_gauss(dynamic_thread, dynamic_thread_static_AVX_task, false, repeat);
        chunk_size =1;
        double t_dynamic = measure_time_gauss(dynamic_thread, dynamic_thread_dynamic_simple_task, false, repeat);
        double t_dynamic_AVX = measure_time_gauss(dynamic_thread, dynamic_thread_dynamic_AVX_task, false, repeat);
        double total = (t_serial + t_static + t_static_AVX + t_dynamic + t_dynamic_AVX) * repeat;

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_serial, t_static, t_static_AVX, t_dynamic, t_dynamic_AVX, total);
    }

    printf("\n动态线程    线程数量：8\n");
    printf("采用较优线程数量，全面测试不同规模下情况\n");
    printf("%-10s %-12s %-12s %-12s %-12s %-12s %-12s\n",
        "规模", "串行AVX", "静态任务", "静态AVX", "动态任务", "动态AVX", "原总用时\n");
    thread_num = 8;
    for (int i = 128; i <=2560 ; i += 128){
        n = i;

        double t_serial = measure_time_gauss(nullptr, nullptr, true, repeat);
        double t_static = measure_time_gauss(dynamic_thread, dynamic_thread_static_simple_task, false, repeat);
        double t_static_AVX = measure_time_gauss(dynamic_thread, dynamic_thread_static_AVX_task, false, repeat);
        chunk_size =1;
        double t_dynamic = measure_time_gauss(dynamic_thread, dynamic_thread_dynamic_simple_task, false, repeat);
        double t_dynamic_AVX = measure_time_gauss(dynamic_thread, dynamic_thread_dynamic_AVX_task, false, repeat);
        double total = (t_serial + t_static + t_static_AVX + t_dynamic + t_dynamic_AVX) * repeat;

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_serial, t_static, t_static_AVX, t_dynamic, t_dynamic_AVX, total);
    }

    printf("\n动态线程（左两列） - 静态线程（右两列）    数据规模：2560    线程数量：8\n");
    printf("使用并行策略显著优于串行策略的数据规模，测试不同粒度下动态分配任务情况\n");
    printf("%-10s %-12s %-12s %-12s %-12s %-12s\n",
        "分配粒度", "静态AVX", "动态AVX", "静态AVX", "动态AVX", "原总用时");
    n = 2560;
    thread_num = 8;
    for (int i = 1; i <=64 ;){
        double t_dynamic_static_AVX = measure_time_gauss(dynamic_thread, dynamic_thread_static_AVX_task, false, repeat);
        chunk_size =i;
        double t_dynamic_dynamic_AVX = measure_time_gauss(dynamic_thread, dynamic_thread_dynamic_AVX_task, false, repeat);
        double t_static_static_AVX = measure_time_gauss(static_thread, static_thread_static_AVX_task, false, repeat);
        chunk_size = i;
        double t_static_dynamic_AVX = measure_time_gauss(static_thread, static_thread_dynamic_AVX_task, false, repeat);
        double total = (t_dynamic_static_AVX + t_dynamic_dynamic_AVX + t_static_static_AVX + t_static_dynamic_AVX) * repeat;

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_dynamic_static_AVX, t_dynamic_dynamic_AVX, t_static_static_AVX, t_static_dynamic_AVX, total);
        if (i < 4) {
            i *= 2;
        }
        else {
            i += 4;
        }
    }

    printf("\n动态线程（左两列） - 静态线程（右两列）    分配粒度：64    线程数量：8\n");
    printf("使用较优粒度和较优线程数，比较静态分配线程和动态分配线程\n");
    printf("%-10s %-12s %-12s %-12s %-12s %-12s\n",
        "规模", "静态AVX", "动态AVX", "静态AVX", "动态AVX", "原总用时");
    thread_num = 8;
    for (int i = 128; i <=2560 ; i += 128){
        n = i;

        double t_dynamic_static_AVX = measure_time_gauss(dynamic_thread, dynamic_thread_static_AVX_task, false, repeat);
        chunk_size = 64;
        double t_dynamic_dynamic_AVX = measure_time_gauss(dynamic_thread, dynamic_thread_dynamic_AVX_task, false, repeat);
        double t_static_static_AVX = measure_time_gauss(static_thread, static_thread_static_AVX_task, false, repeat);
        chunk_size = 64;
        double t_static_dynamic_AVX = measure_time_gauss(static_thread, static_thread_dynamic_AVX_task, false, repeat);
        double total = (t_dynamic_static_AVX + t_dynamic_dynamic_AVX + t_static_static_AVX + t_static_dynamic_AVX) * repeat;

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_dynamic_static_AVX, t_dynamic_dynamic_AVX, t_static_static_AVX, t_static_dynamic_AVX, total);
    }
}
