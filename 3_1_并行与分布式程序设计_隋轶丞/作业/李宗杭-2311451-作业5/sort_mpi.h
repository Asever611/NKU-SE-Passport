// sort_mpi.h
#ifndef MPI_ARR_SORT_H_INCLUDED
#define MPI_ARR_SORT_H_INCLUDED

#include "mpi.h"
#include "utils.h"

void init_arrays(int arr_num, int arr_len);
void sort_mpi_static(int arr_num, int arr_len);
void sort_mpi_dynamic(int arr_num, int arr_len, int chunk);
double measure_time_sort(int type, int arr_num, int arr_len, int chunk, int repeat);
void test_sort_mpi();

#endif // MPI_ARR_SORT_H_INCLUDED