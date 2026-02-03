// matrix_multiply.cpp
#include "matrix_multiply.h"
#include <stdio.h>
#include <stdlib.h>
#include <arm_neon.h>
using namespace std;

// 1. 串行算法
void matrix_multiply_serial(int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

// 2. Cache优化（矩阵转置）
void matrix_multiply_cache(int n) {
    // 转置矩阵b，将列访问转为行访问
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            b_t[i][j] = b[j][i];
        }
    }
    // 行访问优化
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            c[i][j] = 0.0f;
            for (int k = 0; k < n; ++k) {
                c[i][j] += a[i][k] * b_t[j][k];
            }
        }
    }
}

// 3. NEON版本
void matrix_multiply_neon(int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            b_t[i][j] = b[j][i];
        }
    }
    float32x4_t vec_a, vec_b, vec_sum;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            c[i][j] = 0.0f;
            vec_sum = vdupq_n_f32(0.0f);
            // 批量处理4个元素
            for (int k = n - 4; k > 0; k -= 4) {
                vec_a = vld1q_f32(a[i] + k);
                vec_b = vld1q_f32(b_t[j] + k);
                vec_a = vmulq_f32(vec_a, vec_b);
                vec_sum = vaddq_f32(vec_sum, vec_a);
            }
            // 水平累加结果
            float32x2_t sum_low = vadd_f32(vget_low_f32(vec_sum), vget_high_f32(vec_sum));
            float32x2_t sum_final = vpadd_f32(sum_low, sum_low);
            c[i][j] += vget_lane_f32(sum_final, 0);
            // 处理剩余元素
            for (int k = n % 4 - 1; k > 0; k--) {
                c[i][j] += a[i][k] * b_t[j][k];
            }
        }
    }
}

// 4. 分片策略
void matrix_multiply_tile_neon(int n, int T) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            b_t[i][j] = b[j][i];
        }
    }
    float32x4_t vec_a, vec_b, vec_sum;
    float tmp;
    // 按分片大小遍历
    for (int r = 0; r < n / T; r ++) {
        for (int q = 0; q < n / T; q ++) {
            // 初始化当前子矩阵
            for (int i = 0; i < T; ++i) {
                    for (int j = 0; j < T; ++j) {
                        c[r * T + i][q * T + j] = 0.0f;
                    }
            }
            // 处理子矩阵
            for (int p = 0; p <  n / T; p ++) {
                for (int i = 0; i < T; ++i) {
                    for (int j = 0; j < T; ++j) {
                        vec_sum = vdupq_n_f32(0.0f);
                        for (int k = 0; k < T; k += 4) {
                            vec_a = vld1q_f32(&a[r*T + i][p*T + k]);
                            vec_b = vld1q_f32(&b_t[q*T + j][p*T + k]);
                            vec_a = vmulq_f32(vec_a, vec_b);
                            vec_sum = vaddq_f32(vec_sum, vec_a);
                        }
                        // 累加结果
                        float32x2_t sum_low = vadd_f32(vget_low_f32(vec_sum), vget_high_f32(vec_sum));
                        float32x2_t sum_final = vpadd_f32(sum_low, sum_low);
                        tmp = vget_lane_f32(sum_final, 0);
                        c[r*T + i][q*T + j] += tmp;
                    }
                }
            }
        }
    }
}
