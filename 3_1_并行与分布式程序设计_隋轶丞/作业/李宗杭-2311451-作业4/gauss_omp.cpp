// gauss_omp.cpp
#include "gauss_omp.h"
#include <iostream>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <immintrin.h>
#include <omp.h>
#include "utils.h"
using namespace std;

float A[MAX_N][MAX_N];
float B[MAX_N];
float X[MAX_N];

// 初始化线性方程组（对角线占优，确保有解）
void init_gauss(int n) {
    srand(unsigned(time(NULL)));
    for (int i = 0; i < n; i++) {
        float sum = 0.0f;
        for (int j = 0; j < n; j++) {
            A[i][j] = (float)rand() / RAND_MAX;
            sum += fabs(A[i][j]);
        }
        A[i][i] = sum + 1.0f; // 对角线占优
        B[i] = (float)rand() / RAND_MAX * 100.0f;
    }
}

// 串行
void gauss_serial_avx(int n) {
    for (int k = 0; k < n - 1; k++) {
        for (int i = k + 1; i < n; i++) {
            float factor = A[i][k] / A[k][k];
            int j = k;
            __m256 vec_factor = _mm256_set1_ps(factor);

            for (; j <= n - 8; j += 8) {
                __m256 vec_Ak = _mm256_loadu_ps(A[k] + j);
                __m256 vec_Ai = _mm256_loadu_ps(A[i] + j);
                vec_Ai = _mm256_sub_ps(vec_Ai, _mm256_mul_ps(vec_factor, vec_Ak));
                _mm256_storeu_ps(A[i] + j, vec_Ai);
            }

            for (; j < n; j++) {
                A[i][j] -= factor * A[k][j];
            }
            B[i] -= factor * B[k];
        }
    }
}

// OpenMP静态调度
void gauss_omp_static_avx(int n, int thread_count, int chunk) {
    #pragma omp parallel num_threads(thread_count)
    {
        for (int k = 0; k < n - 1; k++) {
            #pragma omp for schedule(static)
            for (int i = k + 1; i < n; i++) {
                float factor = A[i][k] / A[k][k];
                int j = k;
                __m256 vec_factor = _mm256_set1_ps(factor);

                for (; j <= n - 8; j += 8) {
                    __m256 vec_Ak = _mm256_loadu_ps(A[k] + j);
                    __m256 vec_Ai = _mm256_loadu_ps(A[i] + j);
                    vec_Ai = _mm256_sub_ps(vec_Ai, _mm256_mul_ps(vec_factor, vec_Ak));
                    _mm256_storeu_ps(A[i] + j, vec_Ai);
                }

                for (; j < n; j++) {
                    A[i][j] -= factor * A[k][j];
                }
                B[i] -= factor * B[k];
            }
        }
    }
}

// OpenMP动态调度
void gauss_omp_dynamic_avx(int n, int thread_count, int chunk) {
    #pragma omp parallel num_threads(thread_count)
    {
        for (int k = 0; k < n - 1; k++) {
            if (chunk == 0) {
                #pragma omp for schedule(dynamic)
                for (int i = k + 1; i < n; i++) {
                    float factor = A[i][k] / A[k][k];
                    int j = k;
                    __m256 vec_factor = _mm256_set1_ps(factor);

                    for (; j <= n - 8; j += 8) {
                        __m256 vec_Ak = _mm256_loadu_ps(A[k] + j);
                        __m256 vec_Ai = _mm256_loadu_ps(A[i] + j);
                        vec_Ai = _mm256_sub_ps(vec_Ai, _mm256_mul_ps(vec_factor, vec_Ak));
                        _mm256_storeu_ps(A[i] + j, vec_Ai);
                    }

                    for (; j < n; j++) {
                        A[i][j] -= factor * A[k][j];
                    }
                    B[i] -= factor * B[k];
                }
            }
            else {
                #pragma omp for schedule(dynamic, chunk)
                for (int i = k + 1; i < n; i++) {
                    float factor = A[i][k] / A[k][k];
                    int j = k;
                    __m256 vec_factor = _mm256_set1_ps(factor);

                    for (; j <= n - 8; j += 8) {
                        __m256 vec_Ak = _mm256_loadu_ps(A[k] + j);
                        __m256 vec_Ai = _mm256_loadu_ps(A[i] + j);
                        vec_Ai = _mm256_sub_ps(vec_Ai, _mm256_mul_ps(vec_factor, vec_Ak));
                        _mm256_storeu_ps(A[i] + j, vec_Ai);
                    }

                    for (; j < n; j++) {
                        A[i][j] -= factor * A[k][j];
                    }
                    B[i] -= factor * B[k];
                }
            }
        }
    }
}

