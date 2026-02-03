// integral_mpi.cpp
#include "integral_mpi.h"
#include <cmath>
#include <cstdio>
#include <algorithm>
using namespace std;

// 被积函数：f(x) = x^2 + 2x + 1
double f(double x) {
    return x * x + 2 * x + 1;
}

// 静态任务分配-对等模型
void integral_mpi_static(double a, double b, int n, double& result) {
    double h = (b - a) / n;
    double local_sum = 0.0;
    int local_n = (n + size - 1) / size;
    int left = my_rank * local_n + 1;
    int right = min(n - 1, left + local_n - 1);

    for (int i = left; i <= right; i++) {
        double x = a + i * h;
        local_sum += f(x);
    }

    double global_sum = 0.0;
    double init_val = (f(a) + f(b)) / 2.0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (my_rank == 0) {
        result = (init_val + global_sum) * h;
    }
}

// 动态任务分配-主从模型
void integral_mpi_dynamic(double a, double b, int n, int chunk, double& result) {
    double h = (b - a) / n;
    MPI_Status status;

    if (my_rank == 0) {
        double global_sum = 0.0;
        int task = 0;       // 待分配任务
        int finished = 1;   // 已完成进程数
        double recv_sum = 0.0;

        // 分配初始任务
        for (int i = 1; i < size; i++) {
            if (task < n) {
                MPI_Send(&task, 1, MPI_INT, i, task, MPI_COMM_WORLD);
                task += chunk;
            } 
            else {  // 若初始阶段就分配完
                MPI_Send(&task, 1, MPI_INT, i, n, MPI_COMM_WORLD); // 发送结束信号
                finished++;
            }
        }

        // 接收结果并分配新任务
        while (finished < size) {
            MPI_Recv(&recv_sum, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG < n) {
                global_sum += recv_sum;
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

        // 计算最终结果
        double init_val = (f(a) + f(b)) / 2.0;
        result = (init_val + global_sum) * h;
    } 
    else {
        int task = 0;
        double local_sum = 0.0;

        while (true) {
            MPI_Recv(&task, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG >= n) break;

            int local_right = min(task + chunk - 1, n - 1);
            local_sum = 0.0;
            for (int i = task; i <= local_right; i++) {
                double x = a + i * h;
                local_sum += f(x);
            }
            MPI_Send(&local_sum, 1, MPI_DOUBLE, 0, status.MPI_TAG, MPI_COMM_WORLD);
        }
    }
}

// 计时函数
double measure_time_integral(int type, double a, double b, int n, int chunk, double& result, int repeat) {
    double start_time, end_time;
    start_time = MPI_Wtime();
    for (int i = 0; i < repeat; i++) {
        if (type == 0) {
            integral_mpi_static(a, b, n, result);
        } 
        else if (type == 1) {
            integral_mpi_dynamic(a, b, n, chunk, result);
        }
    }
    end_time = MPI_Wtime();
    return (end_time - start_time) * 1000 / repeat;
}

// 测试函数
void test_integral_mpi() {
    double a = 0.0, b = 10.0;
    int n = 1000000;
    double result_static = 0.0, result_dynamic = 0.0, result_coarse = 0.0;
    int repeat = 10;
    int chunk = DEFAULT_CHUNK;

    // 单独测试计算结果
    integral_mpi_static(a, b, n, result_static);
    integral_mpi_dynamic(a, b, n, 1, result_dynamic);
    integral_mpi_dynamic(a, b, n, chunk, result_coarse);

    if (my_rank == 0) {
        printf("===============梯形积分法===============\n");
        printf("%-10s %-12.10g %-12.10g %-12.10g\n", 
            "计算结果", result_static, result_dynamic, result_coarse);
    }

    if (my_rank == 0) {
        printf("\n%-10s %-12s %-12s %-12s\n",
            "动态分块", "静态", "动态", "粗粒度");
    }
    // 不受变量影响的仅测试一次，降低实验耗时
    double time_static = measure_time_integral(0, a, b, n, 1, result_static, repeat);
    double time_dynamic = measure_time_integral(1, a, b, n, 1, result_dynamic, repeat);
    for (int i = 2; i <= 4096; i *= 2) {
        chunk = i;

        double time_coarse = measure_time_integral(1, a, b, n, chunk, result_coarse, repeat);

        if (my_rank == 0) {
            printf("%-10d %-12.4f %-12.4f %-12.4f\n",
                chunk, time_static, time_dynamic, time_coarse);
        }
    }

    if (my_rank == 0) {
        printf("\n%-10s %-12s %-12s %-12s\n",
            "规模", "静态", "动态", "粗粒度");
    }
    for (int i = 10000; i <= 10000000; i *= 10) {
        n = i;

        double time_static = measure_time_integral(0, a, b, n, 1, result_static, repeat);
        double time_dynamic = measure_time_integral(1, a, b, n, 1, result_dynamic, repeat);
        double time_coarse = measure_time_integral(1, a, b, n, chunk, result_coarse, repeat);

        if (my_rank == 0) {
            printf("%-10d %-12.4f %-12.4f %-12.4f\n",
                n, time_static, time_dynamic, time_coarse);
        }
    }
}