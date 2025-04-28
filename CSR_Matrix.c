#include <stdio.h>
#include <stdlib.h>
#include "include/CSR_Matrix.h"
#include "include/mmio.h"

int compare_row_col(const void* a, const void* b){
   int ra = ((int*)a)[0], ca = ((int*)a)[1];
   int rb = ((int*)b)[0], cb = ((int*)b)[1];
   return (ra != rb) ? (ra - rb) : (ca - cb);
}

CSRMatrix* load_matrix_market_to_csr(const char* filename) {
   FILE* f;
   MM_typecode matcode;
   int M, N, NZ;

   f = fopen(filename, "r");
   if (!f) {
       perror("Error opening file");
       return NULL;
   }

   if (mm_read_banner(f, &matcode) != 0) {
       printf("Error reading Matrix Market Banner");
       fclose(f);
       return NULL;
   }

   if (!mm_is_matrix(matcode) || !mm_is_coordinate(matcode) || 
       (!mm_is_real(matcode) && !mm_is_pattern(matcode))) {
       printf("Format not supported (only real or pattern matrices in coordinate format)");
       fclose(f);
       return NULL;
   }

   int is_pattern = mm_is_pattern(matcode);
   int is_symmetric = mm_is_symmetric(matcode);

   mm_read_mtx_crd_size(f, &M, &N, &NZ);

   int capacity = is_symmetric ? 2 * NZ : NZ;

   int* row_counts = calloc(M, sizeof(int));
   int* entry_row = malloc(sizeof(int) * capacity);
   int* entry_col = malloc(sizeof(int) * capacity);
   double* vals = malloc(sizeof(double) * capacity);

   int count = 0;
   for (int i = 0; i < NZ; i++) {
       int row, col;
       double val = 1.0;
       if (is_pattern) {
           fscanf(f, "%d %d\n", &row, &col);
       } else {
           fscanf(f, "%d %d %lf\n", &row, &col, &val);
       }
       row--; col--;

       entry_row[count] = row;
       entry_col[count] = col;
       vals[count] = val;
       row_counts[row]++;
       count++;

       if (is_symmetric && row != col) {
           entry_row[count] = col;
           entry_col[count] = row;
           vals[count] = val;
           row_counts[col]++;
           count++;
       }
   }
   fclose(f);

   int* IRP = malloc((M + 1) * sizeof(int));
   IRP[0] = 0;
   for (int i = 0; i < M; i++)
       IRP[i + 1] = IRP[i] + row_counts[i];

   int* JA = malloc(sizeof(int) * count);
   double* AS = malloc(sizeof(double) * count);

   int* counter = calloc(M, sizeof(int));
   for (int i = 0; i < count; i++) {
       int r = entry_row[i];
       int idx = IRP[r] + counter[r]++;
       JA[idx] = entry_col[i];
       AS[idx] = vals[i];
   }

   free(entry_row);
   free(entry_col);
   free(vals);
   free(row_counts);
   free(counter);

   CSRMatrix* mat = malloc(sizeof(CSRMatrix));
   mat->M = M;
   mat->N = N;
   mat->NZ = count;
   mat->IRP = IRP;
   mat->JA = JA;
   mat->AS = AS;

   return mat;
}


void free_csr_matrix(CSRMatrix* mat){
   if (!mat) return;

   free(mat->IRP);
   free(mat->JA);
   free(mat->AS);
   free(mat);
   
}

void print_csr_matrix(const CSRMatrix* mat) {
    printf("CSR Matrix (%d x %d), NZ = %d\n", mat->M, mat->N, mat->NZ);

    printf("IRP: ");
    for (int i = 0; i <= mat->M; ++i)
        printf("%d ", mat->IRP[i]);
    printf("\n");

    printf("JA:  ");
    for (int i = 0; i < mat->NZ; ++i)
        printf("%d ", mat->JA[i]);
    printf("\n");

    printf("AS:  ");
    for (int i = 0; i < mat->NZ; ++i)
        printf("%.2f ", mat->AS[i]);
    printf("\n");
}
