// matrix_multiply.h
#ifndef MATRIX_MULTIPLY_H_INCLUDED
#define MATRIX_MULTIPLY_H_INCLUDED
#include "utils.h"

void matrix_multiply_serial(int n);
void matrix_multiply_cache(int n);
void matrix_multiply_neon(int n);
void matrix_multiply_tile_neon(int n, int T);

#endif // MATRIX_MULTIPLY_H_INCLUDED