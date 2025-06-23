Compilation and Makefile
========================

As our projects grow to include multiple source files, header files, and external libraries like MKL LAPACKE, manually compiling is no longer a one-liner (maybe a very long one-line) and linking becomes tedious and error-prone. Fortunately, automation tools streamline this process.

For C programming, the traditional and highly effective tool is **``make``** (using a **Makefile**). While ``make`` is versatile enough for many automation tasks beyond C, its primary use in C/C++ is managing the build process. For very large or cross-platform projects, tools like **CMake** offer even more advanced capabilities.

Before jumping into Makefile syntax, let’s review the C build pipeline. Converting your ``.c`` sources into an executable involves four main stages, in chronological order:

1.  **Preprocessing (``.c`` → ``.i``)**

    The first step involves the C preprocessor. It reads your source code and:

    * Processes directives starting with ``#``, such as expanding ``#define`` macros.
    * Replaces ``#include`` directives with the content of the specified header files.

    The result is an expanded C source file (often with a ``.i`` suffix).

    **E.g.:**

    ::

       gcc -E linear-algebra-GJ-filescope.c -o linear-algebra-GJ-filescope.i

2.  **Compilation (``.i`` → ``.s``)**

    The compiler takes the preprocessed code and translates it into assembly language, an intermediate representation of the code. This produces an assembly file (often ``.s``).

    **E.g.:**

    ::

       gcc -S linear-algebra-GJ-filescope.i -o linear-algebra-GJ-filescope.s

3.  **Assembly (``.s`` → ``.o``)**

    The assembler converts the assembly code into actual machine code (binary). This output is an **object file** (typically ``.o``). An object file contains the compiled code for that specific source file but usually cannot be run on its own because it might reference functions or data defined in other files or libraries.

    **E.g.:**

    ::

       gcc -c linear-algebra-GJ-filescope.s -o linear-algebra-GJ-filescope.o

4.  **Linking (``.o`` files + libraries → executable)**

    Finally, the linker's role is to take all the object files generated from your project's source code, along with any necessary libraries (like the math library ``-lm`` or MKL Lapack), and combine them into a single executable file.

    **E.g.:**

    ::

       gcc linear-algebra-GJ-filescope.o -o linear-algebra-GJ-filescope -lm


Exercises
---------

1.  Can you repeat the four steps to generate an executable file for **``linear-algebra-GJ.c``**?

2.  Compiling **``linear-algebra-lapack.c``** is likely to be more tricky given the external library linking. Try it yourself.

If you agree with the above exercise statement, you might need a **Makefile** to help you. Another significant advantage is that the make system uses the date and time stamps of files to determine when files are out of date (changed), so when the program is recompiled, it only recompiles the changed source codes.

In a Makefile, we define targets, dependencies, and commands to build our project. The basic structure of a Makefile includes:

-  **File name**: usually ``Makefile``.
-  **Special variables**: Makefile has special variables like ``CC`` (compiler), ``CFLAGS`` (compiler flags), and ``LDFLAGS`` (linker flags) that can be used to customize the build process.
-  **Targets**: the name/label of the file you want to build. This allows us to build part of the project independently.
-  **Automatic variables**: ``$@`` expands to the current target name, ``$^`` expands to all prerequisites.

**E.g.:**

::

   gcc -Wall -Wextra -g \
       -I. \
       -I/opt/intel/oneapi/mkl/latest/include \
       -L/opt/intel/oneapi/mkl/latest/lib \
       -Wl,-rpath=/opt/intel/oneapi/mkl/latest/lib \
       linear-algebra-lapack-sln.c util.c primitives.c \
       -o solver -lmkl_rt -lm

Makefile Example
----------------

.. code-block:: make

   # Compiler and flags
   CC       = gcc
   CFLAGS   = -Wall -Wextra -g \
              -I. \
              -I/opt/intel/oneapi/mkl/latest/include

   # Intel MKL lib path
   LDFLAGS  = -L/opt/intel/oneapi/mkl/latest/lib \
              -Wl,-rpath=/opt/intel/oneapi/mkl/latest/lib

   # Linking libraries
   LDLIBS   = -lmkl_rt -lm

   # Source files
   SRCS     = linear-algebra-lapack.c util.c primitives.c

   # Auto-generated object files
   OBJS     = $(SRCS:.c=.o)
   TARGET   = solver

   # Default target: build all
   all: $(TARGET)

   # Link step
   $(TARGET): $(OBJS)
       $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

   # Compilation step: build each .o from its corresponding .c
   # $< is the first dependency (the .c file)
   # $@ is the target (the .o file)
   %.o: %.c
       $(CC) $(CFLAGS) -c $< -o $@

   clean:
       rm -f $(OBJS) $(TARGET)

Exercises
---------

1.  Go to **``Makefile``**, and add a new target called ``solver`` for ``linear-algebra-lapack.c`` with its dependencies and rules.
