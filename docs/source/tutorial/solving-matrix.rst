Performance Comparison
=========================

The executable **``linear-algebra-lapack``** contains our solver, which provides three solution methods accessible via the following flags:
- -g: Gauss-Jordan Elimination
- -c: Our primitive Cholesky solver
- -cl: LAPACK Cholesky solver.

We aim to demonstrate that existing libraries often provide better performance than custom implementations.
As expected, the well-optimized LAPACK library offers a much faster Cholesky method.
To this end, we will solve a relatively large matrix system using the ``-c`` and ``-cl`` flags.
The matrix used for this comparison is a 4515 x 4515 sparse, square matrix: `Boeing/msc04515 <https://www.cise.ufl.edu/research/sparse/matrices//Boeing/msc04515.html>`_ .
The matrix file, located at `/scratch/vp91/msc04515.dat`, has been sanitized from the original data to ensure it has the correct header for our solvers.

Exercise
-----------

1. Run the solver with the following two options. What metric(s) can you use to benchmark their performance?

.. code-block:: bash

    ./linear-algebra-lapack -c /scratch/vp91/msc04515.dat
    ./linear-algebra-lapack -cl /scratch/vp91/msc04515.dat

.. note::
    Note that in practice, one might prefer using iterative methods to solve sparse systems such as msc04515.