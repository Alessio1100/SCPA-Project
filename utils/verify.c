#include <stdio.h>
#include <stdbool.h>
#include "include/CSR_Matrix.h"
#include "include/HLL_Matrix.h"
/**
 * Verifica la validità strutturale della matrice CSR.
 * 
 * Controlla:
 * - Che IRP sia crescente
 * - Che IRP[M] == NZ
 * - Che tutti gli indici JA siano validi (0 <= JA[i] < N)
 * 
 * Se `verbose == true`, stampa anche la struttura CSR.
 */
bool verify_csr_matrix(const CSRMatrix* mat, bool verbose) {
    if (!mat || !mat->IRP || !mat->JA || !mat->AS) {
        printf("❌ CSRMatrix non inizializzata correttamente\n");
        return false;
    }

    // Verifica 1: IRP deve essere strettamente crescente o uguale (ma mai decrescente)
    for (int i = 0; i < mat->M; ++i) {
        if (mat->IRP[i] > mat->IRP[i + 1]) {
            printf("❌ Errore: IRP non crescente tra IRP[%d]=%d e IRP[%d]=%d\n",
                   i, mat->IRP[i], i + 1, mat->IRP[i + 1]);
            return false;
        }
    }

    // Verifica 2: IRP[M] deve essere uguale al numero totale di elementi non zero
    if (mat->IRP[mat->M] != mat->NZ) {
        printf("❌ Errore: IRP[%d]=%d ma NZ=%d\n", mat->M, mat->IRP[mat->M], mat->NZ);
        return false;
    }

    // Verifica 3: Tutti gli indici JA devono essere nel range [0, N-1]
    for (int i = 0; i < mat->NZ; ++i) {
        if (mat->JA[i] < 0 || mat->JA[i] >= mat->N) {
            printf("❌ Errore: JA[%d]=%d fuori dal range valido [0, %d]\n", i, mat->JA[i], mat->N - 1);
            return false;
        }
    }
    return true;
}
bool verify_hll_matrix(const HLLMatrix* mat, bool verbose) {
    if (!mat || !mat->blocks) {
        printf("\u274c HLLMatrix non inizializzata correttamente\n");
        return false;
    }

    for (int b = 0; b < mat->num_blocks; ++b) {
        HLLBlock block = mat->blocks[b];

        if (!block.JA || !block.AS) {
            printf("\u274c Errore: Blocchi non inizializzati correttamente (block %d)\n", b);
            return false;
        }

        int rows_in_block = block.rows_in_block;
        int max_nz_per_row = block.max_nz_per_row;

        if (rows_in_block <= 0 || max_nz_per_row <= 0) {
            printf("\u274c Errore: Parametri invalidi nel blocco %d (rows_in_block = %d, max_nz_per_row = %d)\n",
                   b, rows_in_block, max_nz_per_row);
            return false;
        }

        int size = rows_in_block * max_nz_per_row;

        for (int i = 0; i < size; ++i) {
            if (block.JA[i] < 0 || block.JA[i] >= mat->N) {
                if (block.AS[i] != 0.0) { // Solo se c'è un valore significativo
                    printf("\u274c Errore: JA[%d] nel blocco %d fuori range [0, %d] (JA=%d)\n",
                           i, b, mat->N - 1, block.JA[i]);
                    return false;
                }
            }
        }
    }

    if (verbose) {
        printf("\u2705 HLL Matrix (%d x %d), HackSize = %d\n", mat->M, mat->N, mat->HackSize);
        for (int b = 0; b < mat->num_blocks; ++b) {
            HLLBlock block = mat->blocks[b];
            printf(" Block %d: rows = %d, max_nz_per_row = %d\n",
                   b, block.rows_in_block, block.max_nz_per_row);
        }
    }

    return true;
}
