# Introduction - Program of the Day

This workshop guides you through the process of writing robust linear algebra solver programs in C. More than just implementing algorithms, we'll concentrate on the software engineering principles crucial for developing reliable scientific software.

Given a linear system `Ax = b`, numerous methods can find the solution `x`. We will implement and provide:

1.  **Gauss-Jordan Elimination:** A basic and generic algorithm applicable to all matrices.
2.  **Cholesky Decomposition:** A highly efficient algorithm for symmetric positive definite matrices.

The emphasis of this workshop, however, is less on the intricate mathematical details of these algorithms and more on **how to build a well-structured codebase around them.** We aim to mirror the design of established scientific libraries by learning to:

*   Offer users choices of solution methods through configurable options (e.g., command-line flags).
*   Learn effective error handling strategies to make our C programs more resilient and easier to debug (though segmentation faults remain a C programmer's rite of passage!).
*   Organise larger projects using multiple source files for better modularity.
*   Autemate compilation and linking with Makefiles for build.
*   Port existing, highly optimised numerical libraries. Specifically, we'll explore how to interface with LAPACK on Gadi, a foundational library that underpins much of the scientific software ecosystem.

We'll kick off with a focused C programming refresher, covering concepts that will be central to our exercises.

# Refresher

## Pointers

A pointer is a variable whose value is a memory address.

![Memory layout showing a variable x and a pointer ptr pointing to x](fig/pointers.png)

Let `x` be an integer variable defined and initialized to the value 10.

```c
int x = 10;
```

An integer take 4 bytes (32 bits) of memory in a 32-bit or 64-bit system. If we assume the first byte is stored in memory address 0X00 (0th bit) then the last byte will be stored in memory address 0X03 (23rd bit). An integer pointer can store the address of an integer variable.

```c
int *ptr = &x;
```

The result of the unary & operator is a pointer to its operand. The operand shall be an lvalue. -- C++03 standard.

```c 
int i =5;
int *p;

p = &5; // Error, 5 is not an lvalue
p = &i; // OK
```

As the address of the variable **x** starts with address 0X00, the pointer variable **ptr** will have the value 0X00. 

**Pointer Arithmetic**

![memory](fig/pointers_arith_int.png)

![memory](fig/pointers_arith_char.png)

## Memory Allocation

| Segment       | Description                                                                        |
|---------------|------------------------------------------------------------------------------------|
| Text segment  | Compiled program code (executable instructions)                                                                      |
| Data segment  | Global variables, static variables.                                                |
| Stack         | Stores local variables, function arguments, and return addresses. Memory is managed automatically by the compiler.                                          |
| Heap          | Used for dynamically allocated memory. Managed by the programmer using library functions (e.g., malloc, free in C; new, delete in C++).


![memory](fig/malloc.png)


![memory](fig/dynamic_mem.png)

If memory allocation is successful, **malloc** returns a void pointer (void*) to the beginning of the allocated memory block. This pointer can then be cast to the desired type. If memory allocation fails (e.g., not enough memory available), **malloc** returns a NULL pointer.

> **Important:** Always check the return value of **malloc** to ensure allocation was successful before using the pointer. Remember to free memory allocated with **malloc** when it's no longer needed to prevent memory leaks.


> **E.g.**
```c
/* Allocate memory for a vector of double-precision numbers */
    float *b = (float *)malloc(n * sizeof(float)); 
    // however, the vector has yet been initialised.
```

> **E.g.**
```c
    /* allocate memory for  the matrix */
    float **A = (float **)malloc((size_t)n * sizeof(float *)); 
    float *A_data = (float *)malloc((size_t)n * n * sizeof(float));
    for (int i = 0; i < n; i++){
        A[i] = A_data + (size_t) i * n; //link the data to the matrix
    }
```

The **A** and **A_data** layout can be visualised as follows:
```text
A  --> [ptr_0] ----> A_data --> [val_00] [val_01] ... [val_0(n-1)]  <-- Row 0
       [ptr_1] ----,            [val_10] [val_11] ... [val_1(n-1)]  <-- Row 1
       [ptr_2] ----|-,          [val_20] [val_21] ... [val_2(n-1)]  <-- Row 2
       ...         | `->        ...
       [ptr_n-1] --`----->      [val_(n-1)0] ... [val_(n-1)(n-1)] <-- Row n-1
```



## Command-Line Arguments

When a program is executed from the command line, the command interpreter (e.g., `sh`/`bash` in Unix/Linux) parses the command line and passes information to the `main` function as:

- **`argc`** (argument count): An integer representing the number of command-line arguments.  
- **`argv`** (argument vector): An array of C-style strings (`char* argv[]` or `char** argv`), where each string is one of the arguments.

> **E.g.**

```bash
./myprogram arg1 "another arg"
```
In the above commannd line, we have the following layout:

```text
argv:
+-----------+     +---------------------------------------+
| argv[0]   |---->| '.', '/', 'm', 'y', ..., 'm', '\0'    |  (Program name string)
+-----------+     +---------------------------------------+
| argv[1]   |---->| 'a', 'r', 'g', '1', '\0'              |  (First argument string)
+-----------+     +---------------------------------------+
| argv[2]   |---->| 'a', 'n', 'o', 't', ..., 'g', '\0'    |  (Second argument string)
+-----------+     +---------------------------------------+
| argv[3]   |---->  NULL  (This is the NULL pointer terminating the `argv` array itself)
+-----------+
```

To provide utilities based on user input, we can parse command-line arguments. The **`strcmp`** function (from <string.h>) is useful for comparing strings:

```c
int strcmp ( const char * str1, const char * str2 );
```

It returns 0 if the contents of both string matches.

> **E.g.**
```c
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc > 1) { 
        if (strcmp(argv[1], "--myflag") == 0) {
            printf("myflag was detected!\n");
        } else if (strcmp(argv[1], "--help") == 0) {
            printf("Usage: %s [--myflag] [--help] ...\n", argv[0]);
        } else {
            printf("Unknown argument: %s\n", argv[1]);
        }
    } else {
        printf("No arguments provided.\n");
    }
    return 0;
}
```



## File process

The following C standard library functions (from <stdio.h>) are commonly used for file input/output:

```c
FILE *fopen(const char *filename, const char *mode);
```
- Opens the file specified by `filename` in the mode specified by `mode` (e.g., `"r"` for read, `"w"` for write, `"a"` for append).  
- Returns a `FILE *` if successful, or `NULL` if an error occurs.  

> **E.g.**
```c
FILE *fp = fopen("input.txt", "r");
if (fp == NULL) {
    fprintf(stderr, "Error opening file");
}
```

```c
char *fgets(char *str, int n, FILE *stream);
```

- Reads a line from the specified stream and stores it into the string buffer.
- It stops when (n-1) characters are read, the newline character is read, or the end-of-file is reached, whichever comes first.
- It can be used to consume the file headers.


> **E.g.**
```c
char buffer[256];
if (fgets(buffer, MAXSTR, fp) != NULL )
{
 printf("Read line: %s", buffer);   
};
```



```c
int fscanf(FILE *stream, const char *format, ... );
```

- Reads formatted input from a stream.
- Works like **`scanf`**, but reads from a file stream instead of **`stdin`**.
- Returns the number of input items successfully matched and assigned.


> **E.g.**
```c
#include <stdio.h>
#include <stdlib.h> 



int n = 10; 
float *b = (float *)malloc(n * sizeof(float));
if (b == NULL) {
    perror("Failed to allocate memory");
    // Handle error
} 
else {
    for (int i = 0; i < n; i++) {
        if (fscanf(fp, "%f", &b[i]) != 1) { // read the floating number from file fp into the array b
            fprintf(stderr, "Error reading float at index %d\n", i);
            break; 
        }
    }
    free(b); 
}
```

```c 
int fprintf(FILE *stream, const char *format, ...);
```

- stream: A pointer to the FILE object that identifies the stream where the output is to be written. 

- format: A string that specifies the format of the output. It may contain format specifiers that are replaced by the values specified in the subsequent arguments

- The **`fprintf()`** function returns the number of characters written if successful, and a negative value if an error occurs. 

> **E.g.**
```c
if (fprintf(out_fp, "%.8f\n", x[k]) < 0) {
    fprintf(stderr, "Error writing element x[%d] to output file.\n", k);
    break;
    }
```

```c
int fclose(FILE *stream);
```

- Closes the stream.
- Returns 0 if the stream is successfully closed.


> **E.g.**
```c
if (fp != NULL) {
    fclose(fp);
}
```

You can read through the file **`read_mat_file`** to see how those functions are used to read a matrix input file.

```bash
$ gcc read_mat_file.c -o read_mat_file
$ ./read_mat_file trefethen_dense.dat
```



## Exercises

1. 
   In `linear-algebra-GJ-filescope.c`, review the `gauss_jordan_partial` function and its invocation in `main`. Add any missing validation or error checks (e.g., null pointers, invalid return values etc) to make the program more resilient.

2.  
   Throughout `linear-algebra-GJ-filescope.c`, you’ll find TODO comments for error handling. Replace these placeholders with sensible logic: print informative messages, and exit if you decide so.

3.  
   The file `linear-algebra-GJ-filescope.c` may seem a little unwieldy. Refactor it by splitting related functionality into separate source (`.c`) and header (`.h`) files — e.g., one for matrix operations, one for utilities — so that each component is easier to maintain.

4.   
   In `linear-algebra-multisolvers.c`, add a new command-line option (for example, `--cholesky`) that lets users explicitly select the Cholesky solver when working with symmetric matrices. Update `main` and your argument-parsing logic accordingly.


# LAPACK
Our program currently relies on custom matrix-solver primitives, but in practice you should always first seek existing, highly optimized libraries rather than reinvent the wheel. The **Linear Algebra PACKage (LAPACK)** is the standard for problems: it offers a wide range of efficient, well implemented algorithms for solving matrix equations and is used extensively in scientific software.

If you need fast, reliable linear-algebra routines, LAPACK should be your first choice. 
To demonstrate how to integrate it, we’ll add a new command-line flag (for example, `--lapack-cholesky`) that tells the program to use LAPACK’s Cholesky decomposition routine.

We use two LAPACK functions: **`LAPACKE_dpotrf`** and **`LAPACKE_dpotrs`**. Their documentation can be found on the official [LAPACK website](https://www.netlib.org/lapack/explore-html/d1/dd3/group__potrf2_ga7a1158271be5fac6e3d89b7ca8d71a07.html#ga7a1158271be5fac6e3d89b7ca8d71a07) and [Intel oneAPI]( https://www.intel.com/content/www/us/en/docs/onemkl/developer-reference-c/2025-1/potrf2.html).

> **E.g.**
```c
int info = LAPACKE_dpotrf(LAPACK_ROW_MAJOR, 'U', n_row, A_lapack_1d, n_row);
```

-  Specifies whether matrix storage layout is row major (LAPACK_ROW_MAJOR) or column major (LAPACK_COL_MAJOR).

- 'U': Upper triangle of A is stored;

- The order of the matrix A.

- A pointer to the first element of the 1D array storing the matrix `A`. **This array will be overwritten**. 

- The **upper triangular part** of `A` is supplied. The function computes `A = U^T * U`, and the upper triangular factor `U` overwrites the upper triangle of `A_lapack_1d`.

- Return value: On success, info == 0; if info > 0, the leading minor of order info is not positive definite.


## Exercises
1. 
  In **`linear-algebra-lapack.c`**, after performing Cholesky factorisation with **`LAPACKE_dpotrf`**, call **`LAPACKE_dpotrs`** to solve the system. Fill in all required arguments—matrix layout, triangle indicator, dimensions, pointers to U and the right-hand side. See the manual [here]( https://www.intel.com/content/www/us/en/docs/onemkl/developer-reference-c/2025-1/potrs.html).



On Gadi, LAPACK is shipped with intel-mkl.
To use the library, 
```bash
module load intel-compiler-llvm/2025.0.4
module load intel-mkl/2025.0.1
```