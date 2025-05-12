#ifndef PRIMITIVES_H
#define PRIMITIVES_H


void cholesky(float **A, int n);

void cholesky_solve(float **A, float *b, float *x, int n);

void gauss_jordan_partial(float **A, int N);

int is_symmetric(float **a, int n);

int is_symmetric_double(double **a, int n);

#endif