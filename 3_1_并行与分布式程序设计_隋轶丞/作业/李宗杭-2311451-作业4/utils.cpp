// utils.cpp
#include <pthread.h>
#include <windows.h>
#include "utils.h"
using namespace std;

int thread_num = 8;
int next_row = 0;
pthread_mutex_t task_mutex;
pthread_barrier_t barrier;

double get_time() {
    LARGE_INTEGER freq, curr;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&curr);
    return (double)curr.QuadPart / freq.QuadPart;
}
