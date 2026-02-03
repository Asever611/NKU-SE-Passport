// gauss_elimination.h
#ifndef GAUSS_ELIMINATION_H_INCLUDED
#define GAUSS_ELIMINATION_H_INCLUDED

#include <utility>

void init_memory();
void free_memory();
void init_gauss(int n);
void serial_elimination(int n);
void serial_back(int n);
void sse_elimination(int n);
void sse_back(int n);
void avx_elimination(int n);
void avx_back(int n);
std::pair<double, double> measure_time_gauss(void (*f_elimination)(int), void (*f_back)(int), int n, int repeat);
void test_gauss();

#endif // GAUSS_ELIMINATION_H_INCLUDED
