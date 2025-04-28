#include <stdio.h>
#include <stdlib.h>
#include "include/mmio.h"
#include "include/CSR_Matrix.h"
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
        printf("✅ Verifica completata: la matrice è valida.\n");
    } else {
        printf("❌ Verifica fallita: la matrice NON è valida.\n");
    }

    // Libera la memoria
    free_csr_matrix(A);
    return EXIT_SUCCESS;
}
