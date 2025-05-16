#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "include/mmio.h"
#include "include/CSR_Matrix.h"
#include "include/HLL_Matrix.h"

double *initialize_x_vector(int size)
{

    // Inizialize the psudo-random number generator with time-based seed
    srand(time(NULL));

    // Create x vector
    double *x = (double *)calloc(size, sizeof(double));
    if (x == NULL)
    {
        printf("Error occur in malloc for the x vector!\nError code: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Inizialize x vector to presudo-random double values
    for (int i = 0; i < size; i++)
        x[i] = (rand() % 5) + 1;

    return x;
}

double *initialize_y_vector(int size)
{
    double *y = (double *)calloc(size, sizeof(double));
    if (y == NULL)
    {
        printf("Error occour in malloc for the y vector!\n Error code: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    return y;
}