// barriers.cpp
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <vector>
#include "utils.h"
#include "barriers.h"
using namespace std;
// 屏障对象：条件变量+互斥量
CondMutexBarrier cond_mutex_barrier;
// 屏障对象：信号量
SemaphoreBarrier sem_barrier;

// 初始化屏障：条件变量+互斥量
void init_cond_mutex_barrier(CondMutexBarrier* cond_mutex_barrier, int thread_count) {
    cond_mutex_barrier->thread_count = thread_count;
    cond_mutex_barrier->arrived_count = 0;
    cond_mutex_barrier->cycle = 0;
    pthread_mutex_init(&cond_mutex_barrier->barrier_mutex, NULL);
    pthread_cond_init(&cond_mutex_barrier->barrier_cond, NULL);
}

// 屏障：条件变量+互斥量
void cond_mutex_barrier_wait(CondMutexBarrier* cond_mutex_barrier) {
    pthread_mutex_lock(&cond_mutex_barrier->barrier_mutex);
    int current_cycle = cond_mutex_barrier->cycle;
    cond_mutex_barrier->arrived_count++;

    // 最后一个到达的线程：重置计数器并唤醒所有线程
    if (cond_mutex_barrier->arrived_count == cond_mutex_barrier->thread_count) {
        cond_mutex_barrier->arrived_count = 0;
        cond_mutex_barrier->cycle++;
        pthread_cond_broadcast(&cond_mutex_barrier->barrier_cond);
    }
    // 其他线程：阻塞等待，直到当前循环的所有线程到达
    else {
        while (current_cycle == cond_mutex_barrier->cycle) {
            pthread_cond_wait(&cond_mutex_barrier->barrier_cond, &cond_mutex_barrier->barrier_mutex);
        }
    }
    pthread_mutex_unlock(&cond_mutex_barrier->barrier_mutex);
}

// 销毁屏障：条件变量+互斥量
void destroy_cond_mutex_barrier(CondMutexBarrier* cond_mutex_barrier) {
    pthread_mutex_destroy(&cond_mutex_barrier->barrier_mutex);
    pthread_cond_destroy(&cond_mutex_barrier->barrier_cond);
}


// 初始化屏障：信号量
void init_sem_barrier(SemaphoreBarrier* sem_barrier, int thread_count) {
    sem_barrier->thread_count = thread_count;
    sem_barrier->arrived_count = 0;
    sem_init(&sem_barrier->count_sem, 0, thread_count);
    sem_init(&sem_barrier->barrier_sem, 0, 0);
    pthread_mutex_init(&sem_barrier->barrier_mutex, NULL);
}

// 屏障：信号量
void sem_barrier_wait(SemaphoreBarrier* sem_barrier) {
    // 计数信号量P操作：记录线程到达（每到一个减1）
    sem_wait(&sem_barrier->count_sem);

    pthread_mutex_lock(&sem_barrier->barrier_mutex);
    sem_barrier->arrived_count++;
    // 最后一个到达的线程：释放阻塞信号量（允许所有线程继续）
    if (sem_barrier->arrived_count == sem_barrier->thread_count) {
        sem_barrier->arrived_count = 0;
        // 释放thread_count个信号量，唤醒所有等待线程
        for (int i = 0; i < sem_barrier->thread_count; i++) {
            sem_post(&sem_barrier->barrier_sem);
        }
    }
    pthread_mutex_unlock(&sem_barrier->barrier_mutex);

    // 阻塞信号量P操作：等待所有线程到达
    sem_wait(&sem_barrier->barrier_sem);
    // 支持重复使用：在通过屏障后，将count_sem加回来
    sem_post(&sem_barrier->count_sem);
}

// 销毁屏障：信号量
void destroy_sem_barrier(SemaphoreBarrier* sem_barrier) {
    sem_destroy(&sem_barrier->count_sem);
    sem_destroy(&sem_barrier->barrier_sem);
    pthread_mutex_destroy(&sem_barrier->barrier_mutex);
}

// 测试任务
void* barrier_task(void* parm) {
    threadParam_t* params = (threadParam_t*)parm;

    long long something = 0;
    for (int i = 0; i < 100000; i++) {
        something += i;
    }      // 模拟执行其他逻辑

    if (params->barrier_type == "std_barrier") {
        pthread_barrier_wait((pthread_barrier_t*)params->barrier_instance);
    }
    else if (params->barrier_type == "cond_mutex_barrier") {
        cond_mutex_barrier_wait((CondMutexBarrier*)params->barrier_instance);
    }
    else if (params->barrier_type == "sem_barrier") {
        sem_barrier_wait((SemaphoreBarrier*)params->barrier_instance);
    }

    something = 0;
    for (int i = 0; i < 100000; i++) {
        something += i;
    }

    pthread_exit(NULL);
    return NULL;
}

// 测试任务主函数
void barrier_main(string barrier_type, int thread_count) {
    vector<pthread_t> thread_handles(thread_count);
    vector<threadParam_t> params(thread_count);

    if (barrier_type == "std_barrier") {
        pthread_barrier_init(&barrier, NULL, thread_count);
    }
    else if (barrier_type == "cond_mutex_barrier") {
        init_cond_mutex_barrier(&cond_mutex_barrier, thread_count);
    }
    else if (barrier_type == "sem_barrier") {
        init_sem_barrier(&sem_barrier, thread_count);
    }

    for (int i = 0; i < thread_count; i++) {
        params[i].t_id = i;
        params[i].barrier_type = barrier_type;
        if (barrier_type == "std_barrier") {
            params[i].barrier_instance = &barrier;
        }
        else if (barrier_type == "cond_mutex_barrier") {
            params[i].barrier_instance = &cond_mutex_barrier;
        }
        else if (barrier_type == "sem_barrier") {
            params[i].barrier_instance = &sem_barrier;
        }
        pthread_create(&thread_handles[i], NULL, barrier_task, (void*)&params[i]);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(thread_handles[i], NULL);
    }

    if (barrier_type == "std_barrier") {
        pthread_barrier_destroy(&barrier);
    }
    else if (barrier_type == "cond_mutex_barrier") {
        destroy_cond_mutex_barrier(&cond_mutex_barrier);
    }
    else if (barrier_type == "sem_barrier") {
        destroy_sem_barrier(&sem_barrier);
    }
}

// 计时函数
double measure_time_barrier(string barrier_type, int thread_count, int repeat) {
    double sum = 0;
    for (int i = 0; i < repeat; i++) {
        sum -= get_time();
        barrier_main(barrier_type, thread_count);
        sum += get_time();
    }
    return sum / repeat * 1000.0;
}

// 测试函数
void test_barrier() {
    int repeat = 10;

    printf("%-10s %-12s %-12s %-12s %-12s\n",
        "线程数", "标准屏障", "条件变量+互斥锁", "信号量",  "原总用时");
    for (int i = 2; i <=32 ; i += 2){
        thread_num = i;

        double t_std = measure_time_barrier("std_barrier", thread_num, repeat);
        double t_cond_mutex = measure_time_barrier("cond_mutex_barrier", thread_num, repeat);
        double t_sem = measure_time_barrier("sem_barrier", thread_num, repeat);
        double total = (t_std + t_cond_mutex + t_sem) * repeat;

        printf("%-10d %-12.5g %-12.5g %-12.5g %-12.5g\n",
               i, t_std, t_cond_mutex, t_sem, total);
    }
}
