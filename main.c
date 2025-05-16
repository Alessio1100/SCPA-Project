#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include "include/mmio.h"
#include "include/CSR_Matrix.h"
#include "include/HLL_Matrix.h"
#include "include/verify.h"
#include "include/initialize.h"
#include "include/utils.h"
#include "include/calculus.h"

#define COMPUTATION_NUMBER 5
#define MATRIX_DIR "../matrix/"

extern void csr_serial_mat_per_vec(CSRMatrix *csr_matrix, double *x, double *y);
extern void hll_serial_mat_per_vec(HLLMatrix *hll_matrix, const double *x, double *y);
extern void re_initialize_y_vector(int size, double *y);
extern int compute_norm(const double *v1, const double *v2, int size, double tol);

int main() {
    DIR *dir;
    struct dirent *entry;
    char path[512];

    dir = opendir(MATRIX_DIR);
    if (!dir) {
        perror("Errore apertura directory matrici");
        return EXIT_FAILURE;
    }

    while ((entry = readdir(dir)) != NULL) {

        if (entry->d_name[0] == '.') continue; // salta . e ..

        snprintf(path, sizeof(path), "%s%s", MATRIX_DIR, entry->d_name);
        printf("\nProcessing matrix: %s\n", entry->d_name);

        // Caricamento matrice CSR
        CSRMatrix* csr = load_matrix_market_to_csr(path);
        if (!csr) {
            printf("Errore nella lettura CSR per %s\n", entry->d_name);
            continue;
        }

        // Conversione a HLL
        int hacksize = 32;
        HLLMatrix* hll = convert_csr_to_hll(csr, hacksize);

        // Inizializzazione vettori
        double *x = initialize_x_vector(csr->N);
        double *y = initialize_y_vector(csr->M);
        double *z = initialize_y_vector(csr->M);

        double start, end, time_csr = 0.0, time_hll = 0.0;

        // Esecuzione CSR seriale
        for (int i = 0; i < COMPUTATION_NUMBER; i++) {
            memset(y, 0, csr->M * sizeof(double));
            start = omp_get_wtime();
            csr_serial_mat_per_vec(csr, x, y);
            end = omp_get_wtime();
            time_csr += end - start;
        }

        // Esecuzione HLL seriale + verifica
        for (int i = 0; i < COMPUTATION_NUMBER; i++) {
            memset(z, 0, csr->M * sizeof(double));
            start = omp_get_wtime();
            hll_serial_mat_per_vec(hll, x, z);
            end = omp_get_wtime();
            time_hll += end - start;

            if (!compute_norm(y, z, csr->M, 1e-4)) {
                printf("\u274c Differenza nei risultati CSR vs HLL per %s\n", entry->d_name);
            }
        }

        printf("\u2705 Tempo medio CSR seriale per %s: %.6lf s\n", entry->d_name, time_csr / COMPUTATION_NUMBER);
        printf("\u2705 Tempo medio HLL seriale per %s: %.6lf s\n", entry->d_name, time_hll / COMPUTATION_NUMBER);

        // Cleanup
        free(x); free(y); free(z);
        free_csr_matrix(csr);
        free_hll_matrix(hll);
    }

    closedir(dir);
    return EXIT_SUCCESS;
}

