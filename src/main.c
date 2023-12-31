// basic mpi program

#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#define CUSTOM_PRINT 1
#define STDOUT_PRINT 0
#define MY_INFINITY INT_MAX
#define DIM 2
#define VERBOSE 0
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
void print_matrix(int rank, int matrix_size, int *matrix, int is_custom_print);
void print_matrix_custom(int rank, int matrix_size, int *matrix);
void print_matrix_stdout(int rank, int matrix_size, int *matrix);
void row_broadcast(int blck_size, int *matrixA, int *new_matrix, int step, int my_row, int my_rank, int m, MPI_Comm row_comm);
void compute_min_plus(int blck_size, int *matrixA, int *matrixB, int *solution_matrix);
void circular_column_shift(int blck_size, int *matrix, int my_row, int steps, int m, MPI_Comm col_comm);
void reorder_matrix(int *solution_matrix, int *solution_matrix_wrong_order, int m, int blck_size, int matrix_size);
void read_matrix(int matrix_size, int blck_size, int m, int *reoordered_matrix);

int inf_sum(int a, int b)
{
    if (a == MY_INFINITY || b == MY_INFINITY)
    {
        return MY_INFINITY;
    }

    if (MY_INFINITY - b < a)
        return MY_INFINITY;

    return a + b;
}
int min(int a, int b)
{
    if (a < b)
    {
        return a;
    }

    return b;
}

