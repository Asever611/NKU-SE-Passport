// sort_omp.h
#ifndef SORT_OMP_H_INCLUDED
#define SORT_OMP_H_INCLUDED

#include <vector>
#include <string>

extern int arr_num;
extern int arr_len;
extern std::vector<std::vector<int>> arrays;
extern std::vector<std::vector<int>> arrays_copy;

void init_arrays();
void sort_serial();
void sort_omp_static(int thread_count, int chunk);
void sort_omp_dynamic(int thread_count, int chunk);
void sort_omp_guided(int thread_count, int chunk);
double measure_time_sort(void (*test_func)(int, int), int thread_count, int chunk, int repeat);
void test_sort_omp();

#endif // SORT_OMP_H_INCLUDED
