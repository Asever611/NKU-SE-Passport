// utils.cpp
#include "utils.h"
using namespace std;

int my_rank = 0;
int size = 1;
MPI_Comm comm = MPI_COMM_WORLD;
float a[MAX_N][MAX_N], b[MAX_N][MAX_N], c[MAX_N][MAX_N], b_t[MAX_N][MAX_N];