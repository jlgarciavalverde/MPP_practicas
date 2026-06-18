#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

/**
 * @brief Lee las distancias entre nodos desde la entrada estándar.
 * @param n Número de nodos.
 * @return Puntero a un arreglo de distancias.
 */
double *read_distances(int n)
{
	int i, aux;

        //if(scanf("%d", &n)){};
        //if(scanf("%d", &aux)){};

        // Linea original double *d = (double *) malloc(((n*n-n)/2)*sizeof(int));
        double *d = (double *) malloc(((n*n-n)/2)*sizeof(double));

        for(i = 0; i < (n*n-n)/2; i++) {
                scanf("%d %d %lf", &aux, &aux, &d[i]);
        }

	return d;
}

/**
 * @brief Imprime las distancias entre nodos.
 * @param d Puntero al arreglo de distancias.
 * @param n Número de nodos.
 */
void print_distances(double *d, int n)
{
        int i,j,pos=0;

        printf("\nDistances: \n\n");
        // Aqui iba hasta <=n y se salia
        for(i = 0; i < n; i++)
        {
           for(j=i+1;j<n;j++)
           {
               printf("d %d %d %.2lf\n", i, j, d[pos]);
               pos+=1;
           }
           printf("\n");
        }
        //printf("\n");
}

void print_solution(int n, int m, const int *solucion, double valor)
{
	printf("\nSolution: ");
	for(int i = 0; i < m; i++) { printf("%d ", solucion[i]); }
	printf("\nDistance: %.2lf\n", valor);
}
