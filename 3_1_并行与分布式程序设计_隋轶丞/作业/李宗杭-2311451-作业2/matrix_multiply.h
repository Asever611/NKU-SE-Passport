// matrix_multiply.h
#ifndef MATRIX_MULTIPLY_H_INCLUDED
#define MATRIX_MULTIPLY_H_INCLUDED

void init_matrix(int n);
void matrix_multiply_serial(int n);
void matrix_multiply_cache(int n);
void matrix_multiply_sse(int n);
void matrix_multiply_tile_sse(int n, int T);
double measure_time(void (*mul_func)(int), int n, int repeat);
double measure_time(void (*mul_func)(int, int), int n, int T, int repeat);
void test_mul();

#endif // MATRIX_MULTIPLY_H_INCLUDED
