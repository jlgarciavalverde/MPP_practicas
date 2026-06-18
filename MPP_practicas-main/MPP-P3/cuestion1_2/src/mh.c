#include "../include/mh.h"
#include <assert.h>
#include <mpi.h>
#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PRINT 0

// ------------------------- UTILIDADES DE PARALELISMO -------------------------

// Semilla privada para cada hilo (como en MPP-P1)
static unsigned int seed;
#pragma omp threadprivate(seed)

// ------------------------- GENERACIÓN DE ALEATORIOS -------------------------

/**
 * @brief Genera un número aleatorio seguro para hilos.
 * @param n Límite superior del rango (exclusivo).
 * @return Número aleatorio entre 0 y n-1.
 */
int aleatorio(int n)
{
    return rand_r(&seed) % n; // genera un número aleatorio entre 0 y n-1
}

// ------------------------- BÚSQUEDA Y ORDENACIÓN -------------------------

// Versión secuencial (más eficiente que la paralela para bucles pequeños)
/**
 * @brief Busca un elemento en un arreglo.
 * @param array Arreglo donde buscar.
 * @param end Tamaño del arreglo.
 * @param element Elemento a buscar.
 * @return 1 si el elemento está presente, 0 en caso contrario.
 */
int find_element(int *array, int end, int element) 
{
    for (int i = 0; i < end; i++) {
        if (array[i] == element)
            return 1;
    }
    return 0;
}

/**
 * @brief Compara dos enteros para ordenación.
 * @param a Puntero al primer entero.
 * @param b Puntero al segundo entero.
 * @return Diferencia entre los dos enteros.
 */
int comp_array_int(const void *a, const void *b) 
{
    return (*(int *)a - *(int *)b);
}

/**
 * @brief Compara el fitness de dos individuos para ordenación.
 * @param a Puntero al primer individuo.
 * @param b Puntero al segundo individuo.
 * @return 1 si el segundo tiene mejor fitness, -1 si el primero tiene mejor fitness, 0 si son iguales.
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

// ------------------------- CREACIÓN DE INDIVIDUOS -------------------------

/**
 * @brief Crea un nuevo individuo con valores únicos.
 * @param n Número total de nodos.
 * @param m Número de nodos en el individuo.
 * @return Puntero al arreglo que representa el individuo.
 */
int *crear_individuo(int n, int m) 
{
    if (n < m) {
        fprintf(stderr, "Error: n < m, imposible generar individuo único\n");
        exit(EXIT_FAILURE);
    }

    int *individuo = (int *)malloc(m * sizeof(int));
    memset(individuo, -1, m * sizeof(int));

    int i = 0, value;
    // Bucle inherentemente secuencial por la dependencia de 'individuo' en cada iteración
    while (i < m) {
        value = aleatorio(n);
        if (!find_element(individuo, i, value)) {
            individuo[i] = value;
            i++;
        }
    }
    return individuo;
}

// ------------------------- FACTIBILIZAR -------------------------

/**
 * @brief Corrige un individuo reemplazando duplicados por valores únicos.
 * @param hijo Puntero al individuo a corregir.
 * @param n Número total de nodos.
 * @param m Número de nodos en el individuo.
 */
void factibilizar(Individuo *hijo, int n, int m) 
{
    // Bucle secuencial interno, pero la llamada es thread-safe
    for (int i = 1; i < m; i++) {
        int val = hijo->array_int[i];
        int nuevo_valor;
        // comprobar duplicado
        for (int j = 0; j < i; j++) {
            if (hijo->array_int[j] == val) { // Entra si hay duplicado
                nuevo_valor = aleatorio(n);
                while (find_element(hijo->array_int, m, nuevo_valor)) {
                    nuevo_valor = aleatorio(n);
                }
                hijo->array_int[j] = nuevo_valor;
            }
        }
    }
}

// ------------------------- CRUCE Y MUTACION -------------------------

