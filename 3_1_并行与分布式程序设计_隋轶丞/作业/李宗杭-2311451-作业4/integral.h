// integral.h
#ifndef INTEGRAL_H_INCLUDED
#define INTEGRAL_H_INCLUDED

#include <pthread.h>
#include <string>

extern int next_task;
extern double result;

double f(double x);
double trap_serial(double a, double b, int n);
double trap_pthread(double a, double b, int n, int thread_count = 4, int chunk = 0);
double trap_omp_static(double a, double b, int n, int thread_count);
double trap_omp_dynamic(double a, double b, int n, int thread_count, int chunk);
double trap_omp_guided(double a, double b, int n, int thread_count);
double measure_time_integral(double a, double b, int n, std::string type, int repeat = 10, int thread_count = 4, int chunk = 0);
void test_integral();

#endif // INTEGRAL_H_INCLUDED
