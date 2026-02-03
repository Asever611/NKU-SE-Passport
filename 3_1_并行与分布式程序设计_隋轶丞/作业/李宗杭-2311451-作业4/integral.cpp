// integral.cpp
#include <pthread.h>
#include <omp.h>
#include <windows.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include "integral.h"
#include "utils.h"
using namespace std;

int next_task = 1;
double result = 0.0;

// Pthread线程参数结构体
typedef struct {
    double a;           // 积分区间左端点
    double h;           // 步长
    int n;                  // 总分割数
    int chunk;          // 动态分配粒度
    int thread_id;     // 当前线程的ID（从0开始）
    int thread_count;        // 总线程数
    double local_result;    // 当前线程计算的局部积分结果
} PthreadTrapParam;

// 被积函数：f(x) = x² + 2x + 1
double f(double x) {
    return x * x + 2 * x + 1;
}

// 串行梯形积分法
double trap_serial(double a, double b, int n) {
    double h = (b - a) / n;
    double approx = (f(a) + f(b)) / 2.0;
    for (int i = 1; i <= n - 1; i++) {
        double x = a + i * h;
        approx += f(x);
    }
    return approx * h;
}

// Pthread线程函数-静态分配任务
void* trap_pthread_thread_static(void* param) {
    PthreadTrapParam* p = (PthreadTrapParam*)param;
    double a = p->a;
    double h = p->h;
    int n = p->n;
    int thread_id = p->thread_id;
    int thread_count = p->thread_count;

    // 划分当前线程的局部任务
    int local_n = (n +thread_count - 1) / thread_count;
    int left = thread_id * local_n +1;
    int right = min(left + local_n - 1, n - 1);

    // 局部积分计算
    double local_result = 0.0;
    for (int i = left; i <= right; i++) {
        double x = a + i * h;
        local_result += f(x);
    }

    // 记录局部结果
    p->local_result = local_result;

    pthread_exit(NULL);
    return NULL;
}

// Pthread线程函数-动态分配任务
void* trap_pthread_thread_dynamic(void* param) {
    PthreadTrapParam* p = (PthreadTrapParam*)param;
    double a = p->a;
    double h = p->h;
    int n = p->n;
    int chunk = p->chunk;
    int thread_id = p->thread_id;
    int thread_count = p->thread_count;

    int task;
    double local_result = 0.0;

    while (true) {
        // 动态获取任务块
        pthread_mutex_lock(&task_mutex);
        task = next_task;
        next_task += chunk;
        pthread_mutex_unlock(&task_mutex);
        if (task >= n) {
            break;
        }

        // 划分当前线程的局部任务
        int local_n = chunk;
        int left = task;
        int right = min(left + local_n - 1, n - 1);

        // 局部积分计算
        for (int i = left; i <= right; i++) {
            double x = a + i * h;
            local_result += f(x);
        }
    }

    // 记录局部结果
    p->local_result = local_result;

    pthread_exit(NULL);
    return NULL;
}

// Pthread并行梯形积分法
double trap_pthread(double a, double b, int n, int thread_count, int chunk) {
    vector<pthread_t> threads(thread_count);
    vector<PthreadTrapParam> params(thread_count);
    pthread_mutex_init(&task_mutex, NULL);
    double global_result = (f(a) + f(b)) / 2.0;     // 全局积分结果
    double h = (b - a) / n;
    next_task = 1;

    // 初始化线程参数，创建线程
    for (int i = 0; i < thread_count; i++) {
        params[i].a = a;
        params[i].h = h;
        params[i].n = n;
        params[i].chunk = chunk;
        params[i].thread_id = i;
        params[i].thread_count = thread_count;
        if (chunk == 0) {
            pthread_create(&threads[i], NULL, trap_pthread_thread_static, &params[i]);
        }
        else {
            pthread_create(&threads[i], NULL, trap_pthread_thread_dynamic, &params[i]);
        }
    }

    // 等待所有线程完成，汇总局部结果
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
        global_result += params[i].local_result;
    }

    pthread_mutex_destroy(&task_mutex);
    return global_result * h;
}

// OpenMP并行梯形积分法-静态调度
double trap_omp_static(double a, double b, int n, int thread_count) {
    double h = (b - a) / n;
    double approx = (f(a) + f(b)) / 2.0;

    #pragma omp parallel for num_threads(thread_count) schedule(static) reduction(+:approx)
    for (int i = 1; i <= n - 1; i++) {
        double x = a + i * h;
        approx += f(x);
    }

    return approx * h;
}