/**
 * @brief Realiza un cruce de un punto entre dos padres para generar dos hijos.
 * @param padre1 Primer padre.
 * @param padre2 Segundo padre.
 * @param hijo1 Primer hijo generado.
 * @param hijo2 Segundo hijo generado.
 * @param n Número total de nodos.
 * @param m Número de nodos en el individuo.
 */
void cruzar(Individuo *padre1, Individuo *padre2, Individuo *hijo1,
            Individuo *hijo2, int n, int m) 
{
    int posicionCorte = (aleatorio(m - 1)) + 1;

    // Paralelización de la copia de arrays
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < posicionCorte; i++) {
            hijo1->array_int[i] = padre1->array_int[i];
            hijo2->array_int[i] = padre2->array_int[i];
        }

        #pragma omp for
        for (int i = posicionCorte; i < m; i++) {
            hijo1->array_int[i] = padre2->array_int[i];
            hijo2->array_int[i] = padre1->array_int[i];
        }
    }
    
    // Factibilizar a los dos hijos en paralelo (funciona bien si factibilizar es eficiente)
    #pragma omp parallel sections 
    {
        #pragma omp section 
        factibilizar(hijo1,n,m);
        
        #pragma omp section
        factibilizar(hijo2,n,m);
    }
}

/**
 * @brief Aplica mutaciones aleatorias a un individuo.
 * @param actual Puntero al individuo a mutar.
 * @param n Número total de nodos.
 * @param m Número de nodos en el individuo.
 * @param m_rate Tasa de mutación.
 */
void mutar(Individuo *actual, int n, int m, double m_rate) 
{
    int num_mutaciones = (int)(m_rate * m);

    // Bucle secuencial por la necesidad de asegurar la no-duplicidad
    for (int i = 0; i < num_mutaciones; i++) {
        int pos = aleatorio(m);
        int nuevo_valor = aleatorio(n);

        int intentos = 0;
        while (find_element(actual->array_int, m, nuevo_valor) &&
               !(intentos > n * 2)) {
            nuevo_valor = aleatorio(n);
            intentos++;
        }
        if (!find_element(actual->array_int, m, nuevo_valor)) { 
            actual->array_int[pos] = nuevo_valor;
        }
    }
}

// ------------------------- DISTANCIA Y FITNESS -------------------------

/**
 * @brief Calcula la distancia entre dos nodos.
 * @param d Puntero a la matriz de distancias.
 * @param i Índice del primer nodo.
 * @param j Índice del segundo nodo.
 * @param n Número total de nodos.
 * @return Distancia entre los nodos i y j.
 */
double distancia_ij(const double *d, int i, int j, int n) 
{
    if (i == j)
        return 0.0;

    if (i > j) {
        int tmp = i;
        i = j;
        j = tmp;
    }

    int a = (n * n - n) / 2;
    int b = ((n - i) * (n - i) - (n - i)) / 2;
    int k = a - b + (j - i - 1);

    return d[k];
}

/**
 * @brief Evalúa el fitness de un individuo.
 * @param d Puntero a la matriz de distancias.
 * @param individuo Puntero al individuo a evaluar.
 * @param n Número total de nodos.
 * @param m Número de nodos en el individuo.
 */
