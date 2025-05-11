#include <stdio.h>
#include <stdlib.h>


/* fgets */
/* we want to read the matrix file it requires:
   1. open the file,
   2. skip the header
   3. read the size
   4. skip another header
   5. read the matrix
   6. skip another header
   7. read the vector
   8. print the matrix and vector
   9. free the memory
   10. close the file
*/


static void print_matrix(float **mat, int n, int m){
    for (int i = 0; i < n; i++){
        for (int j = 0; j < m; j++){
            printf("%f ", mat[i][j]);
        }
        printf("\n");
    }
}
static void print_vector(float *vec, int n){
    for (int i = 0; i < n; i++){
        printf("%f ", vec[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]){

    FILE *fp;

    if (argc != 2){
        fprintf(stderr, "Usage: %s <matrix_data_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *input_filename = argv[1]; 

    /* open the file*/
    fp = fopen(input_filename, "r");

    /* skip the header */
    char buffer[256];
    fgets(buffer, sizeof(buffer), fp); // Skip first line
    fgets(buffer, sizeof(buffer), fp); // Skip second line

    printf("Skipped header lines: %s\n", buffer);

    /* read the matrix size */
    int n, m;
    fscanf(fp, "%d %d", &n, &m);

    /* skip more lines*/
    fgets(buffer, sizeof(buffer), fp); // tricky! Since fscanf does not consume the rest of the line
    fgets(buffer, sizeof(buffer), fp); // Skip header before A



    /* read the matrix */
    float **A = (float **)malloc((size_t)n * sizeof(float *));
    float *A_data = (float *)malloc((size_t)n * n * sizeof(float));
    for (int i = 0; i < n; i++){
        A[i] = A_data + (size_t) i * n; //link the data to the matrix
    }

    for (int i = 0; i <n; i++){
        for (int j = 0; j < n; j++){
            fscanf(fp, "%f", &A[i][j]);
        }
    }

    /* skip more lines*/
    fgets(buffer, sizeof(buffer), fp); // tricky! Since fscanf does not consume the rest of the line
    fgets(buffer, sizeof(buffer), fp); // Skip header before A



    /* read the vector */
    float *b = (float *)malloc(n * sizeof(float));
    for (int i = 0; i < n; i++){
        fscanf(fp, "%f", &b[i]);
    }

    /* print the matrix and vector */
    print_matrix(A, n, n);
    print_vector(b, n);

    /* free the memory */
    free(b);
    free(A_data);
    free(A);


    fclose(fp);

}
