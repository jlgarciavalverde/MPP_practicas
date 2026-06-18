#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>
#include <stdbool.h>
#include "../include/mezclar.h"
#include "../include/mh.h"

// La tasa de mutación ahora es variable y se pasa como argumento (m_rate)

#define PRINT 0

// Esto se ha añadido para el ejercicio 2
static unsigned int seed;
#pragma omp threadprivate(seed)

// ------------------------- UTILIDADES -------------------------

int aleatorio(int n)
{
    //return rand() % n; // genera un número aleatorio entre 0 y n-1
    // genera un número aleatorio entre 0 y n-1 (estado por hilo)
    return rand_r(&seed) % n;
}
/*
// MergeSort secuencial
void mergeSort(Individuo **poblacion, int izq, int der)
{   
    int med = (izq + der)/2;
    if ((der - izq) < 2) { return; }
    
    mergeSort(poblacion, izq, med);
    mergeSort(poblacion, med, der);

    mezclar(poblacion, izq, med, der);
}

*/
// Introducido en el ejercicio 6 para comparar con qsort ordenar los individuos de la población por fitness
// MergeSort paralelo mediante tareas
void mergeSort(Individuo **poblacion, int izq, int der)
{   
    int med = (izq + der)/2;
    if ((der - izq) < 2) { return; }
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task shared(poblacion) firstprivate(izq, med)
            mergeSort(poblacion, izq, med);
            
            #pragma omp task shared(poblacion) firstprivate(med, der)
            mergeSort(poblacion, med, der);
            
            #pragma omp taskwait
            mezclar(poblacion, izq, med, der);
        }
    }
}

