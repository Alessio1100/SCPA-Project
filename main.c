#include <stdio.h>
#include <stdlib.h>
#include "include/mmio.h"
#include "include/CSR_Matrix.h"
#include "include/HLL_Matrix.h"
#include "include/verify.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <file_matrix.mtx>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* filename = argv[1];

    // Carica la matrice da file
    CSRMatrix* A = load_matrix_market_to_csr(filename);
    if (!A) {
        fprintf(stderr, "Errore nel caricamento della matrice.\n");
        return EXIT_FAILURE;
    }

    // Esegui la verifica della struttura CSR
    if (verify_csr_matrix(A, true)) {
        printf("✅ Verifica CSR completata: la matrice è valida.\n");
    } else {
        printf("❌ Verifica fallita: la matrice NON è valida.\n");
    }


    HLLMatrix* B = convert_csr_to_hll(A);
    if (!B) {
        fprintf(stderr, "Errore nel caricamento della matrice.\n");
        return EXIT_FAILURE;
    }
    if (verify_hll_matrix(B, true)) {
        printf("✅ Verifica HLL completata: la matrice è valida.\n");
    } else {
        printf("❌ Verifica fallita: la matrice NON è valida.\n");
    }
    // Libera la memoria
    free_csr_matrix(A);
    free_hll_matrix(B);
    return EXIT_SUCCESS;
}
