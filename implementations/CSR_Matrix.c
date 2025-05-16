#include <stdio.h>
#include <stdlib.h>
#include "include/CSR_Matrix.h"
#include "include/mmio.h"

static inline void safe_malloc_check(void* ptr, const char* msg) {
    if (!ptr) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

CSRMatrix* load_matrix_market_to_csr(const char* filename) {
    
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

    int* row_counts = calloc(M, sizeof(int));
    int* rows = malloc(sizeof(int) * capacity);
    int* cols = malloc(sizeof(int) * capacity);
    double* vals = malloc(sizeof(double) * capacity);
    safe_malloc_check(row_counts, "malloc row_counts");
    safe_malloc_check(rows, "malloc rows");
    safe_malloc_check(cols, "malloc cols");
    safe_malloc_check(vals, "malloc vals");

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

    CSRMatrix* mat = malloc(sizeof(CSRMatrix));
    safe_malloc_check(mat, "malloc CSRMatrix");

    mat->M = M;
    mat->N = N;
    mat->NZ = count;
    mat->IRP = malloc((M + 1) * sizeof(int));
    mat->JA = malloc(count * sizeof(int));
    mat->AS = malloc(count * sizeof(double));
    safe_malloc_check(mat->IRP, "malloc IRP");
    safe_malloc_check(mat->JA, "malloc JA");
    safe_malloc_check(mat->AS, "malloc AS");

    mat->IRP[0] = 0;
    for (int i = 0; i < M; ++i) {
        mat->IRP[i + 1] = mat->IRP[i] + row_counts[i];
    }

    int* current_position = calloc(M, sizeof(int));
    safe_malloc_check(current_position, "malloc current_position");

    for (int i = 0; i < count; ++i) {
        int r = rows[i];
        int idx = mat->IRP[r] + current_position[r];
        mat->JA[idx] = cols[i];
        mat->AS[idx] = vals[i];
        current_position[r]++;
    }

    free(rows);
    free(cols);
    free(vals);
    free(row_counts);
    free(current_position);

    return mat;
}

void free_csr_matrix(CSRMatrix* mat) {
    if (!mat) return;
    free(mat->IRP);
    free(mat->JA);
    free(mat->AS);
    free(mat);
}