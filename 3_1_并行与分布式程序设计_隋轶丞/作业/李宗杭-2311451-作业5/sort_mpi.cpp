// sort_mpi.cpp
#include "sort_mpi.h"
#include <algorithm>
#include <time.h>
#include <cstdio>
#include <vector>
using namespace std;

vector<vector<int>> arrays;         // 待排序数组

// 初始化数组（模拟负载不均衡）
void init_arrays(int arr_num, int arr_len) {
    arrays.resize(arr_num);
    srand(unsigned(time(NULL)));
    int seg = arr_num / 4;

    for (int i = 0; i < arr_num; i++) {
        arrays[i].resize(arr_len);
        int ratio;
        if (i < seg) ratio = 0;
        else if (i < seg * 2) ratio = 32;
        else if (i < seg * 3) ratio = 64;
        else ratio = 128;

        if ((rand() & 127) < ratio) {
            for (int j = 0; j < arr_len; j++) arrays[i][j] = j;
        } 
        else {
            for (int j = 0; j < arr_len; j++) arrays[i][j] = arr_len - j;
        }
    }
}

// 静态任务分配-主从模型
void sort_mpi_static(int arr_num, int arr_len) {
    int local_num = (arr_num + size - 1) / size;
    int left = my_rank * local_num;
    int right = min(arr_num, left + local_num);

    for (int i = left; i < right; i++) {
        stable_sort(arrays[i].begin(), arrays[i].end());
    }

    if (my_rank == 0) {
        for (int i = 1; i < size; i++) {
            int slave_left = i * local_num;
            int slave_right = min(arr_num, slave_left + local_num);
            // vector地址不连续，需逐条接收内层数组
            for (int j = slave_left; j < slave_right; j++) {
                MPI_Recv(arrays[j].data(), arr_len, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    } 
    else {
        
        for (int i = left; i < right; i++) {
            MPI_Send(arrays[i].data(), arr_len, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }
}

// 动态任务分配-主从模型
void sort_mpi_dynamic(int arr_num, int arr_len, int chunk) {
    MPI_Status status;

    if (my_rank == 0) {
        int task = 0;
        int finished = 1;

        for (int i = 1; i < size; i++) {
            if (task < arr_num) {
                MPI_Send(&task, 1, MPI_INT, i, task, MPI_COMM_WORLD);
                task += chunk;
            } 
            else {  // 若初始阶段就分配完
                MPI_Send(&task, 1, MPI_INT, i, arr_num, MPI_COMM_WORLD); // 发送结束信号
                finished++;
            }
        }

        while (finished < size) {
            int task_id;
            // 接收任务id
            MPI_Recv(&task_id, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG < arr_num) {
                // 接收排序结果
                for (int i = 0; i < min(chunk, arr_num - task_id); i++) {
                    MPI_Recv(arrays[task_id + i].data(), arr_len, MPI_INT, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                }
            }

            if (task < arr_num) {
                MPI_Send(&task, 1, MPI_INT, status.MPI_SOURCE, task, MPI_COMM_WORLD);
                task += chunk;
            } 
            else {  // 发送结束信号
                MPI_Send(&task, 1, MPI_INT, status.MPI_SOURCE, arr_num, MPI_COMM_WORLD);
                finished++;
            }
        }
    }
    else {
        int task = 0;
        while (true) {
            MPI_Recv(&task, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG >= arr_num) break;

            for (int i = 0; i < chunk && task + i < arr_num; i++) {
                sort(arrays[task + i].begin(), arrays[task + i].end());
            }
            MPI_Send(&task, 1, MPI_INT, 0, status.MPI_TAG, MPI_COMM_WORLD);
            
            for (int i = 0; i < min(chunk, arr_num - task); i++) {
                MPI_Send(arrays[task + i].data(), arr_len, MPI_INT, 0, status.MPI_TAG, MPI_COMM_WORLD);
            }
        }
    }
}

// 计时函数
double measure_time_sort(int type, int arr_num, int arr_len, int chunk, int repeat) {
    double start_time, end_time, total_time = 0.0;
    for (int i = 0; i < repeat; i++) {
        init_arrays(arr_num, arr_len);
        start_time = MPI_Wtime();
        for (int i = 0; i < arr_num; i++) {
            MPI_Bcast(arrays[i].data(), arr_len, MPI_INT, 0, MPI_COMM_WORLD);
        }
        if (type == 0) {
            sort_mpi_static(arr_num, arr_len);
        } 
        else if (type == 1) {
            sort_mpi_dynamic(arr_num, arr_len, chunk);
        }
        end_time = MPI_Wtime();
        total_time += (end_time - start_time);
    }
    return (total_time / repeat) * 1000;
}

void test_sort_mpi() {
    int arr_num = 10000, arr_len = 10000;
    double time_static, time_dynamic, time_coarse;
    int repeat = 5;
    int chunk = DEFAULT_CHUNK;

    if (my_rank == 0) {
        printf("===============多个数组排序===============\n");
        printf("%-10s %-12s %-12s %-12s\n",
            "动态分块", "静态", "动态", "粗粒度");
    }
    // 不受变量影响的仅测试一次，降低实验耗时
    time_static = measure_time_sort(0, arr_num, arr_len, 1, repeat);
    time_dynamic = measure_time_sort(1, arr_num, arr_len, 1, repeat);
    for (int i = 2; i <= 2048; i *= 2) {
        chunk = i;

        time_coarse = measure_time_sort(1, arr_num, arr_len, chunk, repeat);

        if (my_rank == 0) {
            printf("%-10d %-12.4f %-12.4f %-12.4f\n",
                chunk, time_static, time_dynamic, time_coarse);
        }
    }

    chunk = 4; // 选取较优粒度
    if (my_rank == 0) {
        printf("\n%-10s %-12s %-12s %-12s\n",
            "规模", "静态", "动态", "粗粒度");
    }
    for (int i = 2500; i <= 20000; i += 2500) {
        arr_num = i;

        time_static = measure_time_sort(0, arr_num, arr_len, 1, repeat);
        time_dynamic = measure_time_sort(1, arr_num, arr_len, 1, repeat);
        time_coarse = measure_time_sort(1, arr_num, arr_len, chunk, repeat);

        if (my_rank == 0) {
            printf("%-10d %-12.4f %-12.4f %-12.4f\n",
                arr_num, time_static, time_dynamic, time_coarse);
        }
    }
}