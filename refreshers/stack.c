/*
+------------------+  ← Low memory
| .text (code)      |
| .data (initialized static/global vars) |
| .bss (uninitialized vars, zero-filled) |
| Heap (malloc/free) |
| ...               |
| Stack (local vars, function calls) |
+------------------+  ← High memory
*/


#include <stdio.h>

int x = 100;
void main()
{
    printf("1 ---> x = %d \n", x);

    int x = 10;
    
    printf("2 ---> x = %d \n", x);
    
    {
        printf("3 ---> x = %d \n", x);

        int x = 200;

        printf("3 ---> x = %d \n", x);
    }

    printf("5 ---> x = %d \n", x);
}