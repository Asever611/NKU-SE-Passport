// matrix_multiply_mpi.cpp
#include "matrix_multiply_mpi.h"
#include <cstdlib>
#include <cstdio>
#include <algorithm>
using namespace std;

// 初始化矩阵
void init_matrix(int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a[i][j] = (float)rand() / RAND_MAX;
            b[i][j] = (float)rand() / RAND_MAX;
            c[i][j] = 0.0f;
        }
    }
}

// 静态任务分配-主从模型
void matrix_multiply_mpi_static(int n) {
    int local_rows = (n + size - 1) / size;
    int left = my_rank * local_rows;
    int right = min(n, left + local_rows);

    for (int i = left; i < right; i++) {
        for (int j = 0; j < n; j++) {
            float sum = 0.0f;
            for (int k = 0; k < n; k++) {
                sum += a[i][k] * b[k][j];
            }
            c[i][j] = sum;
        }
    }

    if (my_rank == 0) {
        for (int i = 1; i < size; i++) {
            int slave_left = i * local_rows;
            int slave_right = min(n, slave_left + local_rows);
            MPI_Recv(c[slave_left], n * (slave_right - slave_left), MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    } else {
        MPI_Send(c[left], n * (right - left), MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }
}

// 动态任务分配-主从模型
void matrix_multiply_mpi_dynamic(int n, int chunk) {
    MPI_Status status;

    if (my_rank == 0) {
        int task = 0;
        int finished = 1;

        for (int i = 1; i < size; i++) {
            if (task < n) {
                MPI_Send(&task, 1, MPI_INT, i, task, MPI_COMM_WORLD);
                task += chunk;
            } 
            else {  // 若初始阶段就分配完
                int term = n;
                MPI_Send(&term, 1, MPI_INT, i, n, MPI_COMM_WORLD); // 发送结束信号
                finished++;
            }
        }

        while (finished < size) {
            int task_id;
            // 接收任务id
            MPI_Recv(&task_id, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG < n) {
                // 接收计算结果
                MPI_Recv(c[task_id], n * min(chunk, n - task_id), MPI_FLOAT, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            }
            
            if (task < n) {
                MPI_Send(&task, 1, MPI_INT, status.MPI_SOURCE, task, MPI_COMM_WORLD);
                task += chunk;
            } 
            else {  // 发送结束信号
                MPI_Send(&task, 1, MPI_INT, status.MPI_SOURCE, n, MPI_COMM_WORLD);
                finished++;
            }
        }
    }
    else {
        int task = 0;
        while (true) {
            MPI_Recv(&task, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (task >= n) break;

            int task_end = min(task + chunk, n);
            for (int i = task; i < task_end; i++) {
                for (int j = 0; j < n; j++) {
                    float sum = 0.0f;
                    for (int k = 0; k < n; k++) {
                        sum += a[i][k] * b[k][j];
                    }
                    c[i][j] = sum;
                }
            }

            MPI_Send(&task, 1, MPI_INT, 0, status.MPI_TAG, MPI_COMM_WORLD);
            MPI_Send(c[task], n * (task_end - task), MPI_FLOAT, 0, status.MPI_TAG, MPI_COMM_WORLD);
        }
    }
}

// 计时函数
double measure_time_matrix_multiply(int type, int n, int chunk, int repeat) {
    double start_time, end_time, total_time = 0.0;
    for (int i = 0; i < repeat; i++) {
        init_matrix(n);
        start_time = MPI_Wtime();
        if (type == 0 || type == 1) {
            MPI_Bcast(a, n * n, MPI_FLOAT, 0, MPI_COMM_WORLD);
            MPI_Bcast(b, n * n, MPI_FLOAT, 0, MPI_COMM_WORLD);
        }
        if (type == 0) {
            matrix_multiply_mpi_static(n);
        } 
        else if (type == 1) {
            matrix_multiply_mpi_dynamic(n, chunk);
        }
        else if (type == 2) {
            matrix_multiply_serial(n);
        }
        else if (type == 3) {
            matrix_multiply_cache(n);
        }
        else if (type == 4) {
            matrix_multiply_neon(n);
        }
        else if (type == 5) {
            matrix_multiply_tile_neon(n, chunk);
        }
        end_time = MPI_Wtime();
        total_time += (end_time - start_time);
    }
    return (total_time / repeat) * 1000;
}

// 测试函数
void test_matrix_multiply_mpi() {
    int n = 1024;
    double time_static, time_dynamic, time_coarse;
    double time_serial, time_cache, time_neon, time_tile;
    int repeat = 10;
    int chunk = DEFAULT_CHUNK;

    if (my_rank == 0) {
        printf("===============矩阵乘法===============\n");
        printf("\n%-10s %-12s %-12s %-12s %-12s %-12s %-12s %-12s\n",
            "动态分块", "串行", "缓存", "NEON", "分片", "静态", "动态", "粗粒度");
    }
    // 不受变量影响的仅测试一次，降低实验耗时
    time_serial = measure_time_matrix_multiply(2, n, 1, repeat);
    time_cache = measure_time_matrix_multiply(3, n, 1, repeat);
    time_neon = measure_time_matrix_multiply(4, n, 1, repeat);
    time_static = measure_time_matrix_multiply(0, n, 1, repeat);
    time_dynamic = measure_time_matrix_multiply(1, n, 1, repeat);
    for (int i = 2; i <= 256; i *= 2) {
        chunk = i;

        time_tile = measure_time_matrix_multiply(5, n, chunk, repeat);        
        time_coarse = measure_time_matrix_multiply(1, n, chunk, repeat);

        if (my_rank == 0) {
            printf("%-10d %-12.4f %-12.4f %-12.4f %-12.4f %-12.4f %-12.4f %-12.4f\n",
                chunk, time_serial, time_cache, time_neon, time_tile, time_static, time_dynamic, time_coarse);
        }
    }

    chunk = 32; // 选取较优粒度
    if (my_rank == 0) {
        printf("\n%-10s %-12s %-12s %-12s %-12s %-12s %-12s %-12s\n",
            "规模", "串行", "缓存", "NEON", "分片", "静态", "动态", "粗粒度");
    }
    // 相比于第二次作业，降低规模数量，以减少实验总时长，且当前规模足以体现差异
    for (int i = 128; i <= 1280; i += 128) {
        n = i;

        time_serial = measure_time_matrix_multiply(2, n, 1, repeat);
        time_cache = measure_time_matrix_multiply(3, n, 1, repeat);
        time_neon = measure_time_matrix_multiply(4, n, 1, repeat);
        time_tile = measure_time_matrix_multiply(5, n, chunk, repeat);
        time_static = measure_time_matrix_multiply(0, n, 1, repeat);
        time_dynamic = measure_time_matrix_multiply(1, n, 1, repeat);
        time_coarse = measure_time_matrix_multiply(1, n, chunk, repeat);

        if (my_rank == 0) {
            printf("%-10d %-12.4f %-12.4f %-12.4f %-12.4f %-12.4f %-12.4f %-12.4f\n",
                n, time_serial, time_cache, time_neon, time_tile, time_static, time_dynamic, time_coarse);
        }
    }
}