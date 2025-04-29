#include <stdio.h>
#include <stdlib.h>
#include "include/CSR_Matrix.h"
#include "include/HLL_Matrix.h"

#define HACKSIZE 32

static inline void safe_malloc_check(void* ptr, const char* msg) {
    if (!ptr) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

HLLMatrix* convert_csr_to_hll(const CSRMatrix* csr) {
    int M = csr->M;
    int N = csr->N;
    int num_blocks = (M + HACKSIZE - 1) / HACKSIZE;

    HLLMatrix* hll = malloc(sizeof(HLLMatrix));
    safe_malloc_check(hll, "malloc HLLMatrix");

    hll->M = M;
    hll->N = N;
    hll->HackSize = HACKSIZE;
    hll->num_blocks = num_blocks;
    hll->blocks = malloc(num_blocks * sizeof(HLLBlock));
    safe_malloc_check(hll->blocks, "malloc HLL blocks");

    for (int b = 0; b < num_blocks; ++b) {
        int start = b * HACKSIZE;
        int end = (b + 1) * HACKSIZE;
        if (end > M) end = M;
        int rows_in_block = end - start;

        int max_nz = 0;
        for (int i = start; i < end; ++i) {
            int nzr = csr->IRP[i + 1] - csr->IRP[i];
            if (nzr > max_nz) max_nz = nzr;
        }

        int size = rows_in_block * max_nz;
        int* JA = malloc(size * sizeof(int));
        double* AS = malloc(size * sizeof(double));
        safe_malloc_check(JA, "malloc block JA");
        safe_malloc_check(AS, "malloc block AS");

        for (int i = 0; i < size; ++i) {
            JA[i] = 0;
            AS[i] = 0.0;
        }

        for (int i = start; i < end; ++i) {
            int local_row = i - start;
            int row_start = csr->IRP[i];
            int row_end = csr->IRP[i + 1];
            int nzr = row_end - row_start;

            for (int j = 0; j < nzr; ++j) {
                int idx = j * rows_in_block + local_row;
                JA[idx] = csr->JA[row_start + j];
                AS[idx] = csr->AS[row_start + j];
            }
        }

        hll->blocks[b].rows_in_block = rows_in_block;
        hll->blocks[b].max_nz_per_row = max_nz;
        hll->blocks[b].JA = JA;
        hll->blocks[b].AS = AS;
    }

    return hll;
}

void free_hll_matrix(HLLMatrix* mat) {
    if (!mat) return;
    for (int b = 0; b < mat->num_blocks; ++b) {
        free(mat->blocks[b].JA);
        free(mat->blocks[b].AS);
    }
    free(mat->blocks);
    free(mat);
}

void print_hll_matrix(const HLLMatrix* mat) {
    printf("HLL Matrix (%d x %d), HackSize = %d\n", mat->M, mat->N, mat->HackSize);
    for (int b = 0; b < mat->num_blocks; ++b) {
        printf("Block %d: rows = %d, max_nz = %d\n", b, mat->blocks[b].rows_in_block, mat->blocks[b].max_nz_per_row);
    }
}