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


// --- Solver Method Enum 
typedef enum { GAUSS_JORDAN, CHOLESKY } SolverMethod;




// --- Main function modified ---
int main(int argc, char *argv[])
{
    // ... [Declarations and Argument Parsing remain the same] ...
    int j, k, l, n;
    int m_read;
    float **A, **Aug, **A_chol;  // matrices for A, Augmented, and Cholesky
    float *b, *x, *check; // vectors for b, x, and check
    char buffer[MAXSTR];
    FILE *fp;
    char *input_filename = NULL;
    SolverMethod method = GAUSS_JORDAN;

    // --- Parse Command-Line Arguments (remains the same) ---
     if (argc < 2) { /* usage */ exit(EXIT_FAILURE); }

     // Check for optional flag -g or -c
     if (argc > 2 && (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "-g") == 0)) {
         if (strcmp(argv[1], "-c") == 0) method = CHOLESKY;
         else method = GAUSS_JORDAN;
         input_filename = argv[2];
         if (argc > 3) fprintf(stderr, "Warning: Extra arguments ignored.\n");
     } else {
         input_filename = argv[1]; // Assume filename is first argument
         if (argc > 2) fprintf(stderr, "Warning: Treating '%s' as filename. Use -g or -c flag.\n", argv[1]);
     }


    // --- Allocate Matrices/Vectors (remains the same) ---
    A = matrix(MAX_SIZE,  MAX_SIZE);
    b = vector(MAX_SIZE);
    x = vector(MAX_SIZE);
    Aug = matrix(MAX_SIZE, MAX_SIZE + 1);
    A_chol = matrix(MAX_SIZE, MAX_SIZE);
    check = vector(MAX_SIZE);

    // --- Open the specified input file (remains the same) ---
    printf("Input file: %s\n", input_filename);
    printf("Using solver: %s\n", (method == CHOLESKY) ? "Cholesky" : "Gauss-Jordan");
    if ((fp = fopen(input_filename, "r")) == NULL) { /* error */ nrerror("..."); }
    printf("Successfully opened file.\n");

    // --- Consume Initial Header Lines (remains the same) ---
    if (fgets(buffer, MAXSTR, fp) == NULL) { /* Handle error */ }
    if (fgets(buffer, MAXSTR, fp) == NULL) { /* Handle error */ }


    // --- Loop to process systems of equations ---
    printf("\nStarting to read systems from file...\n");
    if(fscanf(fp, "%d %d", &n, &m_read) != 2) {}

    // --- Validation and Header Consumption for A and b ---
    // ... [This part remains the same: read N, M, headers before A, matrix A, headers before b, vector b] ...
    if (n <= 0 || n > MAX_SIZE) { /* Validation */ }
    if (m_read != 1) { /* Warning */ }
    fgets(buffer, MAXSTR, fp); // Consume rest of N M line
    fgets(buffer, MAXSTR, fp); // Consume header before A

    printf("\n--- Processing System (N=%d) from %s ---\n", n, input_filename);
    printf("Reading Matrix A (%d x %d):\n", n, n);
    for (k = 0; k < n; k++) { // Read A
        for (l = 0; l < n; l++) {
            if (fscanf(fp, "%f", &A[k][l]) != 1) { nrerror("Error reading matrix A");}
            A_chol[k][l] = A[k][l]; // COPY A
        }
    }
    fgets(buffer, MAXSTR, fp); // Consume line after A
    fgets(buffer, MAXSTR, fp); // Consume header before b

    printf("Reading Vector b (%d x 1):\n", n);
    for (k = 0; k < n; k++) { // Read b
        if (fscanf(fp, "%f", &b[k]) != 1) { nrerror("Error reading vector b");}
    }

    print_matrix(A, n,  n, "Original A");
    print_vector(b,  n, "Original b");


    // --- Solve using selected method ---
    int solve_success = 1;
    if (method == CHOLESKY) {
        printf("\nAttempting Cholesky Decomposition...\n");
        if (!is_symmetric(A, n)) {
            fprintf(stderr, "ERROR: Matrix A is not symmetric. Cholesky method cannot be used.\n");
            solve_success = 0; // Mark as failed
        } else {
            printf("Matrix is symmetric. Proceeding with Cholesky.\n");
            // Use try-catch or setjmp/longjmp if nrerror doesn't exit
            // For simplicity, assume nrerror exits if not positive-definite
            cholesky(A_chol, n); // Modifies a_chol, might exit
            printf("Cholesky decomposition successful.\n");
            print_matrix(A_chol, n, n, "Decomposed A (L factor)");
            cholesky_solve(A_chol, b, x, n); // Solve using decomposed matrix
            printf("Cholesky solve complete.\n");
        }
    } else { // GAUSS_JORDAN
            // ... [Gauss-Jordan logic remains the same] ...
            printf("\nAttempting Gauss-Jordan Elimination...\n");
            for (k = 0; k < n; k++) {
                for (l = 0; l < n; l++) { Aug[k][l] = A[k][l]; }
                Aug[k][n ] = b[k];
            }
            print_matrix(Aug, n, n + 1, "Initial Augmented [A|b]");
            gauss_jordan_partial(Aug, n); // Modifies Aug, might exit
            printf("Gauss-Jordan complete.\n");
            print_matrix(Aug, n, n + 1, "Final Augmented [I|x]");
            for (k = 0; k < n; k++) { x[k] = Aug[k][n ]; }
    }

    // --- Print and Verify Solution ---
    if (solve_success) {
        // ... [Verification logic remains the same] ...
            print_vector(x,  n, "Solution x");
            printf("Verifying solution (Calculating A * x)...\n");
            for (k = 0; k < n; k++) {
                check[k] = 0.0;
                for (j = 0; j < n; j++) { check[k] += A[k][j] * x[j]; }
            }
            print_vector(check, n, "Calculated A*x");
            printf("Comparing A*x with original b:\n");
            int errors = 0;
            for (k = 0; k < n; k++) {
                if (fabs(check[k] - b[k]) > TOL) {
                    printf("  Mismatch at index [%d]: ...\n", k); errors++;
                }
            }
            if (errors == 0) { printf("  Verification successful...\n"); }
            else { printf("  Verification FAILED...\n"); }

    } else {
            printf("\nSkipping verification for this system due to solver incompatibility or failure.\n");
    }
    printf("\n------------------------------------\n\n"); // Separator




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