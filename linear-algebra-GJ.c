#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include "primitives.h"   
#include "util.h" 

#define MAX_SIZE 20       // Max N dimension
#define MAXSTR 80
#define TOL 1.0e-6 // tolerance 




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
        // print usage instruction to standard error
        fprintf(stderr, "Usage: %s <matrix_data_file>\n", argv[0]);
        // exit indicating an error
        exit(EXIT_FAILURE); // or return 1 (this is a unix thing);
    }

    input_filename = argv[1]; // get filename from the command line


    //  allocate memory for matrices an vectors  
    A = matrix(MAX_SIZE, MAX_SIZE);
    b = vector(MAX_SIZE);
    x = vector(MAX_SIZE);
    Aug = matrix(MAX_SIZE, MAX_SIZE + 1);
    A_chol = matrix(MAX_SIZE, MAX_SIZE);
    check = vector(MAX_SIZE);

    //  input file 
    printf("Input file: %s\n", input_filename);
    if ((fp = fopen(input_filename, "r")) == NULL) { /* Todo: Handle error */ nrerror("..."); }
    printf("Successfully opened file.\n");

    // consume initial header 
    if (fgets(buffer, MAXSTR, fp) == NULL) { /* Todo: Handle error */ }
    if (fgets(buffer, MAXSTR, fp) == NULL) { /* Todo: Handle error */ }


    // process the system 
    printf("\nStarting to read systems from file...\n");
    if(fscanf(fp, "%d %d", &n_row, &m_col) != 2){}

//  validation and header consumption for A and b 
    if (n_row <= 0 || n_row > MAX_SIZE) { /* Todo: Handle validation*/ }
    if (m_col != 1) { /* Todo: handle validation */ }
    fgets(buffer, MAXSTR, fp); // consume rest of N M line
    fgets(buffer, MAXSTR, fp); // consume header before A

    printf("\n--- Processing System (N=%d) from %s ---\n", n_row, input_filename);
    printf("Reading Matrix A (%d x %d):\n", n_row, n_row);
    for (k = 0; k < n_row; k++) { // Read A
        for (l = 0; l < n_row; l++) {
            if (fscanf(fp, "%f", &A[k][l]) != 1) { nrerror("Error reading matrix A");}
        }
    }
    fgets(buffer, MAXSTR, fp); // Consume line after A
    fgets(buffer, MAXSTR, fp); // Consume header before b

    printf("Reading Vector b (%d x 1):\n", n_row);
    for (k = 0; k < n_row; k++) { // Read b
        if (fscanf(fp, "%f", &b[k]) != 1) { nrerror("Error reading vector b");}
    }

    print_matrix(A, n_row,  n_row, "Original A");
    print_vector(b,  n_row, "Original b");


    //  solve using selected method 
    int solve_success = 1;
    // GAUSS_JORDAN
    printf("\nAttempting Gauss-Jordan Elimination...\n");
    for (k = 0; k < n_row; k++) {
        for (l = 0; l < n_row; l++) { Aug[k][l] = A[k][l]; }
        Aug[k][n_row] = b[k];
    }

    print_matrix(Aug, n_row,  n_row + 1, "initial Augmented [A|b]");
    gauss_jordan_partial(Aug, n_row); // modifies Aug, might exit
    printf("Gauss-Jordan complete.\n");
    print_matrix(Aug, n_row, n_row + 1, "Final Augmented [I|x]");
    for (k = 0; k < n_row; k++)  x[k] = Aug[k][n_row]; 


    if (solve_success) {
        print_vector(x, n_row, "Solution x"); 
    
        //  write solution to a file 
        FILE *out_fp = NULL;
        char output_filename[MAXSTR + 20]; // +20 for suffix and null terminator
    
        if (input_filename != NULL) { // again we check the file exists
            snprintf(output_filename, sizeof(output_filename), "%s_solution.txt", input_filename);
        } 
        else {
            snprintf(output_filename, sizeof(output_filename), "solution_output.txt");
        }
    
        printf("Attempting to write solution to: %s\n", output_filename);
        out_fp = fopen(output_filename, "w"); 
    
        if (out_fp == NULL) {
            fprintf(stderr, "Error: Could not open output file '%s' for writing solution.\n", output_filename);
        } 
        else {
            // since we didn't stop on input_filename error, we are defensively checking
            fprintf(out_fp, "# Solution vector x for input: %s\n", (input_filename ? input_filename : "N/A"));
            fprintf(out_fp, "# Number of elements (N_ROW): %d\n", n_row);
            for (k = 0; k < n_row; k++) {
                if (fprintf(out_fp, "%.8f\n", x[k]) < 0) {
                    fprintf(stderr, "Error writing element x[%d] to output file.\n", k);
                    break;
                }
            }
            fclose(out_fp); 
            printf("Solution successfully written to %s.\n", output_filename);
        }
        // end of writing solution to file 
    
        printf("Verifying solution (Calculating A * x)...\n");

        for (k = 0; k < n_row; k++) {
            check[k] = 0.0;
            for (j = 0; j < n_row; j++) {
                check[k] += A[k][j] * x[j]; 
            }
        }
        print_vector(check, n_row, "Calculated A*x");
        printf("Comparing A*x with original b:\n");
    
        int errors = 0;
        for (k = 0; k < n_row; k++) {
            if (fabs(check[k] - b[k]) > TOL) {
                printf("  Mismatch at index [%d]: A*x = %.7g, b = %.7g, Diff = %.7g\n",
                       k, check[k], b[k], fabs(check[k] - b[k]));
                errors++;
            }
        }
        if (errors == 0) {
            printf("  Verification successful (within TOL=%.1e).\n", TOL);
        } else {
            printf("  Verification FAILED (%d mismatches).\n", errors);
        }
    
    } else {
        printf("\nSkipping verification and solution output for this system due to solver incompatibility or failure.\n");
    }

    /* check if the file has been consumed completely. */
    if (!feof(fp)) {
        fprintf(stderr, "Warning: File processing stopped before reaching end-of-file. Check file format near last processed system.\n");
    }

    fclose(fp);
    printf("File processing complete for %s.\n", input_filename);

    //  free Memory 
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