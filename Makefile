# Compiler 
CC = gcc

#  Paths 
MKL_INCLUDE_PATH = /opt/intel/oneapi/mkl/latest/include
MKL_LIB_PATH = /opt/intel/oneapi/mkl/latest/lib

#  Flags 
# Standard compiler flags used by all compilations
CFLAGS = -Wall -Wextra -g 

MKLFLAGS   = -I. -I$(MKL_INCLUDE_PATH)

# dybnamic linking during runtime
LDFLAGS_MKL = -L$(MKL_LIB_PATH) -Wl,-rpath=$(MKL_LIB_PATH)
# Standard libraries (math library)
LDLIBS = -lm
MKL_LIBS = -lmkl_rt #lapack uses multithreaded MKL

#  Source Files 
# Main program sources
SRC_MAIN = linear-algebra-lapack-sln.c   
SRC_GJ = linear-algebra-GJ.c      
SRC_MULTI = linear-algebra-multisolver.c

# dependencies
SRC_UTIL = util.c             
SRC_PRIMITIVES = primitives.c                   

#  object files 
OBJS_MAIN = $(SRC_MAIN:.c=.o)
OBJS_GJ = $(SRC_GJ:.c=.o)
OBJS_MULTI = $(SRC_MULTI:.c=.o)


OBJS_UTIL = $(SRC_UTIL:.c=.o)
OBJS_PRIMITIVES = $(SRC_PRIMITIVES:.c=.o)

# group common objects for convenience
OBJS_COMMON = $(OBJS_UTIL) $(OBJS_PRIMITIVES)

#  Executable Names 
TARGET_MAIN = solver            
TARGET_GJ = solver_gj         
TARGET_MULTI = solver_multi      


#  Targets 

# default Target: Build all executables
all: $(TARGET_MAIN) $(TARGET_GJ) $(TARGET_MULTI) 

# rule to build the main multi-solver executable, including lapack
# TODO: ADD Rule to build the linear-algebra-lapack executable, linking with MKL


# rule to build the original GJ solver 
$(TARGET_GJ): $(OBJS_GJ) $(OBJS_COMMON)
	@echo "Linking $@..."
	$(CC) $(CFLAGS)  $^ -o $@ $(LDLIBS)
	@echo "Built $@ successfully."

# rule to build the multisolver
$(TARGET_MULTI): $(OBJS_MULTI) $(OBJS_COMMON)
	@echo "Linking $@..."
	$(CC) $(CFLAGS)  $^ -o $@ $(LDLIBS)
	@echo "Built $@ successfully."


# generic rule to compile all .c file into a .o file
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(MKLFLAGS) $(CFLAGS) -c $< -o $@


#  Cleanup 
clean:
	@echo "Cleaning up..."
	rm -f $(TARGET_MAIN) $(TARGET_GJ) $(TARGET_MULTI) \
	      $(OBJS_MAIN) $(OBJS_GJ) $(OBJS_MULTI)  \
	      $(OBJS_UTIL) $(OBJS_PRIMITIVES) \
