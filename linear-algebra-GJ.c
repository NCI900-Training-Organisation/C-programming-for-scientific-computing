#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // Needed for fabs in verification step
#include "na.h"   // Assumed to contain the definition of gauss_jordan_partial
#include "na_util.h" // Assumed to contain matrix(), free_matrix(), vector(), free_vector(), nrerror()

#define NP 20       // Max N dimension
#define MAXSTR 80
#define TOL 1.0e-6 // Tolerance for verification



// --- Forward Declarations ---
int is_symmetric(float **a, int n);
void gauss_jordan_partial(float **A, int N);



// --- Main function modified ---
int main(int argc, char *argv[])
{
    // ... [Declarations and Argument Parsing remain the same] ...
    int j, k, l, n;
    int m_read;
    float **a, **Aug, **a_chol;
    float *b, *x, *check;
    char dummy[MAXSTR];
    FILE *fp;
    char *input_filename = NULL;


    // --- Check Command-Line Arguments ---
    if (argc != 2) {
        // Print usage instruction to standard error
        fprintf(stderr, "Usage: %s <matrix_data_file>\n", argv[0]);
        // Exit indicating an error
        exit(EXIT_FAILURE); // Or return 1;
    }
    input_filename = argv[1]; // Get filename from the command line


    // --- Allocate Matrices/Vectors (remains the same) ---
    a = matrix(1, NP, 1, NP);
    b = vector(1, NP);
    x = vector(1, NP);
    Aug = matrix(1, NP, 1, NP + 1);
    a_chol = matrix(1, NP, 1, NP);
    check = vector(1, NP);

    // --- Open the specified input file (remains the same) ---
    printf("Input file: %s\n", input_filename);
    if ((fp = fopen(input_filename, "r")) == NULL) { /* error */ nrerror("..."); }
    printf("Successfully opened file.\n");

    // --- Consume Initial Header Lines (remains the same) ---
    if (fgets(dummy, MAXSTR, fp) == NULL) { /* Handle error */ }
    if (fgets(dummy, MAXSTR, fp) == NULL) { /* Handle error */ }


    // --- Loop to process systems of equations ---
    printf("\nStarting to read systems from file...\n");
    while (fscanf(fp, "%d %d", &n, &m_read) == 2) {

        // --- Validation and Header Consumption for A and b ---
        // ... [This part remains the same: read N, M, headers before A, matrix A, headers before b, vector b] ...
         if (n <= 0 || n > NP) { /* Validation */ break; }
         if (m_read != 1) { /* Warning */ }
         fgets(dummy, MAXSTR, fp); // Consume rest of N M line
         fgets(dummy, MAXSTR, fp); // Consume header before A

         printf("\n--- Processing System (N=%d) from %s ---\n", n, input_filename);
         printf("Reading Matrix A (%d x %d):\n", n, n);
         for (k = 1; k <= n; k++) { // Read A
             for (l = 1; l <= n; l++) {
                 if (fscanf(fp, "%f", &a[k][l]) != 1) { nrerror("Error reading matrix A");}
                 a_chol[k][l] = a[k][l]; // COPY A
             }
         }
         fgets(dummy, MAXSTR, fp); // Consume line after A
         fgets(dummy, MAXSTR, fp); // Consume header before b

         printf("Reading Vector b (%d x 1):\n", n);
         for (k = 1; k <= n; k++) { // Read b
              if (fscanf(fp, "%f", &b[k]) != 1) { nrerror("Error reading vector b");}
              while (fgetc(fp) != '\n' && !feof(fp)); // Consume rest of line
         }

         print_nr_matrix(a, 1, n, 1, n, "Original A");
         print_nr_vector(b, 1, n, "Original b");


        // --- Solve using selected method ---
        int solve_success = 1;
        // GAUSS_JORDAN
        // ... [Gauss-Jordan logic remains the same] ...
        printf("\nAttempting Gauss-Jordan Elimination...\n");
        for (k = 1; k <= n; k++) {
            for (l = 1; l <= n; l++) { Aug[k][l] = a[k][l]; }
            Aug[k][n + 1] = b[k];
        }
        print_nr_matrix(Aug, 1, n, 1, n + 1, "Initial Augmented [A|b]");
        gauss_jordan_partial(Aug, n); // Modifies Aug, might exit
        printf("Gauss-Jordan complete.\n");
        print_nr_matrix(Aug, 1, n, 1, n + 1, "Final Augmented [I|x]");
        for (k = 1; k <= n; k++) { x[k] = Aug[k][n + 1]; }
        

        // --- Print and Verify Solution ---
        if (solve_success) {
            // ... [Verification logic remains the same] ...
             print_nr_vector(x, 1, n, "Solution x");
             printf("Verifying solution (Calculating A * x)...\n");
             for (k = 1; k <= n; k++) {
                 check[k] = 0.0;
                 for (j = 1; j <= n; j++) { check[k] += a[k][j] * x[j]; }
             }
             print_nr_vector(check, 1, n, "Calculated A*x");
             printf("Comparing A*x with original b:\n");
             int errors = 0;
             for (k = 1; k <= n; k++) {
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


        // Check for end-of-file inside loop
        int ch = fgetc(fp);
        if (ch == EOF) break;
        ungetc(ch, fp);


    } // End while loop reading file

    fclose(fp);
    printf("File processing complete for %s.\n", input_filename);

    // --- Free Memory ---
    printf("Freeing memory...\n");
    free_matrix(a, 1,  1);
    free_vector(b, 1, NP);
    free_vector(x, 1, NP);
    free_matrix(Aug, 1, 1 );
    free_matrix(a_chol, 1,  1);
    free_vector(check, 1, NP);

    printf("Done.\n");
    return 0;
}