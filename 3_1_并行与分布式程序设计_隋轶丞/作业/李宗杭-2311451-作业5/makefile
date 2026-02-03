EXECS = mpi_homework
MPICC ?= mpicc
CXX = mpicxx
CFLAGS = -O3 -Wall
LDFLAGS = -lm

all: $(EXECS)

$(EXECS): main.cpp integral_mpi.cpp sort_mpi.cpp matrix_multiply.cpp matrix_multiply_mpi.cpp utils.cpp
		$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
		rm -f $(EXECS) *.o