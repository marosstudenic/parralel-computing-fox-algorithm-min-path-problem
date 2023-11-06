// basic mpi program

#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdarg.h>

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

void custom_print(int rank, const char *format, ...);
void print_matrix(int rank, int matrix_size, int matrix[matrix_size][matrix_size], int is_custom_print);
void print_matrix_custom(int rank, int matrix_size, int matrix[matrix_size][matrix_size]);
void print_matrix_stdout(int rank, int matrix_size, int matrix[matrix_size][matrix_size]);

int main(int argc, char *argv[])
{
    int my_rank, size;
    int m;           // sqrt of number of processes
    int matrix_size; // size of matrix

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // calculate m
    m = (int)sqrt(size);

    // check number of processes
    if (m * m != size)
    {
        printf("Number of processes must be a square number!\n");
        MPI_Finalize();
        return 1;
    }

    if (my_rank == 0)
    {
        scanf("%d", &matrix_size);
    }

    // broadcast matrix size
    MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // check if matrix size is divisible by m
    if (matrix_size % m != 0)
    {
        printf("Matrix size must be divisible by sqrt(number of processes)!, %d\n", my_rank);
        MPI_Finalize();
        return 1;
    }

    // create grid communicator
    MPI_Comm grid_comm;
    int dims[2] = {m, m};
    // we want periodic boundaries of columns
    int periods[2] = {0, 1};
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &grid_comm);

    // create row and column communicators
    MPI_Comm row_comm;
    MPI_Comm col_comm;
    MPI_Comm_split(grid_comm, my_rank / m, my_rank % m, &row_comm);
    MPI_Comm_split(grid_comm, my_rank % m, my_rank / m, &col_comm);

    // create matrix
    int matrix[matrix_size][matrix_size];
    int matrix_part[matrix_size / m][matrix_size / m];

    // read matrix from input
    if (my_rank == 0)
    {
        for (int i = 0; i < matrix_size; i++)
        {
            for (int j = 0; j < matrix_size; j++)
            {
                scanf("%d", &matrix[i][j]);
            }
        }

        print_matrix(my_rank, matrix_size, matrix, 1);
        print_matrix(my_rank, matrix_size, matrix, 0);
    }

    MPI_Finalize();
}

// print function which prints output to file based on rank
void custom_print(int rank, const char *format, ...)
{
    va_list args;
    char filename[256];

    // Create filename based on process rank
    snprintf(filename, sizeof(filename), "output/proc-%d.out", rank);

    // Open the file
    FILE *file = fopen(filename, "a");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Use the variable argument list feature of stdarg.h
    va_start(args, format);
    // Print to the file using vfprintf which takes a va_list
    vfprintf(file, format, args);
    va_end(args);

    // Close the file
    fclose(file);
}

void print_matrix(int rank, int matrix_size, int matrix[matrix_size][matrix_size], int is_custom_print)
{
    if (is_custom_print)
    {
        print_matrix_custom(rank, matrix_size, matrix);
    }
    else
    {
        print_matrix_stdout(rank, matrix_size, matrix);
    }
}

void print_matrix_custom(int rank, int matrix_size, int matrix[matrix_size][matrix_size])
{
    for (int i = 0; i < matrix_size; i++)
    {
        for (int j = 0; j < matrix_size; j++)
        {
            custom_print(rank, "%d ", matrix[i][j]);
        }
        custom_print(rank, "\n");
    }
}

void print_matrix_stdout(int rank, int matrix_size, int matrix[matrix_size][matrix_size])
{
    for (int i = 0; i < matrix_size; i++)
    {
        for (int j = 0; j < matrix_size; j++)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}