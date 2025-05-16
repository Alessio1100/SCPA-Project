#include <stdio.h>
#include <stdlib.h>
#include "include/CSR_Matrix.h"
#include "include/HLL_Matrix.h"

void csr_serial_mat_per_vec(CSRMatrix *csr_matrix, double *x, double *y){
    for (int i = 0; i < csr_matrix->M; i++) {
        double sum = 0.0;
        for (int j = csr_matrix->IRP[i]; j < csr_matrix->IRP[i+1]; j++) {
            sum += csr_matrix->AS[j] * x[csr_matrix->JA[j]];
        }
        y[i] = sum;
    }
}

void hll_serial_mat_per_vec(HLLMatrix *hll_matrix, const double *x, double *y) {
    int global_row_offset = 0;

    for (int b = 0; b < hll_matrix->num_blocks; b++) {
        HLLBlock block = hll_matrix->blocks[b];
        int rows_in_block = block.rows_in_block;
        int max_nz = block.max_nz_per_row;

        for (int i = 0; i < rows_in_block; i++) {
            double sum = 0.0;
            for (int j = 0; j < max_nz; j++) {
                int idx = j * rows_in_block + i;  // ELLPACK column-major access
                int col = block.JA[idx];
                double val = block.AS[idx];
                sum += val * x[col];
            }
            y[global_row_offset + i] = sum;
        }

        global_row_offset += rows_in_block;
    }
}

void print_vector(double *y, int size) {
    for (int i = 0; i < size; i++) {
        printf("%.6lf ", y[i]);
    }
    printf("\n");
}
