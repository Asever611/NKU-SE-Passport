// utils.h
#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <mpi.h>
using namespace std;

// 全局配置参数
const int MAX_N = 2048;         // 矩阵最大规模
const int DEFAULT_CHUNK = 50;   // 粗粒度分配粒度
extern int my_rank;             // 当前进程编号
extern int size;                // 总进程数
extern MPI_Comm comm;           // 通信域
extern float a[MAX_N][MAX_N], b[MAX_N][MAX_N], c[MAX_N][MAX_N], b_t[MAX_N][MAX_N];

#endif // UTILS_H_INCLUDED