// SECUENCIAL
int find_element(int *array, int end, int element)
{
    for (int i = 0; i < end; i++)
    {
        if (array[i] == element)
            return 1;
    }
    return 0;
}
/*
// PARALELO TARDA MÁS
int find_element(const int *array, int end, int element) {
    int found = 0;

    #pragma omp parallel shared(found)
    {
        #pragma omp for
        for (int i = 0; i < end; ++i) {

            int f;
            #pragma omp atomic read
            f = found;

            if (f) { 
                #pragma omp cancel for
                continue; // salta al punto de cancelación
            }

            if (array[i] == element) {
                #pragma omp atomic write
                found = 1;
                #pragma omp cancel for
            }

            #pragma omp cancellation point for
        }
    }
    return found;
}
*/
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

    // NO PARARELIZABLE: puesto que hay dependencia de datos (RAW)
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
/*
// CRUZAR SECUENCIAL
 void cruzar(Individuo *padre1, Individuo *padre2,
            Individuo *hijo1, Individuo *hijo2, int n, int m)
{
    int posicionCorte = (aleatorio(m - 1)) + 1;
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
*/
/*
// CRUZAR PARALELO CON SECTIONS
 void cruzar(Individuo *padre1, Individuo *padre2,
            Individuo *hijo1, Individuo *hijo2, int n, int m)
{
    int posicionCorte = (aleatorio(m - 1)) + 1;

    #pragma omp parallel sections 
    {
        #pragma omp section 
        for (int i = 0; i < posicionCorte; i++)
        {   
            hijo1->array_int[i] = padre1->array_int[i];
            hijo2->array_int[i] = padre2->array_int[i];
        }

        #pragma omp section 
        for (int i = posicionCorte; i < m; i++)
        {
            hijo1->array_int[i] = padre2->array_int[i];
            hijo2->array_int[i] = padre1->array_int[i];
        }
    }
    // Barrea ímplicita al acabar las secciones. Procedemos a factibilizar

    #pragma omp parallel sections 
    {
        #pragma omp section 
        factibilizar(hijo1,n,m);
        
        #pragma omp section
        factibilizar(hijo2,n,m);
    }
}
*/
// CRUZAR PARALELO CON FOR
 void cruzar(Individuo *padre1, Individuo *padre2,
            Individuo *hijo1, Individuo *hijo2, int n, int m)
{
    int posicionCorte = (aleatorio(m - 1)) + 1;

    #pragma omp parallel 
    {
        #pragma omp for
        for (int i = 0; i < posicionCorte; i++)
        {   
            hijo1->array_int[i] = padre1->array_int[i];
            hijo2->array_int[i] = padre2->array_int[i];
        }

        #pragma omp for  
        for (int i = posicionCorte; i < m; i++)
        {
            hijo1->array_int[i] = padre2->array_int[i];
            hijo2->array_int[i] = padre1->array_int[i];
        }
    }
    // Barrea ímplicita al acabar las secciones. Procedemos a factibilizar

    #pragma omp parallel sections 
    {
        #pragma omp section 
        factibilizar(hijo1,n,m);
        
        #pragma omp section
        factibilizar(hijo2,n,m);
    }
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
    
    // NO ES PARALELIZABLE por el find_element y pos
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
/*
void fitness(const double *d, Individuo *individuo, int n, int m)
{
    double suma = 0.0;

    for (int i = 0; i < m; i++)
    {
        int elem_i = individuo->array_int[i];
        for (int j = i + 1; j < m; j++)
        {
            int elem_j = individuo->array_int[j];
            suma += distancia_ij(d, elem_i, elem_j, n); // variable compartida. problemas por L/E
        }
    }

    individuo->fitness = suma;
}
*/
/*
  ===========================================================
   OpenMP con critical
   compartidas:   d, individuo, n, m, suma
   privadas:  i, j, elem_i, elem_j, dist
   ===========================================================

void fitness(const double *d, Individuo *individuo, int n, int m)
{
    double suma = 0.0;

    #pragma omp parallel default(none) shared(d,individuo,n,m,suma) 
    {
        #pragma omp for
        for (int i = 0; i < m; i++)
        {
            int elem_i = individuo->array_int[i];
            for (int j = i + 1; j < m; j++)
            {
                int elem_j = individuo->array_int[j];
                double dist = distancia_ij(d, elem_i, elem_j, n); 

                #pragma omp critical
                { suma += dist; }
            }
        }
    }

    individuo->fitness = suma;
}
*/

 /*===========================================================
   OpenMP con critical OPTIMIZADA
   shared:   d, individuo, n, m, suma
   private:  i, j, elem_i, elem_j, dist
   ===========================================================

void fitness(const double *d, Individuo *individuo, int n, int m)
{
    double suma = 0.0;

    #pragma omp parallel default(none) shared(d,individuo,n,m,suma)
    {
        double dist = 0.0;

        #pragma omp for
        for (int i = 0; i < m; i++)
        {
            int elem_i = individuo->array_int[i];
            for (int j = i + 1; j < m; j++)
            {
                int elem_j = individuo->array_int[j];
                dist += distancia_ij(d, elem_i, elem_j, n);
            }
        }

        #pragma omp critical
        { suma += dist; }
    }

    individuo->fitness = suma;
}
*/
/*
 ===========================================================
   OpenMP con atomic
   shared:   d, individuo, n, m, suma
   private:  i, j, elem_i, elem_j, dist
   ===========================================================

void fitness(const double *d, Individuo *individuo, int n, int m)
{
    double suma = 0.0;

    #pragma omp parallel default(none) shared(d,individuo,n,m,suma) 
    {
        #pragma omp for
        for (int i = 0; i < m; i++)
        {
            int elem_i = individuo->array_int[i];
            for (int j = i + 1; j < m; j++)
            {
                
                int elem_j = individuo->array_int[j];
                double dist = distancia_ij(d, elem_i, elem_j, n);

                #pragma omp atomic
                suma += dist;
            }
        }
    }

    individuo->fitness = suma;
}
*/

/* ===========================================================
   OpenMP con atomic OPTIMIZADA
   shared:   d, individuo, n, m, suma
   private:  i, j, elem_i, elem_j, dist
   ===========================================================

void fitness(const double *d, Individuo *individuo, int n, int m)
{
    double suma = 0.0;
    #pragma omp parallel default(none) shared(d,individuo,n,m,suma) 
    {
        double dist = 0.0;
        #pragma omp for
        for (int i = 0; i < m; i++)
        {
            
            int elem_i = individuo->array_int[i];
            for (int j = i + 1; j < m; j++)
            {
                int elem_j = individuo->array_int[j];
                dist += distancia_ij(d, elem_i, elem_j, n);
            }    
        }
        #pragma omp atomic
        suma += dist; 
    }

    individuo->fitness = suma;
}
*/
/*
  ===========================================================
   OpenMP con reduction
   -----------------------------------------------------------
   shared:   d, individuo, n, m
   private:  i, j, elem_i, elem_j
   reduction: suma
   ===========================================================
*/
void fitness(const double *d, Individuo *individuo, int n, int m)
{
    double suma = 0.0;

    #pragma omp parallel for reduction(+:suma) default(none) shared(d,individuo,n,m)
    for (int i = 0; i < m; i++)
    {
        int elem_i = individuo->array_int[i];
        for (int j = i + 1; j < m; j++)
        {
            int elem_j = individuo->array_int[j];
            suma += distancia_ij(d, elem_i, elem_j, n);
        }
    }

    individuo->fitness = suma;
}

// ------------------------- ALGORITMO MH -------------------------

double aplicar_mh(const double *d, int n, int m, int n_gen, int tam_pob, double m_rate, int *sol)
{
    //srand(time(NULL) + getpid());
    // ------------------------------- Nuevo para el ejercicio 2 -----------------------------
    int maxT = omp_get_max_threads(); // es necesario generar la semilla en paralelo con el número de hilos máximo que se vaya a usar en el algoritmo
    #pragma omp parallel num_threads(maxT)
    {
        // Cada hilo inicializa un seed en paralelo de forma independiente
        unsigned int local_seed = (unsigned int)time(NULL) + omp_get_thread_num(); // TID, id del hilo
        seed = local_seed;
    }
    
    // ----------------------------------------------------------------------------------------
    Individuo **poblacion = (Individuo **)malloc(tam_pob * sizeof(Individuo *));
    assert(poblacion);

    // FOR_INI del ejercicio 2
    // Genera la población inicial
    #pragma omp parallel for schedule(dynamic,5)
    for (int i = 0; i < tam_pob; i++)
    {
        poblacion[i] = (Individuo *)malloc(sizeof(Individuo));
        poblacion[i]->array_int = crear_individuo(n, m);
        fitness(d, poblacion[i], n, m);
    }

    qsort(poblacion, tam_pob, sizeof(Individuo *), comp_fitness); // Ordena la población de modo que poblacion[0] es el individuo con mejor fitness
    //mergeSort(poblacion, 0, tam_pob);

    #pragma omp parallel 
    {
        for (int g = 0; g < n_gen; g++)
        {
            // cruce: reemplaza la segunda mitad  
            #pragma omp for      
            for (int i = 0; i < (tam_pob / 2) - 1; i += 2)
            {
                cruzar(poblacion[i], poblacion[i + 1],
                    poblacion[tam_pob / 2 + i],
                    poblacion[tam_pob / 2 + i + 1], n, m);
            }
    
            // mutación 3/4 de la población
            int mutation_start = tam_pob / 4;
            #pragma omp for
            for (int i = mutation_start; i < tam_pob; i++)
            {
                mutar(poblacion[i], n, m, m_rate);
            }

            #pragma omp for
            for (int i = 0; i < tam_pob; i++)
            {
                fitness(d, poblacion[i], n, m);
            }

            #pragma omp single // master --> más lento porque hay que esperar a que el hilo 0 quede ocioso si es que no lo está
            {
                //printf("Soy el hilo %d\n" , omp_get_thread_num());
                qsort(poblacion, tam_pob, sizeof(Individuo *), comp_fitness); // Ordena la población de modo que poblacion[0] es el individuo con mejor fitness 
                //mergeSort(poblacion, 0, tam_pob);
                
                if (PRINT)
                {
                    printf("Generacion %d - Fitness = %.0lf\n", g+1, poblacion[0]->fitness);
                }  
            }
        }
    }
    
    qsort(poblacion[0]->array_int, m, sizeof(int), comp_array_int); // Una vez tenemos el mejor individuo de la última generación, ordena sus elementos (distancias) en orden ascendente
    memmove(sol, poblacion[0]->array_int, m * sizeof(int)); // copia en sol los m elementos del array poblacion[0], solución final 

    double value = poblacion[0]->fitness;

    // Primero tenemos que liberar cada individuo
    for (int i = 0; i < tam_pob; i++)
    {
        free(poblacion[i]->array_int);
        free(poblacion[i]);
    }
    // Ahora liberamos la población
    free(poblacion);

    
    printf("Generaciones realizadas: %d\n", n_gen);

    return value;
}