int main(int argc, char *argv[])
{
    int my_rank, nproc, old_rank;
    int m;           // sqrt of number of processes
    int matrix_size; // size of matrix

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &old_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if (VERBOSE)
    {
        printf("starting %d\n", old_rank);
    }

    // calculate m
    m = (int)sqrt(nproc);

    if (old_rank == 0)
    {
        // check number of processes
        if (m * m != nproc)
        {
            printf("Number of processes must be a square number!\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
    }

    if (old_rank == 0)
    {
        scanf("%d", &matrix_size);
    }

    // broadcast matrix size
    MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // check if matrix size is divisible by m
    if (matrix_size % m != 0)
    {
        printf("Matrix size must be divisible by sqrt(number of processes)!\n");
        MPI_Finalize();
        return 1;
    }

    // create grid communicator
    MPI_Comm grid_comm;
    MPI_Comm row_comm;
    MPI_Comm col_comm;

    int dims[2] = {m, m};
    // we want periodic boundaries of columns
    int periods[2] = {1, 1};
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &grid_comm);
    MPI_Comm_rank(grid_comm, &my_rank);

    // save coords
    int grid_coords[2];
    MPI_Cart_coords(grid_comm, my_rank, DIM, grid_coords);
    int my_row = grid_coords[0];
    int my_col = grid_coords[1];
    int row_communicator[2] = {0, 1};
    MPI_Cart_sub(grid_comm, row_communicator, &row_comm);

    int col_communicator[2] = {1, 0};
    MPI_Cart_sub(grid_comm, col_communicator,
                 &col_comm);

    int blck_size = matrix_size / m;
    int *matrix_part = (int *)malloc(blck_size * blck_size * sizeof(int));
    int *reoordered_matrix = (int *)malloc(matrix_size * matrix_size * sizeof(int));

    if (matrix_part == NULL)
    {
        printf("malloc error in proc: %d", my_rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (reoordered_matrix == NULL)
    {
        printf("malloc error in proc: %d", my_rank);
        free(matrix_part);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // reorder matrix
    if (old_rank == 0)
    {
        read_matrix(matrix_size, blck_size, m, reoordered_matrix);
    }

    // scatter matrix to processes
    MPI_Scatter(reoordered_matrix, matrix_size * matrix_size / nproc, MPI_INT, matrix_part, matrix_size * matrix_size / nproc, MPI_INT, 0, grid_comm);
    free(reoordered_matrix);

    int *matrixA = (int *)malloc(blck_size * blck_size * sizeof(int));
    int *matrixB = (int *)malloc(blck_size * blck_size * sizeof(int));
    int *matrix_partial_solution = (int *)malloc(blck_size * blck_size * sizeof(int));
    int *temp_matrix = (int *)malloc(blck_size * blck_size * sizeof(int));

    if (matrixA == NULL)
    {
        printf("malloc error in proc: %d", my_rank);
        free(matrix_part);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (matrixB == NULL)
    {
        printf("malloc error in proc: %d", my_rank);
        free(matrixA);
        free(matrix_part);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (matrix_partial_solution == NULL)
    {
        printf("malloc error in proc: %d", my_rank);
        free(matrixB);
        free(matrix_part);
        free(matrixA);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (temp_matrix == NULL)
    {
        printf("malloc error in proc: %d", my_rank);
        free(matrix_partial_solution);
        free(matrixB);
        free(matrix_part);
        free(matrixA);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (int row_i = 0; row_i < blck_size; row_i++)
    {
        for (int col_i = 0; col_i < blck_size; col_i++)
        {
            matrixA[row_i * blck_size + col_i] = matrix_part[row_i * blck_size + col_i];
            matrixB[row_i * blck_size + col_i] = matrix_part[row_i * blck_size + col_i];
            matrix_partial_solution[row_i * blck_size + col_i] = MY_INFINITY;
        }
    }

    free(matrix_part);

    int traveled_distance = 0;

    if (my_rank == 0 && VERBOSE)
    {
        printf("starting to compute\n");
    }

    while (traveled_distance < matrix_size)
    {
        if (my_rank == 0 && VERBOSE)
        {
            printf("traveled distance: %d from %d\n", traveled_distance, matrix_size);
        }

        for (int step = 0; step < m; step++)
        {
            row_broadcast(blck_size, matrixA, temp_matrix, step, my_row, my_rank, m, row_comm);
            compute_min_plus(blck_size, temp_matrix, matrixB, matrix_partial_solution);
            circular_column_shift(blck_size, matrixB, my_row, step, m, col_comm);
        }

        for (int row = 0; row < blck_size; row++)
        {
            for (int col = 0; col < blck_size; col++)
            {
                matrixA[row * blck_size + col] = matrix_partial_solution[row * blck_size + col];
                matrixB[row * blck_size + col] = matrix_partial_solution[row * blck_size + col];
                matrix_partial_solution[row * blck_size + col] = MY_INFINITY;
            }
        }

        if (traveled_distance == 0)
        {
            traveled_distance++;
        }
        else
        {
            traveled_distance *= 2;
        }
    }

    // solution is now in matrixA because we copied it
    for (int row = 0; row < blck_size; row++)
    {
        for (int col = 0; col < blck_size; col++)
        {
            if (matrixA[row * blck_size + col] == MY_INFINITY)
            {
                matrixA[row * blck_size + col] = 0;
            }
        }
    }

    // we need to replace all infinity with 0
    // we need to reorder it and gather in rank 0 process
    int *solution_matrix_wrong_order = (int *)malloc(matrix_size * matrix_size * sizeof(int));

    if (temp_matrix == NULL)
    {
        printf("malloc error in proc: %d", my_rank);
        free(matrix_partial_solution);
        free(matrixB);
        free(temp_matrix);
        free(matrixA);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Gather(matrixA, blck_size * blck_size, MPI_INT, solution_matrix_wrong_order, blck_size * blck_size, MPI_INT, 0, grid_comm);

    free(matrixA);
    free(matrixB);
    free(matrix_partial_solution);
    free(temp_matrix);

    if (my_rank == 0)
    {
        int *solution_matrix = (int *)malloc(matrix_size * matrix_size * sizeof(int));
        reorder_matrix(solution_matrix, solution_matrix_wrong_order, m, blck_size, matrix_size);
        print_matrix(my_rank, matrix_size, solution_matrix, STDOUT_PRINT);
        free(solution_matrix);
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

void print_matrix(int rank, int matrix_size, int *matrix, int is_custom_print)
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

void print_matrix_custom(int rank, int matrix_size, int *matrix)
{
    for (int i = 0; i < matrix_size; i++)
    {
        for (int j = 0; j < matrix_size; j++)
        {
            custom_print(rank, "%d", matrix[i * matrix_size + j]);
            if (j != matrix_size - 1)
            {
                custom_print(rank, " ");
            }
        }
        custom_print(rank, "\n");
    }
}

void print_matrix_stdout(int rank, int matrix_size, int *matrix)
{
    for (int i = 0; i < matrix_size; i++)
    {
        for (int j = 0; j < matrix_size; j++)
        {
            printf("%d", matrix[i * matrix_size + j]);
            if (j != matrix_size - 1)
            {
                printf(" ");
            }
        }
        printf("\n");
    }
}

void circular_column_shift(int blck_size, int *matrix, int my_row, int steps, int m, MPI_Comm col_comm)
{
    int source = (my_row + 1 + m) % m;
    int dest = (my_row - 1 + m) % m;

    MPI_Sendrecv_replace(matrix, blck_size * blck_size, MPI_INT, dest, 1, source, 1, col_comm, MPI_STATUS_IGNORE);
}

void row_broadcast(int blck_size, int *matrixA, int *new_matrix, int step, int my_row, int my_rank, int m, MPI_Comm row_comm)
{
    if (my_rank == my_row * m + (my_row + step) % m)
    {
        memcpy(new_matrix, matrixA, blck_size * blck_size * sizeof(int));
    }

    int root = (my_row + step % m) % m;
    MPI_Bcast(new_matrix, blck_size * blck_size, MPI_INT, root, row_comm);
}

void compute_min_plus(int blck_size, int *matrixA, int *matrixB, int *solution_matrix)
{
    for (int row_i = 0; row_i < blck_size; row_i++)
    {
        for (int col_i = 0; col_i < blck_size; col_i++)
        {
            for (int index = 0; index < blck_size; index++)
            {
                solution_matrix[row_i * blck_size + col_i] = min(solution_matrix[row_i * blck_size + col_i], inf_sum(matrixA[row_i * blck_size + index], matrixB[index * blck_size + col_i]));
            }
        }
    }
}

void reorder_matrix(int *solution_matrix, int *solution_matrix_wrong_order, int m, int blck_size, int matrix_size)
{
    for (int proc_row_i = 0; proc_row_i < m; proc_row_i++)
    {
        for (int proc_col_i = 0; proc_col_i < m; proc_col_i++)
        {
            for (int block_row_i = 0; block_row_i < blck_size; block_row_i++)
            {
                for (int block_col_i = 0; block_col_i < blck_size; block_col_i++)
                {
                    int index_in_block = block_row_i * blck_size + block_col_i;
                    int index_in_array = (proc_row_i * m + proc_col_i) * blck_size * blck_size + index_in_block;
                    int row = proc_row_i * blck_size + block_row_i;
                    int col = proc_col_i * blck_size + block_col_i;
                    solution_matrix[row * matrix_size + col] = solution_matrix_wrong_order[index_in_array];
                }
            }
        }
    }
}

void read_matrix(int matrix_size, int blck_size, int m, int *reoordered_matrix)
{
    for (int row_i = 0; row_i < matrix_size; row_i++)
    {
        for (int col_i = 0; col_i < matrix_size; col_i++)
        {
            int block_row_i = row_i / blck_size;
            int block_col_i = col_i / blck_size;
            int block_i = block_row_i * m + block_col_i;

            int col_i_in_block = col_i % blck_size;
            int row_i_in_block = row_i % blck_size;
            int index_in_block = row_i_in_block * blck_size + col_i_in_block;

            int index_in_array = block_i * blck_size * blck_size + index_in_block;
            int tmp;
            scanf("%d", &tmp);

            // we need to replace all 0 with infinity, except diagonal, because there is no edge between there
            if (tmp == 0 && row_i != col_i)
                tmp = MY_INFINITY;

            reoordered_matrix[index_in_array] = tmp;
        }
    }
}
