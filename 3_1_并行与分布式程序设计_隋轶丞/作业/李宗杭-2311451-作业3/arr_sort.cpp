// arr_sort.cpp
#include <iostream>
#include <algorithm>
#include <vector>
#include <time.h>
#include <pthread.h>
#include <windows.h>
#include "arr_sort.h"
#include "utils.h"
using namespace std;

int arr_num = 10000;                  // 数组个数
int arr_len = 10000;                    // 每个数组长度
int block_size =50;                      // 分配粒度
int seg;                                        // 静态分块大小
vector<vector<int>> arr;          // 待排序数组
LARGE_INTEGER freq;

// 初始化数组（模拟负载不均衡场景）
void init_arr() {
    seg = arr_num / thread_num;
    arr.resize(arr_num);
    srand(unsigned(time(NULL)));
    int ratio;                                           // 比例
    for (int i = 0; i < arr_num; i++) {
        arr[i].resize(arr_len);
        // 分四段设置不同有序度
        if (i < seg) ratio = 0;                     // 完全升序
        else if (i < seg * 2) ratio = 32;      // 1/4升序
        else if (i < seg * 3) ratio = 64;      // 1/2升序
        else ratio = 128;                           // 完全逆序
        if ((rand() & 127) < ratio) {
            for (int j = 0; j < arr_len; j++)   // 升序数组
                arr[i][j] = j;
        } else {
            for (int j = 0; j < arr_len; j++)   // 降序数组
                arr[i][j] = arr_len - j;
        }
    }
}

// 静态划分（负载不均衡）
void *arr_sort_static(void *parm) {
    int r = *(int *)parm;
    // 每个线程处理seg个数组
    for (int i = r * seg; i < (r + 1) * seg; i++) {
        stable_sort(arr[i].begin(), arr[i].end());
    }
    pthread_exit(NULL);
    return NULL;
}

// 动态划分（负载均衡）
void *arr_sort_dynamic(void *parm) {
    int r = *(int *)parm;
    int task;
    while (1) {
        pthread_mutex_lock(&mutex_task);
        task = next_row++;
        pthread_mutex_unlock(&mutex_task);
        if (task >= arr_num) break;
        stable_sort(arr[task].begin(), arr[task].end());
    }
    pthread_exit(NULL);
    return NULL;
}

// 动态划分（每次分配block_size个任务）
void *arr_sort_coarse(void *parm) {
    int r = *(int *)parm;
    int task_start;
    while (1) {
        pthread_mutex_lock(&mutex_task);
        task_start = next_row;
        next_row += block_size;
        pthread_mutex_unlock(&mutex_task);
        if (task_start >= arr_num) break;
        int task_end = min(task_start + block_size, arr_num);
        for (int task = task_start; task < task_end; task++) {
            stable_sort(arr[task].begin(), arr[task].end());
        }
    }
    pthread_exit(NULL);
    return NULL;
}

// 测试静态划分
 double test_static() {
    seg = arr_num / thread_num;
    pthread_t threads[thread_num];
    int id[thread_num];
    LARGE_INTEGER head, tail;
    QueryPerformanceCounter(&head);
    for (int i = 0; i < thread_num; i++) {
        id[i] = i;
        pthread_create(&threads[i], NULL, arr_sort_static, &id[i]);
    }
    for (int i = 0; i < thread_num; i++) {
        pthread_join(threads[i], NULL);
    }
    QueryPerformanceCounter(&tail);
    double during = (tail.QuadPart - head.QuadPart) * 1000.0 / freq.QuadPart;
    return during;
}

// 测试动态划分
double test_dynamic() {
    pthread_mutex_init(&mutex_task, NULL);      // 初始化互斥锁
    pthread_t threads[thread_num];
    int id[thread_num];
    LARGE_INTEGER head, tail;
    QueryPerformanceCounter(&head);
    for (int i = 0; i < thread_num; i++) {
        id[i] = i;
        pthread_create(&threads[i], NULL, arr_sort_dynamic, &id[i]);
    }
    for (int i = 0; i < thread_num; i++) {
        pthread_join(threads[i], NULL);
    }
    QueryPerformanceCounter(&tail);
    pthread_mutex_destroy(&mutex_task);
    double during = (tail.QuadPart - head.QuadPart) * 1000.0 / freq.QuadPart;
    return during;
}

// 测试粗粒度动态划分
double test_dynamic_coarse() {
    pthread_mutex_init(&mutex_task, NULL);
    pthread_t threads[thread_num];
    int id[thread_num];
    LARGE_INTEGER head, tail;
    QueryPerformanceCounter(&head);
    for (int i = 0; i < thread_num; i++) {
        id[i] = i;
        pthread_create(&threads[i], NULL, arr_sort_coarse, &id[i]);
    }
    for (int i = 0; i < thread_num; i++) {
        pthread_join(threads[i], NULL);
    }
    QueryPerformanceCounter(&tail);
    pthread_mutex_destroy(&mutex_task);
    double during = (tail.QuadPart - head.QuadPart) * 1000.0 / freq.QuadPart;
    return during;
}

// 计时函数
double measure_time_sort(double (*test_func)(void), int repeat) {
    double sum = 0;
    for (int i = 0; i < repeat; i++) {
        init_arr();
        next_row = 0;
        sum += test_func();
    }
    return sum / repeat;
}

// 测试函数
void test_sort() {
    QueryPerformanceFrequency(&freq);
    int repeat =10;

    printf("数据规模：10000    分配粒度：50\n");
    printf("%-10s %-12s %-12s %-12s %-12s\n",
        "线程数", "静态划分", "动态划分", "动态粗粒", "原总用时");

    for (int i = 2; i <=32 ; i += 2){
        thread_num = i;

        double t_static = measure_time_sort(test_static, repeat);
        double t_dynamic = measure_time_sort(test_dynamic, repeat);
        double t_coarse = measure_time_sort(test_dynamic_coarse, repeat);
        double total = (t_static + t_dynamic + t_coarse) * repeat;

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_static, t_dynamic, t_coarse, total);
    }

    printf("\n数据规模：10000    线程数：32\n");
    thread_num = 32;
    printf("%-10s %-12s %-12s %-12s %-12s\n",
        "分配粒度", "静态划分", "动态划分", "动态粗粒", "原总用时");

    for (int i = 5; i <= 500; i += 5){
        if(i > 25) i += 20;
        block_size = i;

        double t_static = measure_time_sort(test_static, repeat);
        double t_dynamic = measure_time_sort(test_dynamic, repeat);
        double t_coarse = measure_time_sort(test_dynamic_coarse, repeat);
        double total = (t_static + t_dynamic + t_coarse) * repeat;

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_static, t_dynamic, t_coarse, total);
    }

    printf("\n分配粒度：50    线程数：32\n");
    block_size = 50;
    printf("%-10s %-12s %-12s %-12s %-12s\n",
        "数据规模", "静态划分", "动态划分", "动态粗粒", "原总用时");

    for (int i = 5000; i <= 50000; i += 5000){
        arr_num = i;

        double t_static = measure_time_sort(test_static, repeat);
        double t_dynamic = measure_time_sort(test_dynamic, repeat);
        double t_coarse = measure_time_sort(test_dynamic_coarse, repeat);
        double total = (t_static + t_dynamic + t_coarse) * repeat;

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_static, t_dynamic, t_coarse, total);
    }
}
