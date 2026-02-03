// barriers.h
#ifndef BARRIERS_H_INCLUDED
#define BARRIERS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string>
using namespace std;
typedef struct {
    pthread_mutex_t barrier_mutex;
    pthread_cond_t barrier_cond;
    int thread_count;
    int arrived_count;
    int cycle;
} CondMutexBarrier;
typedef struct {
    sem_t count_sem;
    sem_t barrier_sem;
    int thread_count;
    int arrived_count;
    pthread_mutex_t barrier_mutex;
} SemaphoreBarrier;

void init_cond_mutex_barrier(CondMutexBarrier* cond_mutex_barrier, int thread_count);
void cond_mutex_barrier_wait(CondMutexBarrier* cond_mutex_barrier);
void destroy_cond_mutex_barrier(CondMutexBarrier* cond_mutex_barrier);
void init_sem_barrier(SemaphoreBarrier* sem_barrier, int thread_count);
void sem_barrier_wait(SemaphoreBarrier* sem_barrier);
void destroy_sem_barrier(SemaphoreBarrier* sem_barrier);
void* barrier_task(void* parm);
void barrier_main(string barrier_type, int thread_count);
double measure_time_barrier(string barrier_type, int thread_count, int repeat);
void test_barrier();

#endif // BARRIERS_H_INCLUDED
