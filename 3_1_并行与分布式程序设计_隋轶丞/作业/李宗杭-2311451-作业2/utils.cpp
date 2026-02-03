// utils.cpp
#include "utils.h"
using namespace std;

double get_time() {
    LARGE_INTEGER freq, curr;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&curr);
    return (double)curr.QuadPart / freq.QuadPart;
}
