# C-programming-sc

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

To provide utilities based on user input, you can parse command-line arguments. The strcmp function (from <string.h>) is useful for comparing strings:

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

