#ifdef CALCULUS_H
#define CALCULUS_H

void csr_serial_mat_per_vec(CSRMatrix *csr_matrix, double *x, double *y);
void hll_serial_mat_per_vec(HLLMatrix *hll_matrix, const double *x, double *y);
void print_vector(double *y, int size);

