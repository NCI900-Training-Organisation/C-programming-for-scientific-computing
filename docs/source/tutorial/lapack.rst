LAPACK
======

Our program currently relies on custom matrix-solver primitives, but in practice you should always first seek existing, highly optimized libraries rather than reinvent the wheel. The **Linear Algebra PACKage (LAPACK)** is the standard for matrix problems: it offers a wide range of efficient, well-implemented algorithms for solving matrix equations and is used extensively in scientific software.

If you need fast, reliable linear-algebra routines, LAPACK should be your first choice. To demonstrate how to integrate it, we’ll add a new command-line flag (for example, ``--lapack-cholesky``) that tells the program to use LAPACK’s Cholesky decomposition routine.

We use two LAPACK functions: **``LAPACKE_dpotrf``** and **``LAPACKE_dpotrs``**. Their documentation can be found on the official `LAPACK website <https://www.netlib.org/lapack/explore-html/d1/dd3/group__potrf2_ga7a1158271be5fac6e3d89b7ca8d71a07.html#ga7a1158271be5fac6e3d89b7ca8d71a07>`_ and on `Intel oneAPI <https://www.intel.com/content/www/us/en/docs/onemkl/developer-reference-c/2025-1/potrf2.html>`_.

**E.g.:**

.. code-block:: c

   int info = LAPACKE_dpotrf(LAPACK_ROW_MAJOR, 'U', n_row, A_lapack_1d, n_row);

-  The first argument specifies whether matrix storage layout is row-major (``LAPACK_ROW_MAJOR``) or column-major (``LAPACK_COL_MAJOR``).
-  ``'U'`` indicates the upper triangle of ``A`` is stored.
-  ``n_row`` is the order (number of rows) of the matrix ``A``.
-  ``A_lapack_1d`` is a pointer to the first element of the 1D array storing the matrix; **this array will be overwritten**.
-  On exit, the upper triangular factor ``U`` overwrites the upper triangle of ``A_lapack_1d``, and the routine computes ``A = U^T * U``.
-  Return value: if ``info == 0``, the factorization succeeded; if ``info > 0``, the leading minor of order ``info`` is not positive definite.

Exercises
---------

1.  In **``linear-algebra-lapack.c``**, after performing Cholesky factorisation with **``LAPACKE_dpotrf``**, call **``LAPACKE_dpotrs``** to solve the system. Fill in all required arguments: matrix layout, triangle indicator, dimensions, pointers to ``U`` and the right-hand side. See the manual on `potrs <https://www.intel.com/content/www/us/en/docs/onemkl/developer-reference-c/2025-1/potrs.html>`_.


On Gadi, LAPACK is shipped with Intel MKL. To load the library modules, run::

   module load intel-compiler-llvm/2025.0.4
   module load intel-mkl/2025.0.1
