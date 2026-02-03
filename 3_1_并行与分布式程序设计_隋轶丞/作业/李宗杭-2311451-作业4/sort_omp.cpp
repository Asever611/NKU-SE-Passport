// sort_omp.cpp
#include <iostream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include "sort_omp.h"
#include "utils.h"
using namespace std;

int arr_num = 10000;
int arr_len = 10000;
vector<vector<int>> arrays;          // 待排序数组
vector<vector<int>> arrays_copy;    // 数组副本，用于控制变量

// 初始化数组（模拟负载不均衡）
void init_arrays() {
    arrays.resize(arr_num);
    srand(unsigned(time(NULL)));
    int seg = arr_num / 4; // 分为4段，有序度依次降低

    for (int i = 0; i < arr_num; i++) {
        arrays[i].resize(arr_len);
        int ratio;
        if (i < seg) ratio = 0;     // 完全升序
        else if (i < 2 * seg) ratio = 32; // 1/4逆序
        else if (i < 3 * seg) ratio = 64; // 1/2逆序
        else ratio = 128;           // 完全逆序

        if ((rand() & 127) < ratio) {
            for (int j = 0; j < arr_len; j++) arrays[i][j] = j;
        } else {
            for (int j = 0; j < arr_len; j++) arrays[i][j] = arr_len - j;
        }
    }
}

// 串行排序
void sort_serial() {
    for (int i = 0; i < arr_num; i++) {
        stable_sort(arrays_copy[i].begin(), arrays_copy[i].end());
    }
}

// OpenMP静态调度排序
void sort_omp_static(int thread_count, int chunk) {
    if (chunk == 0) {
        #pragma omp parallel for num_threads(thread_count) schedule(static)
        for (int i = 0; i < arr_num; i++) {
            stable_sort(arrays_copy[i].begin(), arrays_copy[i].end());
        }
    }
    else {
        #pragma omp parallel for num_threads(thread_count) schedule(static, chunk)
        for (int i = 0; i < arr_num; i++) {
            stable_sort(arrays_copy[i].begin(), arrays_copy[i].end());
        }
    }
}

// OpenMP动态调度排序
void sort_omp_dynamic(int thread_count, int chunk) {
    if (chunk == 0) {
        #pragma omp parallel for num_threads(thread_count) schedule(dynamic)
        for (int i = 0; i < arr_num; i++) {
            stable_sort(arrays_copy[i].begin(), arrays_copy[i].end());
        }
    }
    else {
        #pragma omp parallel for num_threads(thread_count) schedule(dynamic, chunk)
        for (int i = 0; i < arr_num; i++) {
            stable_sort(arrays_copy[i].begin(), arrays_copy[i].end());
        }
    }
}

// OpenMP引导调度排序
void sort_omp_guided(int thread_count, int chunk) {
    if (chunk == 0) {
        #pragma omp parallel for num_threads(thread_count) schedule(guided)
        for (int i = 0; i < arr_num; i++) {
            stable_sort(arrays_copy[i].begin(), arrays_copy[i].end());
        }
    }
    else {
        #pragma omp parallel for num_threads(thread_count) schedule(guided, chunk)
        for (int i = 0; i < arr_num; i++) {
            stable_sort(arrays_copy[i].begin(), arrays_copy[i].end());
        }
    }
}

// 排序计时函数
double measure_time_sort(void (*test_func)(int, int), int thread_count, int chunk, int repeat) {
    double sum = 0.0;

    for (int i = 0; i < repeat; i++) {
        arrays_copy = arrays;
        sum -= get_time();
        test_func(thread_count, chunk);
        sum += get_time();
    }

    return sum * 1000 / repeat;
}

// 测试函数
void test_sort_omp() {
    int repeat = 5;
    int thread_count = 4;
    int chunk = 0;
    double t_serial;
    init_arrays();

    printf("数据规模：10000    chunk：无\n");
    printf("%-10s %-12s %-12s %-12s %-12s\n",
        "线程数", "串行", "静态", "动态", "引导");

    arrays_copy = arrays;
    t_serial = get_time();
    sort_serial();
    t_serial = get_time() - t_serial;
    t_serial *= 1000;
    for (int i = 2; i <=32 ; i += 2){
        thread_count = i;

        double t_static = measure_time_sort(sort_omp_static, thread_count, chunk, repeat);
        double t_dynamic = measure_time_sort(sort_omp_dynamic, thread_count, chunk, repeat);
        double t_guided = measure_time_sort(sort_omp_guided, thread_count, chunk, repeat);

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_serial, t_static, t_dynamic, t_guided);
    }

    printf("\n数据规模：10000    线程数：32\n");
    thread_count = 32;
    printf("%-10s %-12s %-12s %-12s %-12s\n",
        "分块大小", "串行", "静态", "动态", "引导");

    for (double i = 0; i <= 4096; i *= 2){
        chunk = (int)i;

        double t_static = measure_time_sort(sort_omp_static, thread_count, chunk, repeat);
        double t_dynamic = measure_time_sort(sort_omp_dynamic, thread_count, chunk, repeat);
        double t_guided = measure_time_sort(sort_omp_guided, thread_count, chunk, repeat);

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g\n",
               (int)i, t_serial, t_static, t_dynamic, t_guided);

        if (i == 0) i = 0.5;
    }

    printf("\nchunk：0    线程数：32\n");
    chunk = 0;
    printf("%-10s %-12s %-12s %-12s %-12s\n",
        "数据规模", "串行", "静态", "动态", "引导");

    for (int i = 5000; i <= 50000; i += 5000){
        arr_num = i;
        init_arrays();
        arrays_copy = arrays;
        t_serial = get_time();
        sort_serial();
        t_serial = get_time() - t_serial;
        t_serial *= 1000;
        double t_static = measure_time_sort(sort_omp_static, thread_count, chunk, repeat);
        double t_dynamic = measure_time_sort(sort_omp_dynamic, thread_count, chunk, repeat);
        double t_guided = measure_time_sort(sort_omp_guided, thread_count, chunk, repeat);

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_serial, t_static, t_dynamic, t_guided);
    }

}
