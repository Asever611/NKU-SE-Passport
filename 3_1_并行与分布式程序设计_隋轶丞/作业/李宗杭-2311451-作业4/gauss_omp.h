#ifndef GAUSS_OMP_H_INCLUDED
#define GAUSS_OMP_H_INCLUDED

#include <cstdint>
#include <cmath>

const int MAX_N = 2560;
extern float A[MAX_N][MAX_N];
extern float B[MAX_N];
extern float X[MAX_N];

void init_gauss(int n);
void gauss_serial_avx(int n);
void gauss_omp_static_avx(int n, int thread_count, int chunk);
void gauss_omp_dynamic_avx(int n, int thread_count, int chunk);
void gauss_omp_guided_avx(int n, int thread_count, int chunk);
double measure_time_gauss(void (*test_func)(int, int, int), int n, int thread_count, int chunk, int repeat);
void test_gauss_omp();

#endif // GAUSS_OMP_H_INCLUDED
