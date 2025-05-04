#ifndef NA_UTIL_H
#define NA_UTIL_H

float *vector(long nl, long nh);

int *ivector(long nl, long nh);

float **matrix(long nrl, long nrh, long ncl, long nch);

double *dvector(long nl, long nh);

double **dmatrix(long nrl, long nrh, long ncl, long nch);

void free_dvector(double *v, long nl, long nh);

void free_dmatrix(double **m, long nrl, long ncl);

void nrerror(char error_text[]);

void print_nr_matrix(float **mat, long nrl, long nrh, long ncl, long nch, const char *name);

void print_nr_vector(float *vec, long nl, long nh, const char *name);

void free_vector(float *v, long nl, long nh) ;

void free_matrix(float **m, long nrl,  long ncl);

void free_ivector(int *v, long nl, long nh);



#endif