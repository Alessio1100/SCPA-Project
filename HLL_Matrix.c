#include <stdio.h>
#include <stdlib.h>
#include "include/mmio.h"
#include "include/HLL_Matrix.h"

#define HS 32  // Può essere parametrico

HLLMatrix* load_matrix_market_to_hll(const char* filename) {
    FILE* f;
    MM_typecode matcode;
    int M, N, NZ;

    f = fopen(filename, "r");
    if (!f) {
        perror("Error opening file");
        return NULL;
    }

    if (mm_read_banner(f, &matcode) != 0 ||
        !mm_is_matrix(matcode) ||
        !mm_is_coordinate(matcode) ||
        (!mm_is_real(matcode) && !mm_is_pattern(matcode))) {
        printf("Unsupported Matrix Market format\n");
        fclose(f);
        return NULL;
    }

    int is_pattern = mm_is_pattern(matcode);
    int is_symmetric = mm_is_symmetric(matcode);

    mm_read_mtx_crd_size(f, &M, &N, &NZ);
    int capacity = is_symmetric ? 2 * NZ : NZ;

    // Allocate and load entries
    int* row_counts = calloc(M, sizeof(int));
    int* row = malloc(sizeof(int) * capacity);
    int* col = malloc(sizeof(int) * capacity);
    double* val = malloc(sizeof(double) * capacity);

    int count = 0;
    for (int i = 0; i < NZ; i++) {
        int r, c;
        double v = 1.0;
        if (is_pattern) {
            fscanf(f, "%d %d\n", &r, &c);
        } else {
            fscanf(f, "%d %d %lf\n", &r, &c, &v);
        }
        r--; c--;

        row[count] = r;
        col[count] = c;
        val[count] = v;
        row_counts[r]++;
        count++;

        if (is_symmetric && r != c) {
            row[count] = c;
            col[count] = r;
            val[count] = v;
            row_counts[c]++;
            count++;
        }
    }
    fclose(f);

    // Init HLL matrix
    int num_blocks = (M + HS - 1) / HS;
    HLLMatrix* hll = malloc(sizeof(HLLMatrix));
    hll->M = M;
    hll->N = N;
    hll->HackSize = HS;
    hll->num_blocks = num_blocks;
    hll->blocks = malloc(num_blocks * sizeof(HLLBlock));

    // Temp row tracker
    int** row_entries = calloc(M, sizeof(int*));
    double** row_vals = calloc(M, sizeof(double*));
    int* row_capacity = calloc(M, sizeof(int));
    int* row_len = calloc(M, sizeof(int));

    // Bucket entries per row
    for (int i = 0; i < count; i++) {
        int r = row[i];
        if (row_len[r] == 0) {
            row_capacity[r] = 4;
            row_entries[r] = malloc(row_capacity[r] * sizeof(int));
            row_vals[r] = malloc(row_capacity[r] * sizeof(double));
        }
        if (row_len[r] == row_capacity[r]) {
            row_capacity[r] *= 2;
            row_entries[r] = realloc(row_entries[r], row_capacity[r] * sizeof(int));
            row_vals[r] = realloc(row_vals[r], row_capacity[r] * sizeof(double));
        }
        row_entries[r][row_len[r]] = col[i];
        row_vals[r][row_len[r]] = val[i];
        row_len[r]++;
    }

    // Build blocks
    for (int b = 0; b < num_blocks; b++) {
        int start = b * HS;
        int end = (b + 1) * HS;
        if (end > M) end = M;
        int rows = end - start;

        int max_nz = 0;
        for (int i = start; i < end; i++) {
            if (row_len[i] > max_nz) max_nz = row_len[i];
        }

        int size = rows * max_nz;
        int* JA = malloc(size * sizeof(int));
        double* AS = malloc(size * sizeof(double));
        for (int i = 0; i < size; i++) {
            JA[i] = -1;
            AS[i] = 0.0;
        }

        for (int i = start; i < end; i++) {
            int local_row = i - start;
            for (int j = 0; j < row_len[i]; j++) {
                int idx = j * rows + local_row; // column-major
                JA[idx] = row_entries[i][j];
                AS[idx] = row_vals[i][j];
            }
        }

        hll->blocks[b].rows_in_block = rows;
        hll->blocks[b].max_nz_per_row = max_nz;
        hll->blocks[b].JA = JA;
        hll->blocks[b].AS = AS;
    }

    // Cleanup temporanei
    for (int i = 0; i < M; i++) {
        free(row_entries[i]);
        free(row_vals[i]);
    }
    free(row_entries);
    free(row_vals);
    free(row_len);
    free(row_capacity);
    free(row);
    free(col);
    free(val);
    free(row_counts);

    return hll;
}
