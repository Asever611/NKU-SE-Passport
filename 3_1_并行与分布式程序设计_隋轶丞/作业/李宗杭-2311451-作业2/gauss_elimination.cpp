// gauss_elimination.cpp
#include "gauss_elimination.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <windows.h>
#include <cmath>
#include <utility>
using namespace std;

float A[MAX_N][MAX_N];
float B[MAX_N];
float X[MAX_N];

// 初始化线性方程组（对角线占优，方程组有唯一解）
void init_gauss(int n) {
    for (int i = 0; i < n; ++i) {
        float sum = 0.0f;
        for (int j = 0; j < n; ++j) {
            A[i][j] = (float)rand() / RAND_MAX;
            sum += fabs(A[i][j]);
        }
        A[i][i] = sum + 1.0f;  // 对角线占优
        B[i] = (float)rand() / RAND_MAX * 100;
    }
}

// 串行高斯消元
void serial_elimination(int n) {
    for (int k = 0; k < n - 1; ++k) {
        for (int i = k + 1; i < n; ++i) {
            float factor = A[i][k] / A[k][k];
            for (int j = k; j < n; ++j) {
                A[i][j] -= factor * A[k][j];
            }
            B[i] -= factor * B[k];
        }
    }
}

// 串行回代
void serial_back(int n) {
    X[n - 1] = B[n - 1] / A[n - 1][n - 1];
    for (int i = n - 2; i >= 0; --i) {
        float sum = B[i];
        for (int j = i + 1; j < n; ++j) {
            sum -= A[i][j] * X[j];
        }
        X[i] = sum / A[i][i];
    }
}

// SSE高斯消元
void sse_elimination(int n) {
    for (int k = 0; k < n - 1; ++k) {
        for (int i = k + 1; i < n; ++i) {
            float factor = A[i][k] / A[k][k];
            int j = k;
            __m128 vec_factor = _mm_set1_ps(factor);
            for (; j <= n - 4; j += 4) {
                __m128 vec_A_k = _mm_loadu_ps(A[k] + j);
                __m128 vec_A_i = _mm_loadu_ps(A[i] + j);
                vec_A_i = _mm_sub_ps(vec_A_i, _mm_mul_ps(vec_factor, vec_A_k));
                _mm_storeu_ps(A[i] + j, vec_A_i);
            }
            // 剩余元素
            for (; j < n; ++j) {
                A[i][j] -= factor * A[k][j];
            }
            B[i] -= factor * B[k];
        }
    }
}

// SSE回代
void sse_back(int n) {
    X[n - 1] = B[n - 1] / A[n - 1][n - 1];
    for (int i = n - 2; i >= 0; --i) {
        float sum = B[i];
        __m128 vec_sum = _mm_set1_ps(sum);
        int j = i + 1;
        for (; j <= n - 4; j += 4) {
            __m128 vec_A_i = _mm_loadu_ps(A[i] + j);
            __m128 vec_x_j = _mm_loadu_ps(X + j);
            vec_sum = _mm_sub_ps(vec_sum, _mm_mul_ps(vec_A_i, vec_x_j));
        }
        // 累加结果
        vec_sum = _mm_hadd_ps(vec_sum, vec_sum);
        vec_sum = _mm_hadd_ps(vec_sum, vec_sum);
        _mm_storeu_ps(&sum, vec_sum);
        // 剩余元素
        for (; j < n; ++j) {
            sum -= A[i][j] * X[j];
        }
        X[i] = sum / A[i][i];
    }
}

// AVX高斯消元
void avx_elimination(int n) {
    for (int k = 0; k < n - 1; ++k) {
        for (int i = k + 1; i < n; ++i) {
            float factor = A[i][k] / A[k][k];
            int j = k;
            __m256 vec_factor = _mm256_set1_ps(factor);
            for (; j <= n - 8; j += 8) {
                __m256 vec_A_k = _mm256_loadu_ps(A[k] + j);
                __m256 vec_A_i = _mm256_loadu_ps(A[i] + j);
                vec_A_i = _mm256_sub_ps(vec_A_i, _mm256_mul_ps(vec_factor, vec_A_k));
                _mm256_storeu_ps(A[i] + j, vec_A_i);
            }
            // 处理剩余元素
            for (; j < n; ++j) {
                A[i][j] -= factor * A[k][j];
            }
            B[i] -= factor * B[k];
        }
    }
}

