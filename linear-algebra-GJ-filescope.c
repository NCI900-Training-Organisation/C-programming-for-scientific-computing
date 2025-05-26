#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 


#define MAX_SIZE 20       // Max N dimension
#define MAXSTR 80
#define TOL 1.0e-6 // tolerance 
#define MACRO_SWAP
#define SWAP(a,b) {float temp=(a);(a)=(b);(b)=temp;}

/* forward decalrations */
void gauss_jordan_partial(float **A, int N);
void nrerror(char error_text[]);
void print_matrix(float **mat, long length_row, long length_col, const char *name);
void print_vector(float *vec, long length, const char *name);
float *vector(long length);
int *ivector(long length);
float **matrix(long length_rows, long length_cols);
void free_vector(float *v);
void free_ivector(int *v);
void free_matrix(float **m);



int main(int argc, char *argv[])
{
    // declarations
    int j, k, l;
    int n_row,m_col;
    float **A, **Aug, **A_chol;
    float *b, *x, *check;
    char buffer[MAXSTR];
    FILE *fp;
    char *input_filename = NULL;


    // check command line arguments 
    if (argc != 2) {
        /* TODO: Handle error*/
    }

    input_filename = argv[1]; // get filename from the command line


    // --- allocate memory for matrices an vectors  ---
    A = matrix(MAX_SIZE, MAX_SIZE);
    b = vector(MAX_SIZE);
    x = vector(MAX_SIZE);
    Aug = matrix(MAX_SIZE, MAX_SIZE + 1);
    A_chol = matrix(MAX_SIZE, MAX_SIZE);
    check = vector(MAX_SIZE);

    //  input file 
    printf("Input file: %s\n", input_filename);
    fp = fopen(input_filename, "r");  /* Todo: Handle error */ 
    printf("Successfully opened file.\n");

    // --- consume initial header ---
    fgets(buffer, MAXSTR, fp);  /* Todo: Handle error */ 
    fgets(buffer, MAXSTR, fp); /* Todo: Handle error */


    // process the system 
    printf("\nStarting to read systems from file...\n");
    if(fscanf(fp, "%d %d", &n_row, &m_col) != 2){/* Todo: Handle error*/}

// --- validation and header consumption for A and b ---
    if (n_row <= 0 || n_row > MAX_SIZE) { /* Todo: Handle validation*/ }
    if (m_col != 1) { /* Todo: handle validation */ }
    fgets(buffer, MAXSTR, fp); // consume rest of N M line
    fgets(buffer, MAXSTR, fp); // consume header before A

    printf("\n--- Processing System (N=%d) from %s ---\n", n_row, input_filename);
    printf("Reading Matrix A (%d x %d):\n", n_row, n_row);
    for (k = 0; k < n_row; k++) { // read A
        for (l = 0; l < n_row; l++) {
            if (fscanf(fp, "%f", &A[k][l]) != 1) { nrerror("Error reading matrix A");}
        }
    }
    fgets(buffer, MAXSTR, fp); // consume line after A
    fgets(buffer, MAXSTR, fp); // consume header before b

    printf("Reading Vector b (%d x 1):\n", n_row);
    for (k = 0; k < n_row; k++) { // Read b
        if (fscanf(fp, "%f", &b[k]) != 1) { nrerror("Error reading vector b");}
    }

    print_matrix(A, n_row,  n_row, "Original A");
    print_vector(b,  n_row, "Original b");


    // --- solve using selected method ---
    int solve_success = 1;
    // GAUSS_JORDAN
    printf("\nAttempting Gauss-Jordan Elimination...\n");
    for (k = 0; k < n_row; k++) {
        for (l = 0; l < n_row; l++) { Aug[k][l] = A[k][l]; }
        Aug[k][n_row] = b[k];
    }

    print_matrix(Aug, n_row,  n_row + 1, "initial Augmented [A|b]");
    gauss_jordan_partial(Aug, n_row); /* TODO: Does this function call guarantee to succeed? If not add a error handling */
    printf("Gauss-Jordan complete.\n");
    print_matrix(Aug, n_row, n_row + 1, "Final Augmented [I|x]");
    for (k = 0; k < n_row; k++)  x[k] = Aug[k][n_row]; 


    // --- print and verify solution ---
    if (solve_success) {
        print_vector(x, n_row, "Solution x");
        printf("Verifying solution (Calculating A * x)...\n");
        for (k = 0; k < n_row; k++) {
            check[k] = 0.0;
            for (j = 0; j < n_row; j++) { check[k] += A[k][j] * x[j]; }
        }
        print_vector(check,  n_row, "Calculated A*x");
        printf("Comparing A*x with original b:\n");
        
        int errors = 0;
        for (k = 0; k < n_row; k++) {
            if (fabs(check[k] - b[k]) > TOL) {
                printf("  Mismatch at index [%d]: ...\n", k); errors++;
            }
        }
        if (errors == 0) { printf("  Verification successful...\n"); }
        else { printf("  Verification FAILED...\n"); }

    } else {
            printf("\nSkipping verification for this system due to solver incompatibility or failure.\n");
    }

    printf("\n------------------------------------\n\n"); // separator

    /* check if the file has been consumed completely. */
    if (!feof(fp)) {
        fprintf(stderr, "Warning: File processing stopped before reaching end-of-file. Check file format near last processed system.\n");
    }

    fclose(fp);
    printf("File processing complete for %s.\n", input_filename);

    // --- Free Memory ---
    printf("Freeing memory...\n");
    free_matrix(A);
    free_vector(b);
    free_vector(x);
    free_matrix(Aug );
    free_matrix(A_chol);
    free_vector(check);

    printf("Done.\n");
    return 0;
}


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

void sin_vector(float *x, int N){
    for (int i = 0; i < N; i++) {
        x[i] = sin((float)i);
    }
    printf("sine over solution.\n");
}
