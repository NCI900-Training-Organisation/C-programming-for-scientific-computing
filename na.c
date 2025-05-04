#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "na_util.h"
#include "na.h"

#define TOL 1.0e-6 
#define TOL_DOUBLE 1.0e-9
#define MACRO_SWAP

#define SWAP(a,b) {float temp=(a);(a)=(b);(b)=temp;}


void gauss_jordan_partial(float **A, int N) {

    int i, j, k, max_row;
    double pivot, factor; // Use double for intermediate precision

    for (i = 1; i <= N; i++) { // Loop through pivot columns 1 to N
        // --- Partial Pivoting ---
        max_row = i;
        for (k = i + 1; k <= N; k++) {
            if (fabs(A[k][i]) > fabs(A[max_row][i])) {
                max_row = k;
            }
        }


        #ifndef MACRO_SWAP
        // Swap rows
        if (max_row != i) {
            for (int j = 1; j <= N + 1; j++) {
                double temp = A[i][j];
                A[i][j] = A[max_row][j];
                A[max_row][j] = temp;
            }
        }
        #endif
        
        #ifdef MACRO_SWAP
        /* Alternatively */
        if (max_row != i) 
            for (j = 1; j<=N+1; j++) SWAP(A[i][j], A[max_row][j]);
        #endif

        // --- Normalization ---
        pivot = A[i][i];
        if (fabs(pivot) < 1e-12) {
            // nrerror("gauss_jordan: Matrix is singular or nearly singular.");
            fprintf(stderr,"gauss_jordan: Matrix is singular or nearly singular at pivot %d.\n", i);
            exit(1); // Or return an error code
        }

        // Divide pivot row by pivot value (Optimized: start from column i)
        for (j = i; j <= N + 1; j++) {
            A[i][j] /= pivot;
        }
        // A[i][i] is now 1.0

        // --- Elimination ---
        for (k = 1; k <= N; k++) {
            if (k == i) continue; // Skip pivot row

            factor = A[k][i]; // Factor for row k, column i

            // Eliminate elements in column i for row k
            // Subtract factor * (pivot row) from row k
            // (Optimized: start from column i)
            for (j = i; j <= N + 1; j++) { // CORRECTED loop: 1-based, starts at i, goes to N+1
                A[k][j] -= factor * A[i][j];
            }
            // A[k][i] is now 0.0
        }
    }
}



// --- Helper: Check for Matrix Symmetry ---
int is_symmetric(float **a, int n) {
    for (int i = 1; i <= n; i++) {
        for (int j = i + 1; j <= n; j++) { // Only check upper triangle against lower
            if (fabs(a[i][j] - a[j][i]) > TOL) {
                fprintf(stderr, "Symmetry Check Failed: A[%d][%d] (%.4f) != A[%d][%d] (%.4f)\n",
                        i, j, a[i][j], j, i, a[j][i]);
                return 0; // Not symmetric
            }
        }
    }
    return 1; // Symmetric
}

int is_symmetric_double(double **a, int n) {
    for (int i = 1; i <= n; i++) {
       for (int j = i + 1; j <= n; j++) {
           if (fabs(a[i][j] - a[j][i]) > TOL_DOUBLE) return 0; // Use fabs and TOL_DOUBLE
       }
   }
   return 1;
}




void cholesky(float **A, int n)
/* Cholesky decomposition of a symmetric positive-definite matrix */
{
    int i, j, k;
    float sum;

    for (i = 1; i <= n; i++) {
        for (j = 1; j <= i; j++) {
            sum = A[i][j];
            for (k = 1; k <= j - 1; k++){
                sum -= A[i][k] * A[j][k];
        }
        if (i==j) {
            if (sum <= 0.0){
                nrerror("Matrix is not positive-definite");
            }
            A[i][i] = sqrt(sum);
        }
        else {
            A[i][j] = sum / A[j][j];
            }
        }
    }

    /* Zero out the upper triangular part of the matrix for clarity */
    for (i = 1; i<=n ; i++){
        for (j=i+1; j<=n ;j++){
            A[i][j] = 0.0;
        }
    }
}


void cholesky_solve(float **A, float *b, float *x, int n)
/* Solve the system Ax = b using Cholesky decomposition */
{
    int i, j;
    float sum;

    // Forward substitution to solve Ly = b
    for (i = 1; i <= n; i++) {
        sum = b[i];
        for (j = 1; j <= i - 1; j++){
            sum -= A[i][j] * x[j];
        }
        x[i] = sum / A[i][i];
    }

    // Back substitution to solve Ux = y
    for (i = n; i >= 1; i--) {
        sum = x[i];
        for (j = i + 1; j <= n; j++){
            sum -= A[j][i] * x[j];
        }
        x[i] = sum / A[i][i];
    }
}


