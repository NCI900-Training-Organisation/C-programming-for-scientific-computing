      
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <math.h>   
#include "primitives.h"   
#include "util.h"

#include "mkl_lapacke.h"



#define MAX_SIZE 5000     // Max 
#define MAXSTR 80
#define TOL_FLOAT 1.0e-6  // Tolerance for float comparisons
#define TOL_DOUBLE 1.0e-9 // Tolerance for double comparisons

// --- Solver Method Enum ---
typedef enum { GAUSS_JORDAN, CHOLESKY_CUSTOM, CHOLESKY_LAPACK } SolverMethod;




// --- Main function modified ---
int main(int argc, char *argv[])
{
    int j, k, l;
    int n_row, m_col;

    // === Data Structures (Mixed Precision) ===
    // Read input into double precision structures
    double **A; 
    double *b;
    double *x;
    double *check; 

    // Structures for specific solvers
    float **A_chol_custom; 
    float *b_custom; 
    float *x_custom;

    double **A_chol_lapack; 

    float **Aug; 

    // Temporary 1D arrays for LAPACK (contiguous memory)
    double *A_lapack_1d; 
    double *b_lapack_1d; 

    char buffer[MAXSTR];
    FILE *fp;
    char *input_filename = NULL;
    SolverMethod method = GAUSS_JORDAN; // Default method
    lapack_int info; // LAPACK return code

    // --- Parse Command-Line Arguments ---
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-g | -c | -cl] <matrix_data_file>\n", argv[0]);
        fprintf(stderr, "  -g : Use Gauss-Jordan (float)\n");
        fprintf(stderr, "  -c : Use Custom Cholesky (float)\n");
        fprintf(stderr, "  -cl: Use LAPACK Cholesky (double)\n");
        exit(EXIT_FAILURE);
    }

    // Check for optional flag
    if (argc > 2 && (argv[1][0] == '-' && (argv[1][1] == 'g' || argv[1][1] == 'c'))) {
        if (strcmp(argv[1], "-c") == 0) {
            method = CHOLESKY_CUSTOM;
        } else if (strcmp(argv[1], "-cl") == 0) {
            method = CHOLESKY_LAPACK;
        } else if (strcmp(argv[1], "-g") == 0) {
            method = GAUSS_JORDAN;
        } else {
             // Treat as filename if flag is unrecognized after '-'
             input_filename = argv[1];
              fprintf(stderr, "Warning: Unrecognized flag '%s'. Defaulting to Gauss-Jordan.\n", argv[1]);
              method = GAUSS_JORDAN;
              if(argc > 2) input_filename = argv[2]; // Assume filename is next if flag was invalid
               else { fprintf(stderr,"Error: Missing filename after potentially invalid flag.\n"); exit(EXIT_FAILURE);}

        }
         if(input_filename == NULL) input_filename = argv[2]; // Filename is the third argument if flag was valid

         if (argc > 3) fprintf(stderr, "Warning: Extra command line arguments ignored.\n");
    } else {
        // No flag or unrecognized flag, assume filename is the second argument
        input_filename = argv[1];
        method = GAUSS_JORDAN; // Default if no flag
        if (argc > 2) fprintf(stderr, "Warning: Arguments after filename ignored. Use -g/-c/-cl flags.\n");
    }


    // --- allocate Primary Double Precision Structures ---
    A = dmatrix(MAX_SIZE,  MAX_SIZE);
    b = dvector(MAX_SIZE);
    x = dvector(MAX_SIZE);        // Solution always stored as double
    check = dvector(MAX_SIZE);    // Verification always uses double


    // --- open the specified input file ---
    printf("Input file: %s\n", input_filename);
    const char* method_str = "Unknown";
    switch(method) {
        case GAUSS_JORDAN: method_str = "Gauss-Jordan (Float)"; break;
        case CHOLESKY_CUSTOM: method_str = "Cholesky (Custom Float)"; break;
        case CHOLESKY_LAPACK: method_str = "Cholesky (LAPACK Double)"; break;
    }
    printf("Using solver: %s\n", method_str);
    if ((fp = fopen(input_filename, "r")) == NULL) { nrerror("File open error"); }
    printf("Successfully opened file.\n");

    // --- consume initial header lines ---
    printf("Skipping initial header lines...\n");
    if (fgets(buffer, MAXSTR, fp) == NULL) { nrerror("Error reading first header or empty file."); }
    if (fgets(buffer, MAXSTR, fp) == NULL) { nrerror("Error reading second header or file too short."); }

    // --- read dimensions N M ---
    if (fscanf(fp, " %d %d", &n_row, &m_col) != 2) {
        nrerror("Error reading matrix dimensions (N M) for the first system.");
    }

    // --- validation ---
    if (n_row <= 0 || n_row > MAX_SIZE) {
        fprintf(stderr, "Error: Invalid dimension N=%d (Max N=%d).\n", n_row, MAX_SIZE);
        fclose(fp); // Close file before exiting
        exit(EXIT_FAILURE);
    }
    if (m_col != 1) {
        fprintf(stderr, "Warning: File specifies M=%d, but expecting M=1 for Ax=b. Proceeding anyway.\n", m_col);
    }

    // --- consume headers/lines before A ---
    fgets(buffer, MAXSTR, fp); // Consume rest of N M line
    fgets(buffer, MAXSTR, fp); // Consume header before A

    // --- read Matrix A (into double) ---
    printf("Reading Matrix A (%d x %d) as double:\n", n_row, n_row);
    for (k = 0; k < n_row; k++) {
        for (l = 0; l < n_row; l++) {
            if (fscanf(fp, "%lf", &A[k][l]) != 1) { nrerror("Error reading matrix A data");}
        }
    }

    // --- consume headers/lines before b ---
    fgets(buffer, MAXSTR, fp); // consume line after A data
    fgets(buffer, MAXSTR, fp); // consume header before b

    printf("Reading Vector b (%d x 1) as double:\n", n_row);
    for (k = 0; k < n_row; k++) {
         if (fscanf(fp, "%lf", &b[k]) != 1) { nrerror("Error reading vector b data");}
         // consume rest of line in case M > 1 was specified
         while (fgetc(fp) != '\n' && !feof(fp));
    }

    fclose(fp); // Close the file, we've read all we need
    printf("Finished reading data from file.\n");

    //print_nr_dmatrix(A, 1, n, 1, n, "Original A (Double)");
    //print_nr_dvector(b, 1, n, "Original b (Double)");

    int solve_success = 1;

    if (method == CHOLESKY_LAPACK) {
        // allocate double copy based on actual size n
        A_chol_lapack = dmatrix(n_row,  n_row);
        for(k=0; k<n_row; ++k) for(l=0; l<n_row; ++l) A_chol_lapack[k][l] = A[k][l];

        printf("\nAttempting Cholesky Decomposition using LAPACKE_dpotrf...\n");
        if (!is_symmetric_double(A_chol_lapack, n_row)) {
            fprintf(stderr, "ERROR: Matrix A is not symmetric. Cholesky method cannot be used.\n");
            solve_success = 0;
            // Free memory allocated ONLY for this block if failing early
            free_dmatrix(A_chol_lapack); A_chol_lapack = NULL;
        } else {
            printf("Matrix appears symmetric. Proceeding with LAPACK.\n");
            // Allocate 1D arrays
            A_lapack_1d = (double*)malloc(n_row * n_row * sizeof(double));
            b_lapack_1d = (double*)malloc(n_row * sizeof(double));
            if (!A_lapack_1d || !b_lapack_1d) nrerror("Memory allocation failed for LAPACK arrays");

            // Copy to 1D arrays
            for (k = 0; k < n_row; k++) for (l = 0; l < n_row; l++) A_lapack_1d[k  * n_row + l ] = A_chol_lapack[k][l];
            for (k = 0; k < n_row; k++) b_lapack_1d[k] = b[k];

            // Call LAPACKE_dpotrf
            info = LAPACKE_dpotrf(LAPACK_ROW_MAJOR, 'U', n_row, A_lapack_1d, n_row);
            if (info != 0) { /* error handling */ solve_success = 0; }
            else {
                // Call LAPACKE_dpotrs
                info = LAPACKE_dpotrs(LAPACK_ROW_MAJOR, 'U', n_row, 1, A_lapack_1d, n_row, b_lapack_1d, 1);
                if (info != 0) { /* error handling */ solve_success = 0; }
                else { /* copy solution */ for (k = 0; k < n_row; k++) x[k] = b_lapack_1d[k]; }
            }
            // Free 1D arrays
            if(A_lapack_1d) {free(A_lapack_1d); A_lapack_1d = NULL;}
            if(b_lapack_1d) {free(b_lapack_1d); b_lapack_1d = NULL;}
            if(A_chol_lapack) {free_dmatrix(A_chol_lapack); A_chol_lapack = NULL;}
        }

    } else if (method == CHOLESKY_CUSTOM) {
        // allocate float structures based on actual size n
        A_chol_custom = matrix(n_row,  n_row);
        b_custom = vector(n_row);
        x_custom = vector(n_row);

        // Copy double input to float structures
        for(k=0; k<n_row; ++k) b_custom[k] = (float)b[k];
        for(k=0; k<n_row; ++k) for(l=0; l<n_row; ++l) A_chol_custom[k][l] = (float)A[k][l];

        printf("\nAttempting Custom Cholesky Decomposition (Float)...\n");
        if (!is_symmetric(A_chol_custom, n_row)) { 
            fprintf(stderr, "ERROR: Matrix A is not symmetric...\n");
            solve_success = 0;
            // Free memory allocated ONLY for this block if failing early
            free_matrix(A_chol_custom); A_chol_custom = NULL;
            free_vector(b_custom); b_custom = NULL; 
            free_vector(x_custom); x_custom = NULL; 
        } else {
            printf("Matrix appears symmetric. Proceeding...\n");
            cholesky(A_chol_custom, n_row);
            // print_nr_matrix(a_chol_custom, 1, n, 1, n, "Decomposed A (Float)");
            cholesky_solve(A_chol_custom, b_custom, x_custom, n_row);
            for(k=0; k<n_row; ++k) x[k] = (double)x_custom[k]; // copy solution to double x

            // Free memory after successful use
            free_matrix(A_chol_custom); A_chol_custom = NULL;
            free_vector(b_custom); b_custom = NULL; 
            free_vector(x_custom); x_custom = NULL; 
        }

    } else { // GJ
        // allocate float augmented matrix based on actual size n
         Aug = matrix(n_row,  n_row + 1);

         printf("\nAttempting Gauss-Jordan Elimination (Float)...\n");
         // Copy double input to float Aug matrix
         for (k = 0; k < n_row; k++) {
             for (l = 0; l < n_row; l++) { Aug[k][l] = (float)A[k][l]; }
             Aug[k][n_row] = (float)b[k];
         }
         // print_nr_matrix(Aug, 1, n, 1, n + 1, "Initial Aug (Float)");

         gauss_jordan_partial(Aug, n_row); // Modifies Aug

         printf("Gauss-Jordan complete.\n");
         // print_nr_matrix(Aug, 1, n, 1, n + 1, "Final Aug (Float)");

         // extract solution x (convert float result back to double)
         for (k = 0; k < n_row; k++) {
             x[k] = (double)Aug[k][n_row];
         }
         // free float augmented matrix
         free_matrix(Aug); Aug = NULL;
    } // end of solver methods

    // --- print and verify solution ---
    if (solve_success) {
         //print_nr_dvector(x, 1, n, "Solution x (Double)"); 
         printf("Verifying solution (Calculating A * x)...\n");
         for (k = 0; k < n_row; k++) { // Use original double 'A'
             check[k] = 0.0;
             for (j = 0; j < n_row; j++) { check[k] += A[k][j] * x[j]; }
         }
         //print_nr_dvector(check, 1, n, "Calculated A*x (Double)"); 
         printf("Comparing A*x with original b:\n");
         int errors = 0;
         for (k = 0; k < n_row; k++) {
              if (fabs(check[k] - b[k]) > TOL_FLOAT) { // Use double TOL
                  printf("  Mismatch at index [%d]: Expected %.6e, Got %.6e (Diff: %.4e)\n",
                         k, b[k], check[k], check[k] - b[k]); errors++;
              }
         }
         if (errors == 0) { printf("  Verification successful (within tolerance %.1e).\n", TOL_DOUBLE); }
         else { printf("  Verification FAILED with %d mismatches.\n", errors); }
    } else {
         printf("\nSkipping verification due to solver incompatibility or failure.\n");
    }

  
    printf("Freeing memory...\n");
    free_dmatrix(A); 
    free_dvector(b); 
    free_dvector(x);       
    free_dvector(check); 

    printf("Done.\n");
    return 0;
}
