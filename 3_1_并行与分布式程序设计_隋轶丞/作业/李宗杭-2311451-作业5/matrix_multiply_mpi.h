// matrix_multiply_mpi.h
#ifndef MATRIX_MULTIPLY_MPI_H_INCLUDED
#define MATRIX_MULTIPLY_MPI_H_INCLUDED
#include "utils.h"
#include "matrix_multiply.h"

void init_matrix(int n);
void matrix_multiply_mpi_static(int n);
void matrix_multiply_mpi_dynamic(int n, int chunk);
double measure_time_matrix_multiply(int type, int n, int chunk, int repeat);
void test_matrix_multiply_mpi();

#endif // MATRIX_MULTIPLY_MPI_H_INCLUDED