// arr_sort.h
#ifndef ARR_SORT_H_INCLUDED
#define ARR_SORT_H_INCLUDED

void init_arr();
void *arr_sort_static(void *parm);
void *arr_sort_dynamic(void *parm);
void *arr_sort_coarse(void *parm);
double test_static();
double test_dynamic();
double test_dynamic_coarse();
double measure_time(double (*test_func)(void), int repeat);
void test_sort();

#endif // ARR_SORT_H_INCLUDED
