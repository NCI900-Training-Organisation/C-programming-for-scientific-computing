#include<stdio.h>
#include<stdlib.h>
#include<stddef.h>
#include<math.h>

#define NR_END 1
#define FREE_ARG char*

void nrerror(char error_text[])
{ 
    fprintf(stderr, "runtime error...\n");
    fprintf(stderr, "%s\n", error_text);
    fprintf(stderr, "... now exiting to system...\n");
    exit(1);
}


void print_nr_matrix(float **mat, long nrl, long nrh, long ncl, long nch, const char *name) {
    printf("%s [%ld..%ld][%ld..%ld]:\n", name, nrl, nrh, ncl, nch);
    for (long i = nrl; i <= nrh; i++) {
        printf("  [ ");
        for (long j = ncl; j <= nch; j++) {
            float val = mat[i][j];
            if (fabs(val) < 1e-12) val = 0.0; // Prevent printing -0.0000
            printf("%10.4f ", val);
        }
        printf("]\n");
    }
    printf("\n");
}


void print_nr_vector(float *vec, long nl, long nh, const char *name) {
    printf("%s [%ld..%ld]:\n", name, nl, nh);
    printf("  [ ");
    for (long i = nl; i <= nh; i++) {
        float val = vec[i];
         if (fabs(val) < 1e-12) val = 0.0; // Prevent printing -0.0000
        printf("%10.4f ", val);
    }
    printf("]\n\n");
}


float *fvector(long nl, long nh)
{
    float *v;
    v = (float *) malloc((size_t) ((nh-nl+1+NR_END) * sizeof(float)));
    if (!v) nrerror("allocation failure in vector()");
    return v-nl+NR_END;
}

int *ivector(long nl, long nh)
{
    int *v;

    v=(int*) malloc((size_t) ((nh-nl+1+NR_END) * sizeof(int)));
    if (!v) nrerror("allocation failure in ivector()");
    return v-nl+NR_END;
}


float *vector(long nl, long nh) {
    float *v;
    v = (float *)malloc((size_t)((nh - nl + 1 + NR_END) * sizeof(float)));
    if (!v) nrerror("allocation failure in vector()");
    return v - nl + NR_END;
}

float **matrix(long nrl, long nrh, long ncl, long nch) {
    long i, nrow = nrh - nrl + 1, ncol = nch - ncl + 1;
    float **m;

    // Allocate pointers to rows
    m = (float **)malloc((size_t)((nrow + NR_END) * sizeof(float *)));
    if (!m) nrerror("allocation failure 1 in matrix()");
    m += NR_END;
    m -= nrl;

    // Allocate rows and set pointers to them
    m[nrl] = (float *)malloc((size_t)((nrow * ncol + NR_END) * sizeof(float)));
    if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
    m[nrl] += NR_END;
    m[nrl] -= ncl;

    for (i = nrl + 1; i <= nrh; i++) m[i] = m[i - 1] + ncol;

    return m;
}


// --- Double Utils ---
double *dvector(long nl, long nh) {
    double *v;
    v = (double *)malloc((size_t)((nh - nl + 1 + NR_END) * sizeof(double)));
    if (!v) nrerror("allocation failure in dvector()");
    return v - nl + NR_END;
}

double **dmatrix(long nrl, long nrh, long ncl, long nch) {
    long i, nrow = nrh - nrl + 1, ncol = nch - ncl + 1;
    double **m;

    // Allocate pointers to rows
    m = (double **)malloc((size_t)((nrow + NR_END) * sizeof(double *)));
    if (!m) nrerror("allocation failure 1 in matrix()");
    m += NR_END;
    m -= nrl;

    // Allocate rows and set pointers to them
    m[nrl] = (double *)malloc((size_t)((nrow * ncol + NR_END) * sizeof(double)));
    if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
    m[nrl] += NR_END;
    m[nrl] -= ncl;

    for (i = nrl + 1; i <= nrh; i++) m[i] = m[i - 1] + ncol;

    return m;
}



// --- Deallocation Functions  ---

void free_vector(float *v, long nl) {
    free((void *)(v + nl - NR_END));
}

void free_matrix(float **m, long nrl,  long ncl) {
    free((void *)(m[nrl] + ncl - NR_END));
    free((void *)(m + nrl - NR_END));
}


void free_dvector(double *v, long nl) {
    free((void *)(v + nl - NR_END));
}

void free_dmatrix(double **m, long nrl, long ncl) {
    free((void *)(m[nrl] + ncl - NR_END));
    free((void *)(m + nrl - NR_END));
}


void free_ivector(int *v, long nl) {
     free((void*)(v + nl - NR_END));
}