// OpenMP并行梯形积分法-动态调度
double trap_omp_dynamic(double a, double b, int n, int thread_count, int chunk) {
    double h = (b - a) / n;
    double approx = (f(a) + f(b)) / 2.0;

    #pragma omp parallel for num_threads(thread_count) schedule(dynamic, chunk) reduction(+:approx)
    for (int i = 1; i <= n - 1; i++) {
        double x = a + i * h;
        approx += f(x);
    }

    return approx * h;
}

// OpenMP并行梯形积分法-引导调度
double trap_omp_guided(double a, double b, int n, int thread_count) {
    double h = (b - a) / n;
    double approx = (f(a) + f(b)) / 2.0;

    #pragma omp parallel for num_threads(thread_count) schedule(guided) reduction(+:approx)
    for (int i = 1; i <= n - 1; i++) {
        double x = a + i * h;
        approx += f(x);
    }

    return approx * h;
}

// 计时函数
double measure_time_integral(double a, double b, int n, string type, int repeat, int thread_count, int chunk) {
    double sum = 0.0;
    sum -= get_time();
    for (int i = 0; i < repeat; i++) {
        if (type == "serial") {
            result = trap_serial(a, b, n);
        }
        else if (type == "pthread") {
            result = trap_pthread(a, b, n, thread_count, chunk);
        }
        else if (type == "omp_static") {
            result = trap_omp_static(a, b, n, thread_count);
        }
        else if (type == "omp_dynamic") {
            result = trap_omp_dynamic(a, b, n, thread_count, chunk);
        }
        else if (type == "omp_guided") {
            result = trap_omp_guided(a, b, n, thread_count);
        }
    }
    sum += get_time();
    return sum * 1000 / repeat;
}

// 测试函数
void test_integral() {
    double a = 0.0, b = 10.0;
    int n = 1000000;
    int thread_count = 4;
    int repeat = 10;

    printf("%-10s %-12s %-12s %-12s %-12s %-12s %-12s\n",
        "线程数", "串行", "pthread静态", "pthread动态", "omp静态", "omp动态", "omp引导");
    double t_serial = measure_time_integral(a, b, n, "serial");
    double r_serial = result;
    for (int i = 2; i <= 32; i += 2) {
        thread_count = i;

        double t_pthread_static = measure_time_integral(a, b, n, "pthread", 10, thread_count, 0);
        double r_pthread_static = result;

        double t_pthread_dynamic = measure_time_integral(a, b, n, "pthread", 10, thread_count, 1);
        double r_pthread_dynamic = result;

        double t_omp_static = measure_time_integral(a, b, n, "omp_static", 10, thread_count);
        double r_omp_static = result;

        double t_omp_dynamic = measure_time_integral(a, b, n, "omp_dynamic", 10, thread_count, 1);
        double r_omp_dynamic = result;

        double t_omp_guided = measure_time_integral(a, b, n, "omp_guided", 10, thread_count);
        double r_omp_guided = result;

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_serial, t_pthread_static, t_pthread_dynamic, t_omp_static, t_omp_dynamic, t_omp_guided);
        if (0) {
            // 展示计算结果，用于查看各算法逻辑是否正确
            printf("%-10d %-12.10g %-12.10g %-12.10g %-12.10g %-12.10g %-12.10g\n",
               i, r_serial, r_pthread_static, r_pthread_dynamic, r_omp_static, r_omp_dynamic, r_omp_guided);
        }
    }

    printf("\n%-10s %-12s %-12s %-12s %-12s %-12s %-12s\n",
        "动态分块", "串行", "pthread静态", "pthread动态", "omp静态", "omp动态", "omp引导");
    thread_count = 16;
    for (int i = 1; i <= 4096; i *= 2) {
        int chunk = i;

        double t_pthread_static = measure_time_integral(a, b, n, "pthread", 10, thread_count, 0);
        double r_pthread_static = result;

        double t_pthread_dynamic = measure_time_integral(a, b, n, "pthread", 10, thread_count, chunk);
        double r_pthread_dynamic = result;

        double t_omp_static = measure_time_integral(a, b, n, "omp_static", 10, thread_count);
        double r_omp_static = result;

        double t_omp_dynamic = measure_time_integral(a, b, n, "omp_dynamic", 10, thread_count, chunk);
        double r_omp_dynamic = result;

        double t_omp_guided = measure_time_integral(a, b, n, "omp_guided", 10, thread_count);
        double r_omp_guided = result;

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_serial, t_pthread_static, t_pthread_dynamic, t_omp_static, t_omp_dynamic, t_omp_guided);
        if (0) {
            printf("%-10d %-12.10g %-12.10g %-12.10g %-12.10g %-12.10g %-12.10g\n",
               i, r_serial, r_pthread_static, r_pthread_dynamic, r_omp_static, r_omp_dynamic, r_omp_guided);
        }
    }
}
