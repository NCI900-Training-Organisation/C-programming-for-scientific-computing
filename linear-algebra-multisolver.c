#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include "primitives.h"   
#include "util.h"

#define MAX_SIZE 20       // Max N dimension
#define MAXSTR 80
#define TOL 1.0e-6  // Tolerance for verification
#define TOL_DOUBLE 1.0e-9


// define solver method
typedef enum { GAUSS_JORDAN, CHOLESKY } SolverMethod;




// --- Main function modified ---
int main(int argc, char *argv[])
{

    int j, k, l;
    int n_row, m_col;
    float **A, **Aug, **A_chol;  // matrices for A, Augmented, and Cholesky
    float *b, *x, *check; // vectors for b, x, and check
    char buffer[MAXSTR];
    FILE *fp;
    char *input_filename = NULL;
    SolverMethod method = GAUSS_JORDAN;

    // --- parse command line Arguments ---
     if (argc < 2) { /* usage */ exit(EXIT_FAILURE); }

     // Check for optional flag -g or -c
     if (argc > 2 && (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "-g") == 0)) {
         if (strcmp(argv[1], "-c") == 0) method = CHOLESKY;
         else method = GAUSS_JORDAN;
         input_filename = argv[2];
         if (argc > 3) fprintf(stderr, "Warning: Extra arguments ignored.\n");
     } else {
         input_filename = argv[1]; 
         if (argc > 2) fprintf(stderr, "Warning: Treating '%s' as filename. Use -g or -c flag.\n", argv[1]);
     }


    // --- allocate memory for matrices vectors  ---
    A = matrix(MAX_SIZE,  MAX_SIZE);
    b = vector(MAX_SIZE);
    x = vector(MAX_SIZE);
    Aug = matrix(MAX_SIZE, MAX_SIZE + 1);
    A_chol = matrix(MAX_SIZE, MAX_SIZE);
    check = vector(MAX_SIZE);

    // --- open the specified input file ---
    printf("Input file: %s\n", input_filename);
    printf("Using solver: %s\n", (method == CHOLESKY) ? "Cholesky" : "Gauss-Jordan");
    if ((fp = fopen(input_filename, "r")) == NULL) { /* error */ nrerror("..."); }
    printf("Successfully opened file.\n");


    if (fgets(buffer, MAXSTR, fp) == NULL) { /* Todo: Handle error */ }
    if (fgets(buffer, MAXSTR, fp) == NULL) { /* Todo: Handle error */ }


    // process the system 
    printf("\nStarting to read systems from file...\n");
    if(fscanf(fp, "%d %d", &n_row, &m_col) != 2) {}

    // --- validation and header consumption for A and b ---
    if (n_row <= 0 || n_row > MAX_SIZE) { /* Todo: Handle validation */ }
    if (m_col != 1) { /* Todo: handle validation */ }
    fgets(buffer, MAXSTR, fp); // consume rest of N M line
    fgets(buffer, MAXSTR, fp); // consume header before A

    printf("\n--- Processing System (N=%d) from %s ---\n", n_row, input_filename);
    printf("Reading Matrix A (%d x %d):\n", n_row, n_row);
    for (k = 0; k < n_row; k++) { // 
        for (l = 0; l < n_row; l++) {
            if (fscanf(fp, "%f", &A[k][l]) != 1) { nrerror("Error reading matrix A");}
            A_chol[k][l] = A[k][l]; // copy A
        }
    }
    fgets(buffer, MAXSTR, fp); // consume line after A
    fgets(buffer, MAXSTR, fp); // consume header before b

    printf("Reading Vector b (%d x 1):\n", n_row);
    for (k = 0; k < n_row; k++) { // 
        if (fscanf(fp, "%f", &b[k]) != 1) { nrerror("Error reading vector b");}
    }

    print_matrix(A, n_row,  n_row, "Original A");
    print_vector(b,  n_row, "Original b");


    // --- solve using selected method ---
    int solve_success = 1;
    if (method == CHOLESKY) {
        printf("\nAttempting Cholesky Decomposition...\n");
        if (!is_symmetric(A, n_row)) {
            fprintf(stderr, "ERROR: Matrix A is not symmetric. Cholesky method cannot be used.\n");
            solve_success = 0; 
        } else {
            printf("Matrix is symmetric. Proceeding with Cholesky.\n");

            cholesky(A_chol, n_row); 
            printf("Cholesky decomposition successful.\n");
            print_matrix(A_chol, n_row, n_row, "Decomposed A (L factor)");
            cholesky_solve(A_chol, b, x, n_row); // solve using decomposed matrix
            printf("Cholesky solve complete.\n");
        }
    } else { // Gauss-Jordan solver

            printf("\nAttempting Gauss-Jordan Elimination...\n");
            for (k = 0; k < n_row; k++) {
                for (l = 0; l < n_row; l++) { Aug[k][l] = A[k][l]; }
                Aug[k][n_row] = b[k];
            }
            print_matrix(Aug, n_row, n_row + 1, "Initial Augmented [A|b]");
            gauss_jordan_partial(Aug, n_row); 
            printf("Gauss-Jordan complete.\n");
            print_matrix(Aug, n_row, n_row + 1, "Final Augmented [I|x]");
            for (k = 0; k < n_row; k++) { x[k] = Aug[k][n_row]; }
    }

    // ---  verify solution ---
    if (solve_success) {
            print_vector(x,  n_row, "Solution x");
            printf("Verifying solution (Calculating A * x)...\n");
            for (k = 0; k < n_row; k++) {
                check[k] = 0.0;
                for (j = 0; j < n_row; j++) { check[k] += A[k][j] * x[j]; }
            }
            print_vector(check, n_row, "Calculated A*x");
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
    free_matrix(Aug);
    free_matrix(A_chol);
    free_vector(check);

    printf("Done.\n");
    return 0;
}