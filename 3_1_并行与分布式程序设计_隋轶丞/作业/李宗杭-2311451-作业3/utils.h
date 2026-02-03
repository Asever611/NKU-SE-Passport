// utils.h
#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <pthread.h>
#include <windows.h>
#include <string>
using namespace std;
extern int thread_num;
extern int next_row;
extern pthread_mutex_t mutex_task;
extern pthread_barrier_t barrier;
struct threadParam_t {
    int k;
    int t_id;
    string barrier_type;
    void* barrier_instance;            // ∆¡’œ µ¿˝÷∏’Î
};

double get_time();

#endif // UTILS_H_INCLUDED
