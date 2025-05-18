# --- Compiler ---
CC = gcc

# --- Paths ---
MKL_INCLUDE_PATH = /opt/intel/oneapi/mkl/latest/include
MKL_LIB_PATH = /opt/intel/oneapi/mkl/latest/lib

# --- Flags ---
# include dirctories for preprocessor (needed for MKL)
CPPFLAGS = -I. -I$(MKL_INCLUDE_PATH)
# Standard compiler flags used by all compilations
CFLAGS = -Wall -Wextra -g 

# dybnamic linking during runtime
LDFLAGS_MKL = -L$(MKL_LIB_PATH) -Wl,-rpath=$(MKL_LIB_PATH)
# Standard libraries (math library)
LDLIBS = -lm
MKL_LIBS = -lmkl_rt #lapack uses multithreaded MKL

# --- Source Files ---
# Main program sources
SRC_MAIN = linear-algebra-lapack.c   # The new multi-solver using MKL
SRC_GJ = linear-algebra-GJ.c      # Original GJ solver
SRC_BASE = linear-algebra-multisolver.c         # Original "base" solver


# Common utility/solver sources
SRC_NA_UTIL = util.c             # Utility functions (used by all)
SRC_NA_OLD = primitives.c                   # Old NA functions (used by original solvers)
#SRC_NA_NEW = na_solvers.c           # New NA functions (used by main multi-solver)

# --- Object Files ---
OBJS_MAIN = $(SRC_MAIN:.c=.o)
OBJS_GJ = $(SRC_GJ:.c=.o)
OBJS_BASE = $(SRC_BASE:.c=.o)


OBJS_NA_UTIL = $(SRC_NA_UTIL:.c=.o)
OBJS_NA_OLD = $(SRC_NA_OLD:.c=.o)
#OBJS_NA_NEW = $(SRC_NA_NEW:.c=.o)

# Group common objects for convenience
OBJS_COMMON_OLD = $(OBJS_NA_UTIL) $(OBJS_NA_OLD)
#OBJS_COMMON_NEW = $(OBJS_NA_UTIL) $(OBJS_NA_NEW)

# --- Executable Names ---
TARGET_MAIN = solver            # Main multi-solver executable
TARGET_GJ = solver_gj         # Original GJ executable
TARGET_BASE = solver_multisolver      # Original base executable


# --- Targets ---

# Default Target: Build all executables
all: $(TARGET_MAIN) $(TARGET_GJ) $(TARGET_BASE) 

# Rule to build the main multi-solver executable (with MKL)
$(TARGET_MAIN): $(OBJS_MAIN) $(OBJS_COMMON_OLD)
	@echo "Linking $@ (with MKL)..."
	$(CC) $(CFLAGS) $(LDFLAGS_MKL) $^ -o $@ $(MKL_LIBS) $(LDLIBS)
	@echo "Built $@ successfully."

# Rule to build the original GJ solver (NO MKL)
$(TARGET_GJ): $(OBJS_GJ) $(OBJS_COMMON_OLD)
	@echo "Linking $@..."
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	@echo "Built $@ successfully."

# Rule to build the original base solver (NO MKL)
$(TARGET_BASE): $(OBJS_BASE) $(OBJS_COMMON_OLD)
	@echo "Linking $@..."
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	@echo "Built $@ successfully."


# Generic rule to compile any .c file into a .o file
# Uses CPPFLAGS (for includes) and CFLAGS (for compilation options)
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# --- Add explicit header dependencies if needed ---
# Example: If na_solvers.c includes na_util.h, etc.
# $(OBJS_NA_NEW): na_util.h na_solvers.h
# $(OBJS_NA_OLD): na_util.h na.h
# $(OBJS_NA_UTIL): na_util.h

# --- Cleanup ---
clean:
	@echo "Cleaning up..."
	rm -f $(TARGET_MAIN) $(TARGET_GJ) $(TARGET_BASE) \
	      $(OBJS_MAIN) $(OBJS_GJ) $(OBJS_BASE)  \
	      $(OBJS_NA_UTIL) $(OBJS_NA_OLD) \
