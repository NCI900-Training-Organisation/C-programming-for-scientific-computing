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


void print_matrix(float **mat, long length_row, long length_col, const char *name) {
    printf("%s [%ld..%ld]:\n", name, length_row, length_col);
    for (long i = 0; i < length_row; i++) {
        printf("  [ ");
        for (long j = 0; j < length_col; j++) {
            float val = mat[i][j];
            if (fabs(val) < 1e-12) val = 0.0; // Prevent printing -0.0000
            printf("%10.4f ", val);
        }
        printf("]\n");
    }
    printf("\n");
}


void print_vector(float *vec, long length, const char *name) {
    printf("%s [%ld]:\n", name, length);
    printf("  [ ");
    for (long i = 0; i < length; i++) {
        float val = vec[i];
         if (fabs(val) < 1e-12) val = 0.0; // Prevent printing -0.0000
        printf("%10.4f ", val);
    }
    printf("]\n\n");
}


float *vector(long length) {
    float *v;
    v = (float *)malloc((size_t)(length * sizeof(float)));
    if (!v) nrerror("allocation failure in vector()");
    return v ;
}


int *ivector(long length)
{
    int *v;

    v = malloc((size_t) (length * sizeof(int)));
    if (!v) nrerror("allocation failure in ivector()");
    return v;
}

double *dvector(long length) {
    double *v;
    v = (double *)malloc((size_t)(length * sizeof(double)));
    if (!v) nrerror("allocation failure in dvector()");
    return v ;
}



float **matrix(long length_rows, long length_cols) {
    float **m;
    float *m_data;
    // allocate pointers to rows
    m = malloc((size_t)(length_rows * sizeof(float *)));
    if (!m) nrerror("allocation failure 1 in matrix()");

    m_data = malloc((size_t)(length_rows * length_cols * sizeof(float)));
    if (!m_data) nrerror("allocation failure 2 in matrix()");

   

    // allocate rows and set pointers to them
    for (long i = 0; i < length_rows; i++) {
        m[i] = m_data + (size_t)i * length_cols; //link the data to the matrix
    }

    return m;
}



double **dmatrix(long length_rows, long length_cols) {
    double **m;
    double *m_data;

    // allocate pointers to rows
    m = (double **)malloc((size_t)( length_rows * sizeof(double *)));
    if (!m) nrerror("allocation failure 1 in matrix()");
    m_data = (double *)malloc((size_t)(length_rows * length_cols * sizeof(double)));
    if (!m_data) nrerror("allocation failure 2 in matrix()");

    // allocate rows and set pointers to them
    for (long i = 0; i < length_rows; i++) {
        m[i] = m_data + (size_t)i * length_cols; //link the data to the matrix
    }

    return m;
}



// deallocation Functions 

void free_vector(float *v) {
    free(v);
}

void free_ivector(int *v) {
    free(v);
}


void free_dvector(double *v) {
    free(v);
}

void free_matrix(float **m) {
    if (m[0] != NULL) {
        free(m[0]); 
    }
    free(m); 
}


void free_dmatrix(double **m) {
    free(m[0]); // free the data array
    free(m); // free the array of pointers
}

