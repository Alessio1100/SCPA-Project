#include <stdio.h>
#include <stdlib.h>
#include "include/mmio.h"
#include "include/HLL_Matrix.h"

static inline void safe_malloc_check(void* ptr, const char* msg) {
    if (!ptr) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

#define DEFAULT_HACKSIZE 32

HLLMatrix* load_matrix_market_to_hll(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Error opening file");
        return NULL;
    }

    MM_typecode matcode;
    if (mm_read_banner(f, &matcode) != 0 ||
        !mm_is_matrix(matcode) ||
        !mm_is_coordinate(matcode) ||
        (!mm_is_real(matcode) && !mm_is_pattern(matcode))) {
        printf("Unsupported Matrix Market format\n");
        fclose(f);
        return NULL;
    }

    int M, N, NZ;
    mm_read_mtx_crd_size(f, &M, &N, &NZ);

    int is_pattern = mm_is_pattern(matcode);
    int is_symmetric = mm_is_symmetric(matcode);

    int capacity = is_symmetric ? 2 * NZ : NZ;

    int* rows = malloc(sizeof(int) * capacity);
    int* cols = malloc(sizeof(int) * capacity);
    double* vals = malloc(sizeof(double) * capacity);
    safe_malloc_check(rows, "malloc rows");
    safe_malloc_check(cols, "malloc cols");
    safe_malloc_check(vals, "malloc vals");

    int* row_counts = calloc(M, sizeof(int));
    safe_malloc_check(row_counts, "malloc row_counts");

    int count = 0;
    for (int i = 0; i < NZ; ++i) {
        int r, c;
        double v = 1.0;
        if (is_pattern) {
            fscanf(f, "%d %d", &r, &c);
        } else {
            fscanf(f, "%d %d %lf", &r, &c, &v);
        }
        r--; c--;
        rows[count] = r;
        cols[count] = c;
        vals[count] = v;
        row_counts[r]++;
        count++;

        if (is_symmetric && r != c) {
            rows[count] = c;
            cols[count] = r;
            vals[count] = v;
            row_counts[c]++;
            count++;
        }
    }
    fclose(f);

    int hacksize = DEFAULT_HACKSIZE;
    int num_blocks = (M + hacksize - 1) / hacksize;

    HLLMatrix* hll = malloc(sizeof(HLLMatrix));
    safe_malloc_check(hll, "malloc HLLMatrix");

    hll->M = M;
    hll->N = N;
    hll->HackSize = hacksize;
    hll->num_blocks = num_blocks;
    hll->blocks = malloc(num_blocks * sizeof(HLLBlock));
    safe_malloc_check(hll->blocks, "malloc HLL blocks");

    int** temp_cols = calloc(M, sizeof(int*));
    double** temp_vals = calloc(M, sizeof(double*));
    int* row_filled = calloc(M, sizeof(int));
    safe_malloc_check(temp_cols, "malloc temp_cols");
    safe_malloc_check(temp_vals, "malloc temp_vals");
    safe_malloc_check(row_filled, "malloc row_filled");

    for (int i = 0; i < count; ++i) {
        int r = rows[i];
        if (!temp_cols[r]) {
            temp_cols[r] = malloc(row_counts[r] * sizeof(int));
            temp_vals[r] = malloc(row_counts[r] * sizeof(double));
            safe_malloc_check(temp_cols[r], "malloc temp_cols[r]");
            safe_malloc_check(temp_vals[r], "malloc temp_vals[r]");
        }
        temp_cols[r][row_filled[r]] = cols[i];
        temp_vals[r][row_filled[r]] = vals[i];
        row_filled[r]++;
    }

    free(rows);
    free(cols);
    free(vals);
    free(row_counts);

    for (int b = 0; b < num_blocks; ++b) {
        int start = b * hacksize;
        int end = (b + 1) * hacksize;
        if (end > M) end = M;
        int rows_in_block = end - start;

        int max_nz = 0;
        for (int i = start; i < end; ++i) {
            if (row_filled[i] > max_nz) max_nz = row_filled[i];
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
            for (int j = 0; j < row_filled[i]; ++j) {
                int idx = j * rows_in_block + local_row;
                JA[idx] = temp_cols[i][j];
                AS[idx] = temp_vals[i][j];
            }
        }

        hll->blocks[b].rows_in_block = rows_in_block;
        hll->blocks[b].max_nz_per_row = max_nz;
        hll->blocks[b].JA = JA;
        hll->blocks[b].AS = AS;
    }

    for (int i = 0; i < M; ++i) {
        free(temp_cols[i]);
        free(temp_vals[i]);
    }
    free(temp_cols);
    free(temp_vals);
    free(row_filled);

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