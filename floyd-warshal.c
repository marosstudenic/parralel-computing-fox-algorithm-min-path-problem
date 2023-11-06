#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

void floyd_warshall(int **dist, int V)
{
    for (int k = 0; k < V; k++)
    {
        for (int i = 0; i < V; i++)
        {
            for (int j = 0; j < V; j++)
            {
                if (dist[i][k] != INT_MAX && dist[k][j] != INT_MAX && dist[i][k] + dist[k][j] < dist[i][j])
                {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }
}

int main()
{
    int V;
    scanf("%d", &V);

    // Dynamically allocate memory for the 2D array
    int **dist = (int **)malloc(V * sizeof(int *));
    for (int i = 0; i < V; i++)
    {
        dist[i] = (int *)malloc(V * sizeof(int));
        for (int j = 0; j < V; j++)
        {
            scanf("%d", &dist[i][j]);
            if (dist[i][j] == 0 && i != j)
            {
                dist[i][j] = INT_MAX;
            }
        }
    }

    floyd_warshall(dist, V);

    // Print the shortest path matrix
    for (int i = 0; i < V; i++)
    {
        for (int j = 0; j < V; j++)
        {
            if (dist[i][j] == INT_MAX)
            {
                printf("0");
            }
            else
            {
                printf("%d", dist[i][j]);
            }
            if (j != V - 1)
            {
                printf(" ");
            }
        }

        printf("\n");
    }

    // Free the dynamically allocated memory
    for (int i = 0; i < V; i++)
    {
        free(dist[i]);
    }
    free(dist);

    return 0;
}
