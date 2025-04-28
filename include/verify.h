#ifndef VERIFY_H
#define VERIFY_H

#include <stdbool.h>
#include "CSR_Matrix.h"

bool verify_csr_matrix(const CSRMatrix* mat, bool verbose);

#endif