// AVX回代
void avx_back(int n) {
    X[n - 1] = B[n - 1] / A[n - 1][n - 1];
    for (int i = n - 2; i >= 0; --i) {
        float sum = B[i];
        __m256 vec_sum = _mm256_set1_ps(sum);
        int j = i + 1;
        for (; j <= n - 8; j += 8) {
            __m256 vec_A_i = _mm256_loadu_ps(A[i] + j);
            __m256 vec_x_j = _mm256_loadu_ps(X + j);
            vec_sum = _mm256_sub_ps(vec_sum, _mm256_mul_ps(vec_A_i, vec_x_j));
        }
        // 累加向量结果
        __m128 s1 = _mm256_extractf128_ps(vec_sum, 0);
        __m128 s2 = _mm256_extractf128_ps(vec_sum, 1);
        s1 = _mm_hadd_ps(s1, s2);
        s1 = _mm_hadd_ps(s1, s1);
        _mm_storeu_ps(&sum, s1);
        // 处理剩余元素
        for (; j < n; ++j) {
            sum -= A[i][j] * X[j];
        }
        X[i] = sum / A[i][i];
    }
}

// 计算时间（非连续多次测量取平均）
pair<double, double> measure_time_gauss(void (*f_elimination)(int), void (*f_back)(int), int n, int repeat) {
    double start, middle, finish;
    double t_elimination = 0, t_back = 0;
    for (int t = 0; t < repeat; ++t) {
        init_gauss(n);
        start = get_time();
        f_elimination(n);
        middle = get_time();
        f_back(n);
        finish = get_time();
        t_elimination += middle - start;
        t_back += finish - middle;
    }
    return {t_elimination / repeat, t_back / repeat}; // 返回平均时间
}

// 测试函数
void test_gauss() {
    const int REPEAT = 20;
    double back_result[8][6];
    pair<double, double> result;

    printf("消元\n");
    printf("%-10s %-10s %-10s %-10s %-10s %-10s\n",
        "规模", "串行", "SSE", "AVX", "SSE提升", "AVX提升");
    int k = 0;
    for (int i = 8; i <= 1024; i *= 2){
        back_result[k][0] = i;
        // 串行
        result = measure_time_gauss(serial_elimination, serial_back, i, REPEAT);
        double t_serial_elimination = result.first;
        back_result[k][1] = result.second;
        // SSE
        result = measure_time_gauss(sse_elimination, sse_back, i, REPEAT);
        double t_sse_elimination = result.first;
        back_result[k][2] = result.second;
        // AVX
        result = measure_time_gauss(avx_elimination, avx_back, i, REPEAT);
        double t_avx_elimination = result.first;
        back_result[k][3] = result.second;
        // 消元效率提升百分比
        double sse_improve = (t_serial_elimination - t_sse_elimination) / t_serial_elimination * 100;
        double avx_improve = (t_serial_elimination - t_avx_elimination) / t_serial_elimination * 100;
        // 回代效率提升百分比
        back_result[k][4] = (back_result[k][1] - back_result[k][2]) / back_result[k][1] * 100;
        back_result[k][5] = (back_result[k][1] - back_result[k][3]) / back_result[k][1] * 100;
        k++;
        char sse_str[20], avx_str[20];
        sprintf(sse_str, "%.2f%%", sse_improve);
        sprintf(avx_str, "%.2f%%", avx_improve);
        printf("%-10d %-10.5g %-10.5g %-10.5g %-10s %-10s\n",
            i, t_serial_elimination, t_sse_elimination, t_avx_elimination, sse_str, avx_str);
    }

    printf("\n回代\n");
    printf("%-10s %-10s %-10s %-10s %-10s %-10s\n",
        "规模", "串行", "SSE", "AVX", "SSE提升", "AVX提升");

    for (int i = 0; i < 8; i++) {
        char sse_back_str[20], avx_back_str[20];
        sprintf(sse_back_str, "%.2f%%", back_result[i][4]);
        sprintf(avx_back_str, "%.2f%%", back_result[i][5]);
        printf("%-10d %-10.5g %-10.5g %-10.5g %-10s %-10s\n",
            (int)back_result[i][0], back_result[i][1], back_result[i][2], back_result[i][3], sse_back_str, avx_back_str);
    }
    printf("测试结束");
}
