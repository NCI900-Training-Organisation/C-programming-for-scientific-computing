#ifndef NA_UTIL_H
#define NA_UTIL_H

void nrerror(char error_text[]);


void print_matrix(float **mat, long length_row, long length_col, const char *name);

void print_vector(float *vec, long length, const char *name);

float *vector(long length);

int *ivector(long length);

double *dvector(long length);

float **matrix(long length_rows, long length_cols);

double **dmatrix(long length_rows, long length_cols);

void free_vector(float *v) ;

void free_ivector(int *v);

void free_dvector(double *v);

void free_matrix(float **m);

void free_dmatrix(double **m);



#endif