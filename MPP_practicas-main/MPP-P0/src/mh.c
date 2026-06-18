
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#include "../include/mh.h"

// La tasa de mutación ahora es variable y se pasa como argumento (m_rate)

#define PRINT 0

// ------------------------- UTILIDADES -------------------------

int aleatorio(int n)
{
    return rand() % n; // genera un número aleatorio entre 0 y n-1
}

int find_element(int *array, int end, int element)
{
    for (int i = 0; i < end; i++)
    {
        if (array[i] == element)
            return 1;
    }
    return 0;
}

// ------------------------- CREACION DE INDIVIDUOS -------------------------

int *crear_individuo(int n, int m)
{
    if (n < m)
    {
        fprintf(stderr, "Error: n < m, imposible generar individuo único\n");
        exit(EXIT_FAILURE);
    }

    int *individuo = (int *)malloc(m * sizeof(int));
    memset(individuo, -1, m * sizeof(int));

    int i = 0, value;
    while (i < m)
    {
        value = aleatorio(n);
        if (!find_element(individuo, i, value))
        {
            individuo[i] = value;
            i++;
        }
    }
    return individuo;
}

// ------------------------- ORDEN Y COMPARACIONES -------------------------

int comp_array_int(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

int comp_fitness(const void *a, const void *b)
{
    return (*(Individuo **)b)->fitness - (*(Individuo **)a)->fitness;
}

// ------------------------- FACTIBILIZAR -------------------------

/**
 * @brief Corrige un individuo reemplazando duplicados por valores no usados,
 *        tomando candidatos del padre.
 *
 * @param hijo Individuo hijo a corregir.
 * @param padre Individuo padre usado como referencia para reemplazos.
 * @param m Número de elementos por individuo.
 * @param n Número total de posibles valores (tamaño del dominio).
 *
 * @note Garantiza que el hijo no tenga valores repetidos.
 */

void factibilizar(Individuo *hijo,int n, int m)
{
    for (int i = 1; i < m; i++)
    {
        int val = hijo->array_int[i];
        int nuevo_valor;
        // comprobar duplicado
        for (int j = 0; j < i; j++) // j recorre la parte izquierda a la i
        {
            if (hijo->array_int[j] == val) // Entra si hay duplicado
            {   
                nuevo_valor = aleatorio(n);
                while (find_element(hijo->array_int,m,nuevo_valor)) {
                    nuevo_valor = aleatorio(n);
                }
                hijo->array_int[j] = nuevo_valor;
            }
        }
    }
}

// ------------------------- CRUCE -------------------------

/**
 * @brief Realiza un cruce de un punto entre dos padres para generar dos hijos.
 *
 * @param padre1 Primer individuo padre.
 * @param padre2 Segundo individuo padre.
 * @param hijo1 Primer individuo hijo (resultado).
 * @param hijo2 Segundo individuo hijo (resultado).
 * @param n Número total de posibles valores.
 * @param m Número de elementos por individuo.
 *
 * @note Se asegura factibilidad de los hijos eliminando duplicados.
 */

void cruzar(Individuo *padre1, Individuo *padre2,
            Individuo *hijo1, Individuo *hijo2, int n, int m)
{
    int posicionCorte = (rand() % (m - 1)) + 1;

    for (int i = 0; i < posicionCorte; i++)
    {
        hijo1->array_int[i] = padre1->array_int[i];
        hijo2->array_int[i] = padre2->array_int[i];
    }
    for (int i = posicionCorte; i < m; i++)
    {
        hijo1->array_int[i] = padre2->array_int[i];
        hijo2->array_int[i] = padre1->array_int[i];
    }

    factibilizar(hijo1,n,m);
    factibilizar(hijo2,n,m);
}

// ------------------------- MUTACION -------------------------

/**
 * @brief Aplica mutaciones aleatorias a un individuo.
 *
 * @param actual Individuo a mutar.
 * @param n Número total de posibles valores (tamaño del dominio).
 * @param m Número de elementos por individuo.
 * @param m_rate Proporción de elementos a mutar (ej: 0.1 = 10%).
 *
 * @note Se intenta evitar duplicados, pero si no se encuentra valor libre
 *       después de varios intentos, se rompe el bucle.
 */

void mutar(Individuo *actual, int n, int m, double m_rate)
{
    int num_mutaciones = (int)(m_rate * m);

    for (int i = 0; i < num_mutaciones; i++)
    {
        int pos = aleatorio(m);
        int nuevo_valor = aleatorio(n); // // genera un número aleatorio entre 0 y n-1

        // buscar un valor disponible
        int intentos = 0;
        // find_element devuelve true (1) si hay en el array de m elementos algún gen con el nuevo valor a mutar
        // En caso de no encontrarlo se procede a la mutación
        // Establecemos un techo de 2n intentos 
        while (find_element(actual->array_int, m, nuevo_valor) && !(intentos > n*2)) {
            nuevo_valor = aleatorio(n);
            intentos++;
        }
        if (!find_element(actual->array_int, m, nuevo_valor)) { // procedemos a la mutación
            actual->array_int[pos] = nuevo_valor; 
        }
    }
}

/**
 * @brief Calcula la distancia entre dos elementos usando una matriz triangular.
 *
 * @param d Arreglo de distancias en forma comprimida (solo triangular superior).
 * @param i Índice del primer elemento.
 * @param j Índice del segundo elemento.
 * @param n Número total de elementos.
 * @return double Distancia entre i y j.
 *
 */

double distancia_ij(const double *d, int i, int j, int n)
{
    if (i == j)
        return 0.0; // distancia nula en la diagonal

    // Asegurar i < j para aplicar la fórmula
    if (i > j)
    {
        int tmp = i;
        i = j;
        j = tmp;
    }

    // Aplicamos la fórmula del enunciado
    int a = (n * n - n) / 2;
    int b = ((n - i) * (n - i) - (n - i)) / 2;
    int k = a - b + (j - i - 1);

    return d[k];
}

// ------------------------- FITNESS -------------------------

/**
 * @brief Evalúa el fitness de un individuo sumando las distancias entre todos
 *        sus elementos.
 *
 * @param d Arreglo de distancias comprimido (triangular superior).
 * @param individuo Individuo a evaluar.
 * @param n Número total de elementos en el dominio.
 * @param m Número de elementos por individuo.
 *
 * @note El fitness se guarda en la estructura del individuo.
 */

void fitness(const double *d, Individuo *individuo, int n, int m)
{
    double suma = 0.0;

    for (int i = 0; i < m; i++)
    {
        for (int j = i + 1; j < m; j++)
        {
            int elem_i = individuo->array_int[i];
            int elem_j = individuo->array_int[j];
            suma += distancia_ij(d, elem_i, elem_j, n);
        }
    }

    individuo->fitness = suma;
}

// ------------------------- ALGORITMO MH -------------------------

double aplicar_mh(const double *d, int n, int m, int n_gen, int tam_pob, double m_rate, int *sol)
{
    srand(time(NULL) + getpid());

    Individuo **poblacion = (Individuo **)malloc(tam_pob * sizeof(Individuo *));
    assert(poblacion);

    for (int i = 0; i < tam_pob; i++)
    {
        poblacion[i] = (Individuo *)malloc(sizeof(Individuo));
        poblacion[i]->array_int = crear_individuo(n, m);
        fitness(d, poblacion[i], n, m);
    }

    qsort(poblacion, tam_pob, sizeof(Individuo *), comp_fitness); // Ordena la población de modo que poblacion[0] es el individuo con mejor fitness

    int g;
    for (g = 0; g < n_gen; g++)
    {
        // cruce: reemplaza la segunda mitad
        for (int i = 0; i < (tam_pob / 2) - 1; i += 2)
        {
            cruzar(poblacion[i], poblacion[i + 1],
                   poblacion[tam_pob / 2 + i],
                   poblacion[tam_pob / 2 + i + 1], n, m);
        }

        // mutación 3/4 de la población
        int mutation_start = tam_pob / 4;
        for (int i = mutation_start; i < tam_pob; i++)
        {
            mutar(poblacion[i], n, m, m_rate);
        }

        for (int i = 0; i < tam_pob; i++)
        {
            fitness(d, poblacion[i], n, m);
        }

        qsort(poblacion, tam_pob, sizeof(Individuo *), comp_fitness); // Ordena la población de modo que poblacion[0] es el individuo con mejor fitness

        if (PRINT)
        {
            printf("Generacion %d - Fitness = %.0lf\n", g, poblacion[0]->fitness);
        }
    }

    qsort(poblacion[0]->array_int, m, sizeof(int), comp_array_int);
    memmove(sol, poblacion[0]->array_int, m * sizeof(int));

    double value = poblacion[0]->fitness;

    // Primero tenemos que liberar cada individuo
    for (int i = 0; i < tam_pob; i++)
    {
        free(poblacion[i]->array_int);
        free(poblacion[i]);
    }
    // Ahora liberamos la población
    free(poblacion);

    printf("Generaciones realizadas: %d\n", g);

    return value;
}
