
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <mpi.h>
#include <unistd.h>
#include "../include/mh.h"

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
/*
int comp_fitness(const void *a, const void *b)
{
    return (*(Individuo **)b)->fitness - (*(Individuo **)a)->fitness;
}
*/
int comp_fitness(const void *a, const void *b)
{
    const Individuo *ia = (const Individuo *)a;
    const Individuo *ib = (const Individuo *)b;
    // Orden descendente (mejor primero):
    if (ib->fitness > ia->fitness) return 1;
    if (ib->fitness < ia->fitness) return -1;
    return 0;
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

double aplicar_mh(const double *d, int n, int m, int n_gen, int tam_pob, double m_rate, int nem, int ngm, int *sol, int myrank, int size, MPI_Datatype mpi_tipo_individuo) {
    srand(time(NULL) + getpid());

    int tam_isla_esclavo = tam_pob / size;
    int tam_isla; 

    if (myrank == 0) { 
        int resto = tam_pob % size;
        tam_isla = tam_isla_esclavo + resto;
    } else {
        tam_isla = tam_isla_esclavo;
    }

    if (nem > tam_isla) {
        fprintf(stderr, "Error: nem=%d excede tam_isla=%d\n", nem, tam_isla);
        MPI_Abort(MPI_COMM_WORLD, 1); 
    }

    Individuo *isla = (Individuo *)malloc(tam_isla * sizeof(Individuo));
    assert(isla);

    /********************************* INICIALIZACIÓN Y REPARTO COLECTIVO (Scatterv) ************************************/
    
    // Variables auxiliares para el reparto con Scatterv
    Individuo *poblacion_inicial = NULL; // Solo P0 la usa
    int *counts_envio = NULL;          
    int *desplazamientos = NULL;         

    if (myrank == 0) { 

        poblacion_inicial = (Individuo *)malloc(tam_pob * sizeof(Individuo)); 
        assert(poblacion_inicial);
        
        // Generación de la población inicial
        for (int i = 0; i < tam_pob; i++) {
            int *tmp_array = crear_individuo(n, m); 
            memcpy(poblacion_inicial[i].array_int, tmp_array, (size_t)m * sizeof(int));
            free(tmp_array);
            fitness(d, &poblacion_inicial[i], n, m);
        }
        
        // Preparar arrays para MPI_Scatterv
        counts_envio = (int *)malloc(size * sizeof(int)); // cuántos elementos enviar a cada proceso
        desplazamientos = (int *)malloc(size * sizeof(int)); // desde dónde debe tomar cada bloque del array poblacion_inicial
        assert(counts_envio && desplazamientos);

        // P0 recibe tam_isla individuos
        counts_envio[0] = tam_isla;
        desplazamientos[0] = 0;
        
        // Esclavos reciben tam_isla_esclavo individuos
        for (int i = 1; i < size; i++) {
            counts_envio[i] = tam_isla_esclavo;
            desplazamientos[i] = tam_isla + (i - 1) * tam_isla_esclavo;
        }
        
    } 
    
    // P0 envía desde poblacion_inicial usando los arrays de control
    // El resto recibe en isla
    MPI_Scatterv(poblacion_inicial, // Buffer de envío (solo P0)
                 counts_envio,      // Array de cuántos enviar (solo P0)
                 desplazamientos,   // Array de desplazamientos (solo P0)
                 mpi_tipo_individuo,// Tipo de dato a enviar
                 isla,              // Buffer de recepción (TODOS)
                 tam_isla,          // Cuánto recibe ESTE proceso
                 mpi_tipo_individuo,// Tipo de dato a recibir
                 0,                 // Root
                 MPI_COMM_WORLD);

    if (myrank == 0) {
        free(poblacion_inicial); 
        free(counts_envio);
        free(desplazamientos);
    }
    /********************************* FIN INICIALIZACIÓN Y REPARTO ************************************/
    
    qsort(isla, tam_isla, sizeof(Individuo), comp_fitness); 

    // BUCLE EVOLUTIVO
    for (int g = 0; g < n_gen; g++)
    {  
        // Migramos si se requiere
        if (g > 0 && (g % ngm == 0)) 
        {   
            int total_migrantes;
            Individuo *migrantes_a_enviar = isla; // Son los NEM mejores de la isla
            Individuo *buffer_mezcla = NULL;      // Solo P0 lo usa para recibir
            
            if (myrank == 0) {
                total_migrantes = nem * size;
                buffer_mezcla = (Individuo *)malloc(total_migrantes * sizeof(Individuo));
                assert(buffer_mezcla);
            }
            
            // Recolección con Gather
            // Todos envían sus nem mejores a buffer_mezcla de P0
            MPI_Gather(migrantes_a_enviar, // Buffer de envío (los NEM mejores)
                       nem,                // Cantidad de envío
                       mpi_tipo_individuo, // Tipo de envío
                       buffer_mezcla,      // Buffer de recepción (solo P0)
                       nem,                // Cantidad de recepción (por proceso)
                       mpi_tipo_individuo, // Tipo de recepción
                       0,                  // Root
                       MPI_COMM_WORLD);

            Individuo *mejores_globales = NULL; 

            if (myrank == 0) {
                qsort(buffer_mezcla, total_migrantes, sizeof(Individuo), comp_fitness);
                mejores_globales = buffer_mezcla; // P0 usará su propio buffer para el Bcast
            } else {
                // Esclavos reservan memoria para recibir el Bcast
                mejores_globales = (Individuo *)malloc(nem * sizeof(Individuo));
                assert(mejores_globales);
            }
            
            // P0 envía los nem mejores a todos
            MPI_Bcast(mejores_globales, // P0 envía buffer_mezcla, esclavos reciben en mejores_globales
                      nem, // numero de elementos en el buffer
                      mpi_tipo_individuo,
                      0, // Root
                      MPI_COMM_WORLD);

            // Todos reemplazan sus NEM peores por los NEM mejores globales
            memcpy(&isla[tam_isla - nem], mejores_globales, nem * sizeof(Individuo));

            // Liberar buffers temporales
            if (myrank == 0) {
                free(buffer_mezcla); // mejores_globales apuntaba a buffer_mezcla
            } else {
                free(mejores_globales);
            }

            // Todos los procesos reordenan su población local después del reemplazo/migración
            qsort(isla, tam_isla, sizeof(Individuo), comp_fitness);
        }
        
        // cruce
        for (int i = 0; i < (tam_isla / 2) - 1; i += 2) 
        {
            cruzar(&isla[i], 
                   &isla[i + 1],
                   &isla[tam_isla / 2 + i], 
                   &isla[tam_isla / 2 + i + 1], n, m); 
        }

        // mutación
        int mutation_start = tam_isla / 4; 
        for (int i = mutation_start; i < tam_isla; i++)
        {
            mutar(&isla[i], n, m, m_rate);
        }

        for (int i = 0; i < tam_isla; i++)
        {
            fitness(d, &isla[i], n, m);
        }

        qsort(isla, tam_isla, sizeof(Individuo), comp_fitness); 
        
        if (PRINT)
        {
            printf("Generacion %d, Proceso %d, Fitness = %.0lf\n", g, myrank, isla[0].fitness); 
        }
    }
    // FIN DEL BUCLE EVOLUTIVO
    
    // RECOLECCIÓN FINAL COLECTIVA con Gather
    double value;
    Individuo *finalistas = NULL; // Solo P0 lo usa

    if (myrank == 0) {
        finalistas = (Individuo *)malloc(size * sizeof(Individuo));
        assert(finalistas);
    }
    
    // Todos envían su mejor individuo isla[0] al buffer finalistas de P0
    MPI_Gather(&isla[0],           // Buffer de envío (el mejor)
               1,                  // Cantidad de envío
               mpi_tipo_individuo, // Tipo de envío
               finalistas,         // Buffer de recepción (solo P0)
               1,                  // Cantidad de recepción (por proceso)
               mpi_tipo_individuo, // Tipo de recepción
               0,                  // Root
               MPI_COMM_WORLD);

    if (myrank == 0) {
        // P0 Ordena a todos los finalistas
        qsort(finalistas, size, sizeof(Individuo), comp_fitness);

        // Preparamos la solución (el mejor de todos)
        qsort(finalistas[0].array_int, m, sizeof(int), comp_array_int);
        memmove(sol, finalistas[0].array_int, m * sizeof(int));
        value = finalistas[0].fitness; 

        free(finalistas); 

    } else { // ESCLAVOS (myrank > 0)
        value = isla[0].fitness; // ignorado en main.c 
    }

    if (isla) free(isla);

    if (PRINT) {
        printf("Generaciones realizadas: %d\n", n_gen);
    }

    return value; 
}