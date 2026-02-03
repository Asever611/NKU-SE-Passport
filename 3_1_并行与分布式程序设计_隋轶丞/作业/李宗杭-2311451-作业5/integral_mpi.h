// integral_mpi.h
#ifndef INTEGRAL_MPI_H_INCLUDED
#define INTEGRAL_MPI_H_INCLUDED

#include <mpi.h>
#include "utils.h"

double f(double x);
void integral_mpi_static(double a, double b, int n, double& result);
void integral_mpi_dynamic(double a, double b, int n, int chunk, double& result);
double measure_time_integral(int type, double a, double b, int n, int chunk, double& result, int repeat);
void test_integral_mpi();

#endif // INTEGRAL_MPI_H_INCLUDED