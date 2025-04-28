#ifndef MATRIX_H
#define MATRIX_H

#include "mmio.h"

typedef struct {
    int M;      // righe
    int N;      // colonne
    int NZ;     // non-zero count
    int* IRP;   // row pointer
    int* JA;    // column indices
    double* AS; // non-zero values
} CSRMatrix;

CSRMatrix* load_matrix_market_to_csr(const char* filename);
void free_csr_matrix(CSRMatrix* mat);
void print_csr_matrix(const CSRMatrix* mat);
#endif