void fitness(const double *d, Individuo *individuo, int n, int m) 
{
    double suma = 0.0;

    // Paraleliza el bucle exterior y usa reduction para manejar 'suma'
    #pragma omp parallel for reduction(+ : suma) default(none)                     \
        shared(d, individuo, n, m)
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

// ------------------------- ALGORITMO MH HÍBRIDO -------------------------

/**
 * @brief Aplica el algoritmo híbrido de metaheurística.
 * @param d Puntero a la matriz de distancias.
 * @param n Número total de nodos.
 * @param m Número de nodos en el individuo.
 * @param n_gen Número de generaciones.
 * @param tam_pob Tamaño de la población.
 * @param m_rate Tasa de mutación.
 * @param nem Número de emigrantes.
 * @param ngm Número de generaciones entre migraciones.
 * @param sol Puntero al arreglo donde se almacenará la solución.
 * @param myrank Rango del proceso MPI.
 * @param size Tamaño del comunicador MPI.
 * @param mpi_tipo_individuo Tipo de dato MPI para el individuo.
 * @return Fitness de la mejor solución encontrada.
 */
double aplicar_mh(const double *d, int n, int m, int n_gen, int tam_pob,
                  double m_rate, int nem, int ngm, int *sol, int myrank,
                  int size, MPI_Datatype mpi_tipo_individuo) 
{
    // Cuestión 1: Inicialización de la semilla para cada hilo (OpenMP)
    int maxT = omp_get_max_threads(); 
    #pragma omp parallel num_threads(maxT)
    {
        unsigned int local_seed = (unsigned int)time(NULL) + omp_get_thread_num();
        seed = local_seed;
    }

    int dest, source = 0;
    int tag_init = 10;
    int tag_migra = 20;
    int tag_final = 30;
    MPI_Status status;

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

    /********************************* INICIALIZACIÓN Y REPARTO (MPI) ************************************/
    if (myrank == 0) { 
        Individuo *poblacion_inicial = (Individuo *)malloc(tam_pob * sizeof(Individuo));
        assert(poblacion_inicial);

        // Generación de la POBLACIÓN COMPLETA (Paralela en P0 - FOR_INI)
        #pragma omp parallel for schedule(dynamic, 5)
        for (int i = 0; i < tam_pob; i++) {
            int *tmp_array = crear_individuo(n, m);
            memcpy(poblacion_inicial[i].array_int, tmp_array, (size_t)m * sizeof(int));
            free(tmp_array);
            // Cálculo de fitness (internamente paralelo con reduction)
            fitness(d, &poblacion_inicial[i], n, m); 
        }

        memcpy(isla, poblacion_inicial, tam_isla * sizeof(Individuo)); 

        // Reparto (MPI_Ssend)
        for (dest = 1; dest < size; dest++) { 
            Individuo *bloque_a_enviar = poblacion_inicial + tam_isla + ((dest - 1) * tam_isla_esclavo);
            MPI_Ssend(bloque_a_enviar, tam_isla_esclavo, mpi_tipo_individuo, dest, tag_init, MPI_COMM_WORLD);
        }
        
        free(poblacion_inicial); 
        
    } else { // ESCLAVOS (myrank > 0)
        MPI_Recv(isla, tam_isla, mpi_tipo_individuo, source, tag_init, MPI_COMM_WORLD, &status);
    }
    /******************************* FIN INICIALIZACIÓN Y REPARTO *********************************/
    
    // Ordenación Inicial (Secuencial, qsort)
    qsort(isla, tam_isla, sizeof(Individuo), comp_fitness); 

    // BUCLE EVOLUTIVO
    for (int g = 0; g < n_gen; g++)
    {  
        // --------------------------------- MIGRACIÓN (MPI - Lógica de control en omp single) ---------------------------------
        if (g > 0 && (g % ngm == 0)) 
        {
            // La lógica de MPI debe ejecutarse secuencialmente por un solo hilo
            #pragma omp master
            {
                Individuo *migrantes_a_enviar = isla; 
                
                if (myrank == 0) {
                    // MAESTRO (P0): Recibe, Mezcla, Ordena y Redistribuye (Lógica MPI)
                    int total_migrantes = nem * size;
                    Individuo *buffer_mezcla = (Individuo *)malloc(total_migrantes * sizeof(Individuo));
                    assert(buffer_mezcla);
                    
                    memcpy(buffer_mezcla, migrantes_a_enviar, nem * sizeof(Individuo)); 

                    for (int i = 1; i < size; i++) {
                        MPI_Recv(&buffer_mezcla[i * nem], nem, mpi_tipo_individuo, MPI_ANY_SOURCE, tag_migra, MPI_COMM_WORLD, &status);
                    }
                    
                    qsort(buffer_mezcla, total_migrantes, sizeof(Individuo), comp_fitness);

                    for (dest = 1; dest < size; dest++) { 
                        MPI_Ssend(buffer_mezcla, nem, mpi_tipo_individuo, dest, tag_migra, MPI_COMM_WORLD);
                    } 
                    
                    memcpy(&isla[tam_isla - nem], buffer_mezcla, nem * sizeof(Individuo));
                    free(buffer_mezcla);

                } else { // ESCLAVOS (myrank > 0)
                    MPI_Ssend(migrantes_a_enviar, nem, mpi_tipo_individuo, 0, tag_migra, MPI_COMM_WORLD);
                    
                    MPI_Recv(&isla[tam_isla - nem], nem, mpi_tipo_individuo, 0, tag_migra, MPI_COMM_WORLD, &status);
                }
                
                qsort(isla, tam_isla, sizeof(Individuo), comp_fitness);
            }
            #pragma omp barrier // Asegurar que la migración y ordenación ha terminado antes de continuar
        }
        // ------------------------------ FIN MIGRACIÓN ----------------------------------
        
        // El bucle de generación debe ejecutarse en un entorno parallel
        #pragma omp parallel
        {
            // cruce (OpenMP)
            // Paralelización del bucle exterior de parejas
            #pragma omp for schedule(static)
            for (int i = 0; i < (tam_isla / 2) - 1; i += 2) 
            {
                // cruzar maneja su paralelismo interno (copia y factibilizar)
                cruzar(&isla[i], 
                       &isla[i + 1],
                       &isla[tam_isla / 2 + i], 
                       &isla[tam_isla / 2 + i + 1], n, m); 
            }

            // mutación (OpenMP)
            int mutation_start = tam_isla / 4; 
            // Paralelización del bucle exterior sobre individuos a mutar
            #pragma omp for schedule(static)
            for (int i = mutation_start; i < tam_isla; i++)
            {
                // mutar es secuencial, pero las llamadas son paralelas
                mutar(&isla[i], n, m, m_rate);
            }

            // fitness (OpenMP)
            // Paralelización del bucle exterior sobre todos los individuos para evaluar
            #pragma omp for schedule(static)
            for (int i = 0; i < tam_isla; i++)
            {
                // fitness maneja su paralelismo interno (reduction)
                fitness(d, &isla[i], n, m);
            }

            // Ordenación: qsort es secuencial. Usamos single para que solo un hilo lo ejecute.
            #pragma omp single
            {
                qsort(isla, tam_isla, sizeof(Individuo), comp_fitness); 
                
                if (PRINT)
                {
                    printf("Generacion %d, Proceso %d, Fitness = %.0lf\n", g, myrank, isla[0].fitness); 
                }
            }
        } // Fin #pragma omp parallel
    } 
    // FIN DEL BUCLE EVOLUTIVO
    
    // RECOLECCIÓN FINAL (MPI)
    double value;

    if (myrank == 0) {
        Individuo *finalistas = (Individuo *)malloc(size * sizeof(Individuo));
        assert(finalistas);

        memcpy(&finalistas[0], &isla[0], sizeof(Individuo));

        for (int i = 1; i < size; i++) {
            MPI_Recv(&finalistas[i], 1, mpi_tipo_individuo, MPI_ANY_SOURCE, tag_final, MPI_COMM_WORLD, &status);
        }

        qsort(finalistas, size, sizeof(Individuo), comp_fitness);

        qsort(finalistas[0].array_int, m, sizeof(int), comp_array_int);
        memmove(sol, finalistas[0].array_int, m * sizeof(int));
        value = finalistas[0].fitness; 

        free(finalistas); 

    } else { 
        MPI_Ssend(&isla[0], 1, mpi_tipo_individuo, 0, tag_final, MPI_COMM_WORLD);
        value = isla[0].fitness;
    }

    if (isla) free(isla);

    if (PRINT) {
        printf("Generaciones realizadas: %d\n", n_gen);
    }

    return value; 
}