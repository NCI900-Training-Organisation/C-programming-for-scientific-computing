      
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Needed for strcmp
#include <math.h>   // Needed for fabs, sqrt

// Include LAPACKE header for C interface
#include "mkl_lapacke.h"

// Assuming na_util.h provides BOTH float and double versions:
// float: vector, matrix, free_vector, free_matrix
// double: dvector, dmatrix, free_dvector, free_dmatrix
// And nrerror. Ensure these exist!
#include "na.h"   
#include "na_util.h"

#define NP 5000     // Max N dimension
#define MAXSTR 80
#define TOL_FLOAT 1.0e-6  // Tolerance for float comparisons
#define TOL_DOUBLE 1.0e-9 // Tolerance for double comparisons

// --- Solver Method Enum ---
typedef enum { GAUSS_JORDAN, CHOLESKY_CUSTOM, CHOLESKY_LAPACK } SolverMethod;




// --- Main function modified ---
int main(int argc, char *argv[])
{
    int j, k, l, n;
    int m_read;

    // === Data Structures (Mixed Precision) ===
    // Read input into double precision structures
    double **a_input = NULL;    // Original matrix read from file
    double *b_input = NULL;     // Original RHS read from file
    double *x = NULL;           // Solution vector (always double for consistency)
    double *check = NULL;       // Verification vector (double)

    // Structures for specific solvers
    float **a_chol_custom = NULL; // Float copy for custom Cholesky
    float *b_custom = NULL;      // Float copy of RHS for custom Cholesky
    float *x_custom = NULL;      // Float solution from custom Cholesky

    double **a_chol_lapack = NULL; // Double copy for LAPACK Cholesky

    float **Aug = NULL;          // Float augmented matrix for Gauss-Jordan

    // Temporary 1D arrays for LAPACK (contiguous memory)
    double *a_lapack_1d = NULL;
    double *b_lapack_1d = NULL;

    char dummy[MAXSTR];
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


    // --- Allocate Primary Double Precision Structures ---
    // !!! Ensure your na_util provides these !!!
    a_input = dmatrix(1, NP, 1, NP);
    b_input = dvector(1, NP);
    x = dvector(1, NP);        // Solution always stored as double
    check = dvector(1, NP);    // Verification always uses double


    // --- Open the specified input file ---
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

    // --- Consume Initial Header Lines ---
    // Assumes first two lines are headers before the first N M line
    printf("Skipping initial header lines...\n");
    if (fgets(dummy, MAXSTR, fp) == NULL) { nrerror("Error reading first header or empty file."); }
    if (fgets(dummy, MAXSTR, fp) == NULL) { nrerror("Error reading second header or file too short."); }

    // --- Read the SINGLE system ---
    printf("\nReading the first system from file...\n");

    // --- Read dimensions N M ---
    if (fscanf(fp, " %d %d", &n, &m_read) != 2) {
        nrerror("Error reading matrix dimensions (N M) for the first system.");
    }

    // --- Validation ---
    if (n <= 0 || n > NP) {
        fprintf(stderr, "Error: Invalid dimension N=%d (Max N=%d).\n", n, NP);
        fclose(fp); // Close file before exiting
        exit(EXIT_FAILURE);
    }
    if (m_read != 1) {
        fprintf(stderr, "Warning: File specifies M=%d, but expecting M=1 for Ax=b. Proceeding anyway.\n", m_read);
    }

    // --- Consume headers/lines before A ---
    fgets(dummy, MAXSTR, fp); // Consume rest of N M line
    fgets(dummy, MAXSTR, fp); // Consume header before A

    // --- Read Matrix A (into double) ---
    printf("Reading Matrix A (%d x %d) as double:\n", n, n);
    for (k = 1; k <= n; k++) {
        for (l = 1; l <= n; l++) {
            if (fscanf(fp, "%lf", &a_input[k][l]) != 1) { nrerror("Error reading matrix A data");}
        }
    }

    // --- Consume headers/lines before b ---
    fgets(dummy, MAXSTR, fp); // Consume line after A data
    fgets(dummy, MAXSTR, fp); // Consume header before b

    // --- Read Vector b (into double) ---
    printf("Reading Vector b (%d x 1) as double:\n", n);
    for (k = 1; k <= n; k++) {
         if (fscanf(fp, "%lf", &b_input[k]) != 1) { nrerror("Error reading vector b data");}
         // Consume rest of line in case M > 1 was specified
         while (fgetc(fp) != '\n' && !feof(fp));
    }

    fclose(fp); // Close the file, we've read all we need
    printf("Finished reading data from file.\n");

    //print_nr_dmatrix(a_input, 1, n, 1, n, "Original A (Double)");
    //print_nr_dvector(b_input, 1, n, "Original b (Double)");

    // --- Allocate solver-specific structures and Solve ---
    int solve_success = 1;

    if (method == CHOLESKY_LAPACK) {
        // Allocate double copy based on actual size n
        a_chol_lapack = dmatrix(1, n, 1, n);
        for(k=1; k<=n; ++k) for(l=1; l<=n; ++l) a_chol_lapack[k][l] = a_input[k][l];

        printf("\nAttempting Cholesky Decomposition using LAPACKE_dpotrf...\n");
        if (!is_symmetric_double(a_chol_lapack, n)) {
            fprintf(stderr, "ERROR: Matrix A is not symmetric. Cholesky method cannot be used.\n");
            solve_success = 0;
            // Free memory allocated ONLY for this block if failing early
            free_dmatrix(a_chol_lapack, 1, 1); a_chol_lapack = NULL;
        } else {
            printf("Matrix appears symmetric. Proceeding with LAPACK.\n");
            // Allocate 1D arrays
            a_lapack_1d = (double*)malloc(n * n * sizeof(double));
            b_lapack_1d = (double*)malloc(n * sizeof(double));
            if (!a_lapack_1d || !b_lapack_1d) nrerror("Memory allocation failed for LAPACK arrays");

            // Copy to 1D arrays
            for (k = 1; k <= n; k++) for (l = 1; l <= n; l++) a_lapack_1d[(k - 1) * n + (l - 1)] = a_chol_lapack[k][l];
            for (k = 1; k <= n; k++) b_lapack_1d[k - 1] = b_input[k];

            // Call LAPACKE_dpotrf
            info = LAPACKE_dpotrf(LAPACK_ROW_MAJOR, 'U', n, a_lapack_1d, n);
            if (info != 0) { /* error handling */ solve_success = 0; }
            else {
                // Call LAPACKE_dpotrs
                info = LAPACKE_dpotrs(LAPACK_ROW_MAJOR, 'U', n, 1, a_lapack_1d, n, b_lapack_1d, 1);
                if (info != 0) { /* error handling */ solve_success = 0; }
                else { /* copy solution */ for (k = 1; k <= n; k++) x[k] = b_lapack_1d[k - 1]; }
            }
            // Free 1D arrays
            if(a_lapack_1d) {free(a_lapack_1d); a_lapack_1d = NULL;}
            if(b_lapack_1d) {free(b_lapack_1d); b_lapack_1d = NULL;}
            // Free NR copy
            if(a_chol_lapack) {free_dmatrix(a_chol_lapack, 1, 1); a_chol_lapack = NULL;}
        }

    } else if (method == CHOLESKY_CUSTOM) {
        // Allocate float structures based on actual size n
        a_chol_custom = matrix(1, n, 1, n);
        b_custom = vector(1, n);
        x_custom = vector(1, n);

        // Copy double input to float structures
        for(k=1; k<=n; ++k) b_custom[k] = (float)b_input[k];
        for(k=1; k<=n; ++k) for(l=1; l<=n; ++l) a_chol_custom[k][l] = (float)a_input[k][l];

        printf("\nAttempting Custom Cholesky Decomposition (Float)...\n");
        if (!is_symmetric(a_chol_custom, n)) { // Use float version of is_symmetric
            fprintf(stderr, "ERROR: Matrix A is not symmetric...\n");
            solve_success = 0;
            // Free memory allocated ONLY for this block if failing early
            free_matrix(a_chol_custom, 1, 1); a_chol_custom = NULL;
            free_vector(b_custom, 1, n); b_custom = NULL; // Use n here if free_vector needs nh
            free_vector(x_custom, 1, n); x_custom = NULL; // Use n here if free_vector needs nh
        } else {
            printf("Matrix appears symmetric. Proceeding...\n");
            cholesky(a_chol_custom, n);
            // print_nr_matrix(a_chol_custom, 1, n, 1, n, "Decomposed A (Float)");
            cholesky_solve(a_chol_custom, b_custom, x_custom, n);
            for(k=1; k<=n; ++k) x[k] = (double)x_custom[k]; // Copy solution to double x

            // Free memory after successful use
            free_matrix(a_chol_custom, 1, 1); a_chol_custom = NULL;
            free_vector(b_custom, 1, n); b_custom = NULL; // Use n here
            free_vector(x_custom, 1, n); x_custom = NULL; // Use n here
        }

    } else { // GAUSS_JORDAN
        // Allocate float augmented matrix based on actual size n
         Aug = matrix(1, n, 1, n + 1);

         printf("\nAttempting Gauss-Jordan Elimination (Float)...\n");
         // Copy double input to float Aug matrix
         for (k = 1; k <= n; k++) {
             for (l = 1; l <= n; l++) { Aug[k][l] = (float)a_input[k][l]; }
             Aug[k][n + 1] = (float)b_input[k];
         }
         // print_nr_matrix(Aug, 1, n, 1, n + 1, "Initial Aug (Float)");

         gauss_jordan_partial(Aug, n); // Modifies Aug

         printf("Gauss-Jordan complete.\n");
         // print_nr_matrix(Aug, 1, n, 1, n + 1, "Final Aug (Float)");

         // Extract solution x (convert float result back to double)
         for (k = 1; k <= n; k++) {
             x[k] = (double)Aug[k][n + 1];
         }
         // Free float augmented matrix
         free_matrix(Aug, 1, 1); Aug = NULL;
    } // end of solver methods

    // --- Print and Verify Solution (using double precision) ---
    if (solve_success) {
         //print_nr_dvector(x, 1, n, "Solution x (Double)"); // Use n here
         printf("Verifying solution (Calculating A * x)...\n");
         for (k = 1; k <= n; k++) { // Use original double 'a_input'
             check[k] = 0.0;
             for (j = 1; j <= n; j++) { check[k] += a_input[k][j] * x[j]; }
         }
         //print_nr_dvector(check, 1, n, "Calculated A*x (Double)"); // Use n here
         printf("Comparing A*x with original b:\n");
         int errors = 0;
         for (k = 1; k <= n; k++) {
              if (fabs(check[k] - b_input[k]) > TOL_FLOAT) { // Use double TOL
                  printf("  Mismatch at index [%d]: Expected %.6e, Got %.6e (Diff: %.4e)\n",
                         k, b_input[k], check[k], check[k] - b_input[k]); errors++;
              }
         }
         if (errors == 0) { printf("  Verification successful (within tolerance %.1e).\n", TOL_DOUBLE); }
         else { printf("  Verification FAILED with %d mismatches.\n", errors); }
    } else {
         printf("\nSkipping verification due to solver incompatibility or failure.\n");
    }

    // --- Final Cleanup ---
    // Free structures allocated *outside* the loop (using NP)
    printf("Freeing memory...\n");
    if(a_input) free_dmatrix(a_input, 1, 1); // Use 1,1 for nrl,ncl
    if(b_input) free_dvector(b_input, 1, NP); // Use NP if allocated with NP
    if(x) free_dvector(x, 1, NP);       // Use NP if allocated with NP
    if(check) free_dvector(check, 1, NP); // Use NP if allocated with NP

    // Structures allocated inside the processing block should be NULL now
    // ASSERT(Aug == NULL);
    // ASSERT(a_chol_custom == NULL);
    // ASSERT(b_custom == NULL);
    // etc.

    printf("Done.\n");
    return 0;
}
