#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "util.h"
#include "primitives.h"

#define TOL 1.0e-6 
#define TOL_DOUBLE 1.0e-9
#define MACRO_SWAP

#define SWAP(a,b) {float temp=(a);(a)=(b);(b)=temp;}


void gauss_jordan_partial(float **A, int N) {

    int i, j, k, max_row;
    double pivot, factor; 

    for (i = 0; i < N; i++) { // Loop through pivot columns 1 to N
        // partial pivoting 
        max_row = i;
        for (k = i + 1; k < N; k++) {
            if (fabs(A[k][i]) > fabs(A[max_row][i])) {
                max_row = k;
            }
        }


        #ifndef MACRO_SWAP
        // Swap rows
        if (max_row != i) {
            for (int j = 0; j <= N ; j++) {
                double temp = A[i][j];
                A[i][j] = A[max_row][j];
                A[max_row][j] = temp;
            }
        }
        #endif
        
        #ifdef MACRO_SWAP
        /* Alternatively */
        if (max_row != i) 
            for (j = 0; j<=N; j++) SWAP(A[i][j], A[max_row][j]);
        #endif

        // normalisation 
        pivot = A[i][i];
        if (fabs(pivot) < 1e-12) {
            nrerror("gauss_jordan: Matrix is singular or nearly singular.");
            fprintf(stderr,"gauss_jordan: Matrix is singular or nearly singular at pivot %d.\n", i);
            exit(1); 
        }

        for (j = i; j <= N ; j++) {
            A[i][j] /= pivot;
        }


        // elimination 
        for (k = 0; k < N; k++) {
            if (k == i) continue; // skip pivot row

            factor = A[k][i]; // factor for row k, column i

            for (j = i; j <= N ; j++) { 
                A[k][j] -= factor * A[i][j];
            }
        }
    }
}



// helper function: check for symmetry matrix
int is_symmetric(float **a, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) { // only check upper triangle against lower
            if (fabs(a[i][j] - a[j][i]) > TOL) {
                fprintf(stderr, "Symmetry Check Failed: A[%d][%d] (%.4f) != A[%d][%d] (%.4f)\n",
                        i, j, a[i][j], j, i, a[j][i]);
                return 0; // not symmetric
            }
        }
    }
    return 1; // symmetric
}

int is_symmetric_double(double **a, int n) {
    for (int i = 0; i < n; i++) {
       for (int j = i + 1; j < n; j++) {
           if (fabs(a[i][j] - a[j][i]) > TOL_DOUBLE) return 0; // 
       }
   }
   return 1;
}




void cholesky(float **A, int n)
/* Cholesky decomposition of a symmetric positive-definite matrix */
{
    int i, j, k;
    float sum;

    for (i = 0; i < n; i++) {
        for (j = 0; j <= i; j++) {
            sum = A[i][j];
            for (k = 0; k < j; k++){
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

    /* zero out the upper triangular part of the matrix for clarity */
    for (i = 0; i<n ; i++){
        for (j=i+1; j<n ;j++){
            A[i][j] = 0.0;
        }
    }
}


void cholesky_solve(float **A, float *b, float *x, int n)
/* solve the system Ax = b using Cholesky decomposition */
{
    int i, j;
    float sum;

    // forward substitution to solve Ly = b
    for (i = 0; i < n; i++) {
        sum = b[i];
        for (j = 0; j < i ; j++){
            sum -= A[i][j] * x[j];
        }
        x[i] = sum / A[i][i];
    }

    // back substitution to solve Ux = y
    for (i = n-1; i >= 0; i--) {
        sum = x[i];
        for (j = i + 1; j < n; j++){
            sum -= A[j][i] * x[j];
        }
        x[i] = sum / A[i][i];
    }
}


