// matrix_multiply.cpp
#include "matrix_multiply.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pmmintrin.h>
#include <windows.h>
using namespace std;

float a[MAX_N][MAX_N], b[MAX_N][MAX_N], c[MAX_N][MAX_N], b_t[MAX_N][MAX_N];

// �����ʼ������
void init_matrix(int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            a[i][j] = (float)rand() / RAND_MAX;
            b[i][j] = (float)rand() / RAND_MAX;
            c[i][j] = 0.0f;
        }
    }
}

// 1. �����㷨
void matrix_multiply_serial(int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

// 2. Cache�Ż�������ת�ã�
void matrix_multiply_cache(int n) {
    // ת�þ���b�����з���תΪ�з���
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            b_t[i][j] = b[j][i];
        }
    }
    // �з����Ż�
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            c[i][j] = 0.0f;
            for (int k = 0; k < n; ++k) {
                c[i][j] += a[i][k] * b_t[j][k];
            }
        }
    }
}

// 3. SSE�汾��128λ���������δ���4��float��
void matrix_multiply_sse(int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            b_t[i][j] = b[j][i];
        }
    }
    __m128 vec_a, vec_b, vec_sum;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            c[i][j] = 0.0f;
            vec_sum = _mm_setzero_ps();
            // ��������4��Ԫ��
            for (int k = n - 4; k > 0; k -= 4) {
                vec_a = _mm_loadu_ps(a[i] + k);
                vec_b = _mm_loadu_ps(b_t[j] + k);
                vec_a = _mm_mul_ps(vec_a, vec_b);
                vec_sum = _mm_add_ps(vec_sum, vec_a);
            }
            // ˮƽ�ۼӽ��
            vec_sum = _mm_hadd_ps(vec_sum, vec_sum);
            vec_sum = _mm_hadd_ps(vec_sum, vec_sum);
            _mm_store_ss(&c[i][j], vec_sum);
            // ����ʣ��Ԫ��
            for (int k = n % 4 - 1; k > 0; k--) {
                c[i][j] += a[i][k] * b_t[j][k];
            }
        }
    }
}

// 4. ��Ƭ����
void matrix_multiply_tile_sse(int n, int T) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            b_t[i][j] = b[j][i];
        }
    }
    __m128 vec_a, vec_b, vec_sum;
    float tmp;
    // ����Ƭ��С����
    for (int r = 0; r < n / T; r ++) {
        for (int q = 0; q < n / T; q ++) {
            // ��ʼ����ǰ�Ӿ���
            for (int i = 0; i < T; ++i) {
                    for (int j = 0; j < T; ++j) {
                        c[r * T + i][q * T + j] = 0.0f;
                    }
            }
            // �����Ӿ���
            for (int p = 0; p <  n / T; p ++) {
                for (int i = 0; i < T; ++i) {
                    for (int j = 0; j < T; ++j) {
                        vec_sum = _mm_setzero_ps();
                        for (int k = 0; k < T; k += 4) {
                            vec_a = _mm_loadu_ps(&a[r*T + i][p*T + k]);
                            vec_b = _mm_loadu_ps(&b_t[q*T + j][p*T + k]);
                            vec_a = _mm_mul_ps(vec_a, vec_b);
                            vec_sum = _mm_add_ps(vec_sum, vec_a);
                        }
                        // �ۼӽ��
                        vec_sum = _mm_hadd_ps(vec_sum, vec_sum);
                        vec_sum = _mm_hadd_ps(vec_sum, vec_sum);
                        _mm_store_ss(&tmp, vec_sum);
                        c[r*T + i][q*T + j] += tmp;
                    }
                }
            }
        }
    }
}

// ����ʱ��
double measure_time(void (*mul_func)(int), int n, int repeat) {
    double start = get_time();
    for (int t = 0; t < repeat; ++t) {
        mul_func(n);
    }
    return (get_time() - start) / repeat; // ����ƽ��ʱ��
}

double measure_time(void (*mul_func)(int, int), int n, int T, int repeat) {
    double start = get_time();
    for (int t = 0; t < repeat; ++t) {
        mul_func(n, T);
    }
    return (get_time() - start) / repeat;
}

// ���Ժ���
void test_mul() {
    const int REPEAT = 20;

    printf("%-10s %-10s %-10s %-10s %-10s %-10s\n",
        "��ģ", "�����㷨", "Cache�Ż�", "SSE�汾", "��ƬSSE", "ԭ��ʱ��");

    for (int i = 128; i <= 2048; i += 128){
        init_matrix(i);

        double t_serial = measure_time(matrix_multiply_serial, i, REPEAT);
        double t_cache = measure_time(matrix_multiply_cache, i, REPEAT);
        double t_sse = measure_time(matrix_multiply_sse, i, REPEAT);
        double t_tile = measure_time(matrix_multiply_tile_sse, i, 64, REPEAT);
        double total = (t_serial + t_cache + t_sse + t_tile) * 20;

        printf("%-10d %-10.5g %-10.5g %-10.5g %-10.5g %-10.5g\n",
               i, t_serial, t_cache, t_sse, t_tile, total);
    }

    printf("\n%-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n",
        "��ģ", "8", "16", "32", "64", "128", "256", "ԭ��ʱ��");

    for (int i = 256; i <= 2048; i *= 2) {
        init_matrix(i);
        printf("%-10d", i);
        double total = 0;
        for (int j = 8; j <= 256; j *= 2) {
            double avg_time = measure_time(matrix_multiply_tile_sse, i, j, REPEAT);
            printf(" %-10.5g", avg_time);
            total += avg_time;
        }
        printf(" %-10.5g\n", total * 20);
    }
    printf("���Խ���");
}