// OpenMP引导调度
void gauss_omp_guided_avx(int n, int thread_count, int chunk) {
    #pragma omp parallel num_threads(thread_count)
    {
        for (int k = 0; k < n - 1; k++) {
            if (chunk == 0) {
                #pragma omp for schedule(guided)
                for (int i = k + 1; i < n; i++) {
                    float factor = A[i][k] / A[k][k];
                    int j = k;
                    __m256 vec_factor = _mm256_set1_ps(factor);

                    for (; j <= n - 8; j += 8) {
                        __m256 vec_Ak = _mm256_loadu_ps(A[k] + j);
                        __m256 vec_Ai = _mm256_loadu_ps(A[i] + j);
                        vec_Ai = _mm256_sub_ps(vec_Ai, _mm256_mul_ps(vec_factor, vec_Ak));
                        _mm256_storeu_ps(A[i] + j, vec_Ai);
                    }

                    for (; j < n; j++) {
                        A[i][j] -= factor * A[k][j];
                    }
                    B[i] -= factor * B[k];
                }
            }
            else {
                #pragma omp for schedule(guided, chunk)
                for (int i = k + 1; i < n; i++) {
                    float factor = A[i][k] / A[k][k];
                    int j = k;
                    __m256 vec_factor = _mm256_set1_ps(factor);

                    for (; j <= n - 8; j += 8) {
                        __m256 vec_Ak = _mm256_loadu_ps(A[k] + j);
                        __m256 vec_Ai = _mm256_loadu_ps(A[i] + j);
                        vec_Ai = _mm256_sub_ps(vec_Ai, _mm256_mul_ps(vec_factor, vec_Ak));
                        _mm256_storeu_ps(A[i] + j, vec_Ai);
                    }

                    for (; j < n; j++) {
                        A[i][j] -= factor * A[k][j];
                    }
                    B[i] -= factor * B[k];
                }
            }
        }
    }
}

// 计时函数
double measure_time_gauss(void (*test_func)(int, int, int), int n, int thread_count, int chunk, int repeat) {
    double sum = 0.0;

    for (int i = 0; i < repeat; i++) {
        init_gauss(n);
        sum -= get_time();
        test_func(n, thread_count, chunk);
        sum += get_time();
    }

    return sum * 1000 / repeat;
}

// 测试函数
void test_gauss_omp() {
    int repeat = 5;
    int thread_count = 4;
    int chunk = 0;
    int n = 1024;
    double t_serial;

    printf("数据规模：10000    chunk：无\n");
    printf("%-10s %-12s %-12s %-12s %-12s\n",
        "线程数", "串行", "静态", "动态", "引导");

    init_gauss(n);
    t_serial = get_time();
    gauss_serial_avx(n);
    t_serial = get_time() - t_serial;
    t_serial *= 1000;
    for (int i = 2; i <=32 ; i += 2){
        thread_count = i;

        double t_static = measure_time_gauss(gauss_omp_static_avx, n, thread_count, chunk, repeat);
        double t_dynamic = measure_time_gauss(gauss_omp_dynamic_avx, n, thread_count, chunk, repeat);
        double t_guided = measure_time_gauss(gauss_omp_guided_avx, n, thread_count, chunk, repeat);

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_serial, t_static, t_dynamic, t_guided);
    }

    printf("\n数据规模：10000    线程数：8\n");
    thread_count = 8;
    printf("%-10s %-12s %-12s %-12s %-12s\n",
        "分块大小", "串行", "静态", "动态", "引导");

    for (double i = 0; i <= 128; i *= 2){
        chunk = (int)i;

        double t_static = measure_time_gauss(gauss_omp_static_avx, n, thread_count, chunk, repeat);
        double t_dynamic = measure_time_gauss(gauss_omp_dynamic_avx, n, thread_count, chunk, repeat);
        double t_guided = measure_time_gauss(gauss_omp_guided_avx, n, thread_count, chunk, repeat);

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g\n",
               (int)i, t_serial, t_static, t_dynamic, t_guided);

        if (i == 0) i = 0.5;
    }

    printf("\nchunk：0    线程数：8\n");
    chunk = 0;
    printf("%-10s %-12s %-12s %-12s %-12s\n",
        "数据规模", "串行", "静态", "动态", "引导");

    for (int i = 128; i <= 2560; i += 128){
        n = i;
        init_gauss(n);
        t_serial = get_time();
        gauss_serial_avx(n);
        t_serial = get_time() - t_serial;
        t_serial *= 1000;
        double t_static = measure_time_gauss(gauss_omp_static_avx, n, thread_count, chunk, repeat);
        double t_dynamic = measure_time_gauss(gauss_omp_dynamic_avx, n, thread_count, chunk, repeat);
        double t_guided = measure_time_gauss(gauss_omp_guided_avx, n, thread_count, chunk, repeat);

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_serial, t_static, t_dynamic, t_guided);
    }
}
