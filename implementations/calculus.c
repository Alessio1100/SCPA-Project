#include <stdio.h>
#include <stdlib.h>
#include "include/CSR_Matrix.h"
#include "include/HLL_Matrix.h"

void csr_serial_mat_per_vec(CSRMatrix *csr_matrix, double *x, double *y){
     
    for (int i = 0; i < csr_matrix->M; i++){

        for (int j = csr_matrix->IRP[i]; j < csr_matrix->IRP[i+1]; i++){
            
            y[i] += csr_matrix->AS[j] * x[csr_matrix->JA[j]]
        }        
    }   
}
static inline int rows(HLLMatrix *hll_matrix, int h){
    if (hll_matrix->hacks_num -1 == h && hll_matrix->M % hll_matrix->hack_size){

        return hll_matrix->M % hll_matrix->hack_size;
    }

    return hll_matrix->hack_size;
}
void hll_serial_mat_per_vec(HLLMatrix *hll_matrix, double *x, double *y){
    
    rows = rows(hll_matrix,0);

    for (int h = 0; h < hll_matrix->hack_size; h++){
        
        for (int r = 0; r < rows(hll_matrix,h); i++){

            double sum = 0.0;
            for (int j = 0; j < hll_matrix->max_nz_per_row[h]; j++)
            {
                /* code */
            }
            
        }
        

    }
    
}