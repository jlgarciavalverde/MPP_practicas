
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

// ------------------------- ALGORITMO MH -------------------------

#define BUFFER_OVERHEAD 1000

double aplicar_mh(const double *d, int n, int m, int n_gen, int tam_pob, double m_rate, int nem, int ngm, int *sol, int myrank, int size)
{
    srand(time(NULL) + getpid());

    // --- INICIO DE CAMBIOS 1 ---
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
    
    // El buffer aloja el bloque más grande que se transfiere. Se reserva para el peor caso
    size_t buffer_size_bytes = tam_isla_esclavo * (sizeof(double) + L_MAX * sizeof(int)) + BUFFER_OVERHEAD;

    char *buffer_pack = (char *)malloc(buffer_size_bytes); 
    assert(buffer_pack);
    
    char *buffer_recepcion = (char *)malloc(buffer_size_bytes); 
    assert(buffer_recepcion);  

    int position;
    int dest, source = 0;
    int tag_init = 10;
    int tag_migra = 20; // Etiqueta para la migración
    MPI_Status status;

    Individuo *isla = (Individuo *)malloc(tam_isla * sizeof(Individuo));
    assert(isla);

    /********************************* INICIALIZACIÓN DE LA POBLACIÓN Y REPARTO ************************************/
    if (myrank == 0) { 
        // MAESTRO (P0)
        Individuo *poblacion_inicial = (Individuo *)malloc(tam_pob * sizeof(Individuo)); 
        assert(poblacion_inicial);
        
        // Generación de la POBLACIÓN COMPLETA (tam_pob)
        for (int i = 0; i < tam_pob; i++) {
            int *tmp_array = crear_individuo(n, m); 
            memcpy(poblacion_inicial[i].array_int, tmp_array, (size_t)m * sizeof(int));
            free(tmp_array);
            fitness(d, &poblacion_inicial[i], n, m);
        }
        
        // P0 se asigna su propia subpoblación
        memcpy(isla, poblacion_inicial, tam_isla * sizeof(Individuo)); 

        // Reparto de la población inicial al resto de procesos
        for (dest = 1; dest < size; dest++) { 
            position = 0;
            // tam_isla es el tamaño de la isla del P0 que puede ser distinto al de los esclavos
            Individuo *bloque_a_enviar = poblacion_inicial + tam_isla + ((dest - 1) * tam_isla_esclavo);
            
            // Empaquetar individuo a individuo en buffer_pack
            for (int i = 0; i < tam_isla_esclavo; i++) {
                MPI_Pack(&bloque_a_enviar[i].fitness, 1, MPI_DOUBLE, buffer_pack, (int)buffer_size_bytes, &position, MPI_COMM_WORLD);
                MPI_Pack(bloque_a_enviar[i].array_int, L_MAX, MPI_INT, buffer_pack, (int)buffer_size_bytes, &position, MPI_COMM_WORLD);
            }
            int packed_size = position;
            
            // Envío de buffer_pack
            MPI_Ssend(buffer_pack, packed_size, MPI_PACKED, dest, tag_init, MPI_COMM_WORLD);
        }
        
        free(poblacion_inicial); 
        
    } else { // ESCLAVOS (myrank > 0)
        
        // Recepción Bloqueante Síncrona del buffer empaquetado
        MPI_Recv(buffer_pack, (int)buffer_size_bytes, MPI_PACKED, source, tag_init, MPI_COMM_WORLD, &status);
                 
        // Obtener el tamaño real y desempaquetar
        int count;
        MPI_Get_count(&status, MPI_PACKED, &count);

        // Desempaquetar los bytes recibidos directamente en la subpoblación local 'isla'
        position = 0;
        
        // Desempaquetar individuo por individuo (tam_isla veces)
        for (int i = 0; i < tam_isla; i++) {
            MPI_Unpack(buffer_pack, count, &position, &isla[i].fitness, 1, MPI_DOUBLE, MPI_COMM_WORLD);
            MPI_Unpack(buffer_pack, count, &position, isla[i].array_int, L_MAX, MPI_INT, MPI_COMM_WORLD);
        }
    }
    /********************************* FIN INICIALIZACIÓN DE LA POBLACIÓN Y REPARTO ************************************/
    //printf("Soy el proceso: %d, el fitness de mi primer individuo es: %f\n", myrank, isla[0].fitness);
    
    // Ordenación Inicial
    qsort(isla, tam_isla, sizeof(Individuo), comp_fitness); 

    // BUCLE EVOLUTIVO
    for (int g = 0; g < n_gen; g++)
    {  
        // 1. MIGRACIÓN (Al inicio de la generación, si toca)
        if (g > 0 && (g % ngm == 0)) 
        {
            Individuo *migrantes_a_enviar = isla; 
            
            if (myrank == 0) {
                // MAESTRO (P0): Recibe, Mezcla, Ordena y Redistribuye
                int total_migrantes = nem * size;
                Individuo *buffer_mezcla = (Individuo *)malloc(total_migrantes * sizeof(Individuo));
                assert(buffer_mezcla);
                
                // Paso 1: Copiar los propios NEM mejores de P0
                memcpy(buffer_mezcla, migrantes_a_enviar, nem * sizeof(Individuo)); 

                // Paso 2: Recibir síncronamente de los esclavos (P1 a P_size-1)
                for (source = 1; source < size; source++) {
                    
                    // Usamos buffer_recepcion para recibir
                    MPI_Recv(buffer_recepcion, (int)buffer_size_bytes, MPI_PACKED, source, tag_migra, MPI_COMM_WORLD, &status);
                             
                    // Desempaquetar en la posición correcta del buffer de mezcla
                    int count;
                    MPI_Get_count(&status, MPI_PACKED, &count);
                    position = 0; 
                    
                    for (int i = 0; i < nem; i++) {
                        MPI_Unpack(buffer_recepcion, count, &position, &buffer_mezcla[source * nem + i].fitness, 1, MPI_DOUBLE, MPI_COMM_WORLD);
                        MPI_Unpack(buffer_recepcion, count, &position, buffer_mezcla[source * nem + i].array_int, L_MAX, MPI_INT, MPI_COMM_WORLD);
                    }
                }
                
                // Paso 3: Ordenación Global (Selección de los mejores)
                qsort(buffer_mezcla, total_migrantes, sizeof(Individuo), comp_fitness);

                // Paso 4: Redistribución Síncrona a los esclavos (P1 a P_size-1)
                for (dest = 1; dest < size; dest++) { 
                    // Empaquetar los NEM MEJORES GLOBALES (buffer_mezcla[0] a buffer_mezcla[nem-1])
                    position = 0;
                    for (int i = 0; i < nem; i++) {
                        MPI_Pack(&buffer_mezcla[i].fitness, 1, MPI_DOUBLE, buffer_pack, (int)buffer_size_bytes, &position, MPI_COMM_WORLD);
                        MPI_Pack(buffer_mezcla[i].array_int, L_MAX, MPI_INT, buffer_pack, (int)buffer_size_bytes, &position, MPI_COMM_WORLD);
                    }
                    int packed_size = position;
                    
                    // Envío Síncrono
                    MPI_Ssend(buffer_pack, packed_size, MPI_PACKED, dest, tag_migra, MPI_COMM_WORLD);
                } 
                
                // Paso 5: Reemplazo local de P0 (reemplazar NEM peores con NEM mejores globales)
                memcpy(&isla[tam_isla - nem], buffer_mezcla, nem * sizeof(Individuo));
                
                free(buffer_mezcla);

            } else { 
                // ESCLAVOS (myrank > 0): ENVÍA y RECIBE SÍNCRONO

                // 1. Empaquetar los NEM mejores individuos locales
                position = 0;
                for (int i = 0; i < nem; i++) {
                    MPI_Pack(&migrantes_a_enviar[i].fitness, 1, MPI_DOUBLE, buffer_pack, (int)buffer_size_bytes, &position, MPI_COMM_WORLD);
                    MPI_Pack(migrantes_a_enviar[i].array_int, L_MAX, MPI_INT, buffer_pack, (int)buffer_size_bytes, &position, MPI_COMM_WORLD);
                }
                int packed_size = position;
                
                // 2. Envío Síncrono del buffer empaquetado al Maestro (P0)
                MPI_Ssend(buffer_pack, packed_size, MPI_PACKED, 0, tag_migra, MPI_COMM_WORLD);
                          
                // 3. Recepción Síncrona del buffer empaquetado desde el Maestro
                MPI_Recv(buffer_recepcion, (int)buffer_size_bytes, MPI_PACKED, 0, tag_migra, MPI_COMM_WORLD, &status);
                         
                // 4. Desempaquetar en la zona de reemplazo (NEM peores)
                int count;
                MPI_Get_count(&status, MPI_PACKED, &count);
                position = 0; 
                
                Individuo *zona_reemplazo = &isla[tam_isla - nem]; 
                
                for (int i = 0; i < nem; i++) {
                    MPI_Unpack(buffer_recepcion, count, &position, &zona_reemplazo[i].fitness, 1, MPI_DOUBLE, MPI_COMM_WORLD);
                    MPI_Unpack(buffer_recepcion, count, &position, zona_reemplazo[i].array_int, L_MAX, MPI_INT, MPI_COMM_WORLD);
                }
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
            printf("Generacion %d, Hilo %d, Fitness = %.0lf\n", g, myrank, isla[0].fitness); 
        }
    }
    // FIN DEL BUCLE EVOLUTIVO
    // RECOLECCIÓN FINAL: P0 RECIBE EL MEJOR INDIVIDUO DE CADA ISLA Y DE ESOS LA SOLUCIÓN ES EL MEJOR
    double value;
    int tag_final = 30;   

    if (myrank == 0) {
        // buffer con los mejores individuos de cada isla
        Individuo *finalistas = (Individuo *)malloc(size * sizeof(Individuo));
        assert(finalistas);

        // P0 copia su propio mejor individuo 
        memcpy(&finalistas[0], &isla[0], sizeof(Individuo));

        // Recibe el mejor de cada esclavo
        MPI_Status status;
        for (source = 1; source < size; source++) {

            MPI_Recv(buffer_recepcion, (int)buffer_size_bytes, MPI_PACKED, source, tag_final, MPI_COMM_WORLD, &status);
            int count;
            MPI_Get_count(&status, MPI_PACKED, &count);
            position = 0; 

            // Desempaqueta directamente en la siguiente posición de 'finalistas'
            MPI_Unpack(buffer_recepcion, count, &position, &finalistas[source].fitness, 1, MPI_DOUBLE, MPI_COMM_WORLD);
            MPI_Unpack(buffer_recepcion, count, &position, finalistas[source].array_int, L_MAX, MPI_INT, MPI_COMM_WORLD);
        }

        // Ordena a todos los finalistas del buffer para encontrar el mejor
        qsort(finalistas, size, sizeof(Individuo), comp_fitness);

        // El mejor es finalistas[0]
        qsort(finalistas[0].array_int, m, sizeof(int), comp_array_int);
        memmove(sol, finalistas[0].array_int, m * sizeof(int));
        value = finalistas[0].fitness; 

        free(finalistas); 

    } else { // ESCLAVOS (myrank > 0)
        
        // Empaqueta su mejor individuo (isla[0])
        position = 0;
        MPI_Pack(&isla[0].fitness, 1, MPI_DOUBLE, buffer_pack, (int)buffer_size_bytes, &position, MPI_COMM_WORLD);
        MPI_Pack(isla[0].array_int, L_MAX, MPI_INT, buffer_pack, (int)buffer_size_bytes, &position, MPI_COMM_WORLD);
        int packed_size = position;

        // Envía su mejor individuo al Maestro (P0)
        MPI_Ssend(buffer_pack, packed_size, MPI_PACKED, 0, tag_final, MPI_COMM_WORLD);
        
        // Ignorado en main.c 
        value = isla[0].fitness;
    }

    // Liberación de memoria (todos los procesos)
    if (isla) free(isla);
    if (buffer_pack) free(buffer_pack);
    if (buffer_recepcion) free(buffer_recepcion);

    if (PRINT) {
        printf("Generaciones realizadas: %d\n", n_gen);
    }

    return value; 
}