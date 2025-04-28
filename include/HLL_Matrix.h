#ifndef HLL_MATRIX_H
#define HLL_MATRIX_H

typedef struct {
    int rows_in_block;       // Numero di righe nel blocco
    int max_nz_per_row;      // Max numero di non-zero per riga (padding ELLPACK)
    int* JA;                 // Indici colonna (dimensione: rows_in_block * max_nz_per_row)
    double* AS;              // Valori (stessa dimensione)
} HLLBlock;

typedef struct {
    int M;                   // Numero di righe della matrice originale
    int N;                   // Numero di colonne
    int HackSize;            // Numero di righe per blocco
    int num_blocks;          // Numero di blocchi totali
    HLLBlock* blocks;        // Array di blocchi HLL
} HLLMatrix;

// Funzione per liberare la memoria di una matrice HLL
void free_hll_matrix(HLLMatrix* hll);

// Funzione per caricare una matrice .mtx in formato HLL column-major
HLLMatrix* load_matrix_market_to_hll_column_major(const char* filename);

#endif // HLL_MATRIX_H
