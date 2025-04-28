#include <stdio.h>
#include <stdbool.h>
#include "include/CSR_Matrix.h"

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
