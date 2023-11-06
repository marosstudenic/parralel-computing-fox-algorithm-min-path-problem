// basic mpi program

#include <stdio.h>
#include <mpi.h>

// TODO:
// check number of processes
// create grid communicator
// create row and column communicators
// 1. read size of matrix from command line
// 2. read matrix from file
// 3. send matrix to other processes
// 4. compute matrix minplus product
// repead until distance_traveled > size of matrix
// 5. print result matrix

int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
}
