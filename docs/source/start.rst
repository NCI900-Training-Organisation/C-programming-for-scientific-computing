Outline
=======

This workshop is part of NCI training provided for NCI users. As such, the contents are tailored to the NCI Gadi supercomputer and high-performance data environments.  

In this tutorial, we will look into some good practices in C programming for writing scientific software for community use.
To this end, we will use numerical linear algebra solver as a concrete example and guide you through the process of writing robust solver programs in C. More than just implementing algorithms, we'll concentrate on the software engineering principles crucial for developing reliable scientific software.

Given a linear system `Ax = b`, numerous methods can find the solution `x`. We will implement and provide:

1.  **Gauss-Jordan Elimination:** A basic and generic algorithm applicable to all matrices.
2.  **Cholesky Decomposition:** A highly efficient algorithm for symmetric positive definite matrices.

The emphasis of this workshop, however, is less on the intricate mathematical details of these algorithms and more on **how to build a well-structured codebase around them.** We aim to mirror the design of established scientific libraries by learning to:

*   Offer users choices of solution methods through configurable options (e.g., command-line flags).
*   Learn effective error handling strategies to make our C programs more resilient and easier to debug (though segmentation faults remain a C programmer's rite of passage!).
*   Organise larger projects using multiple source files for better modularity.
*   Automate compilation and linking with Makefiles for build.
*   Port existing, highly optimised numerical libraries. Specifically, we'll explore how to interface with LAPACK on Gadi, a foundational library that underpins much of the scientific software ecosystem.


The contents of this workshop are organised as follows:

.. list-table::
    :widths: 30 70
    :header-rows: 1

    * - Topic
      - Contents
    * - Topic 1
      - C Programming Refresher 
    * - Topic 2
      - LAPACK  
    * - Topic 3 
      - Compilation and Makefile 
    * - Topic 4
      - Performance Comparison



