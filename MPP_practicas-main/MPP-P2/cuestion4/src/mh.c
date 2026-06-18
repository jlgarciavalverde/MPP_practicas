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
    const Individuo *ia = (const Individuo *)a;
    const Individuo *ib = (const Individuo *)b;
    // Orden descendente (mejor primero):
    if (ib->fitness > ia->fitness) return 1;
    if (ib->fitness < ia->fitness) return -1;
    return 0;
}

// ------------------------- FACTIBILIZAR -------------------------

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

void mutar(Individuo *actual, int n, int m, double m_rate)
{
    int num_mutaciones = (int)(m_rate * m);

    for (int i = 0; i < num_mutaciones; i++)
    {
        int pos = aleatorio(m);
        int nuevo_valor = aleatorio(n); // // genera un número aleatorio entre 0 y n-1

        int intentos = 0;
        while (find_element(actual->array_int, m, nuevo_valor) && !(intentos > n*2)) {
            nuevo_valor = aleatorio(n);
            intentos++;
        }
        if (!find_element(actual->array_int, m, nuevo_valor)) { // procedemos a la mutación
            actual->array_int[pos] = nuevo_valor; 
        }
    }
}

double distancia_ij(const double *d, int i, int j, int n)
{
    if (i == j)
        return 0.0; // distancia nula en la diagonal
    if (i > j)
    {
        int tmp = i;
        i = j;
        j = tmp;
    }
    int a = (n * n - n) / 2;
    int b = ((n - i) * (n - i) - (n - i)) / 2;
    int k = a - b + (j - i - 1);

    return d[k];
}

// ------------------------- FITNESS -------------------------

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

double aplicar_mh(const double *d, int n, int m, int n_gen, int tam_pob, double m_rate, int nem, int ngm, int *sol, int myrank, int size, MPI_Datatype mpi_tipo_individuo)
{
    srand(time(NULL) + getpid());

    int dest, source = 0;
    int tag_init = 10;
    int tag_migra = 20; 
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

    /********************************* INICIALIZACIÓN DE LA POBLACIÓN Y REPARTO (ASÍNCRONO) ************************************/
    if (myrank == 0) { 
        Individuo *poblacion_inicial = (Individuo *)malloc(tam_pob * sizeof(Individuo)); 
        assert(poblacion_inicial);
        
        for (int i = 0; i < tam_pob; i++) {
            int *tmp_array = crear_individuo(n, m); 
            memcpy(poblacion_inicial[i].array_int, tmp_array, (size_t)m * sizeof(int));
            free(tmp_array);
            fitness(d, &poblacion_inicial[i], n, m);
        }
        
        memcpy(isla, poblacion_inicial, tam_isla * sizeof(Individuo)); 

        MPI_Request *init_requests = (MPI_Request *)calloc(size - 1, sizeof(MPI_Request));
        assert(init_requests);
        
        for (dest = 1; dest < size; dest++) { 
            Individuo *bloque_a_enviar = poblacion_inicial + tam_isla + ((dest - 1) * tam_isla_esclavo);
            MPI_Isend(bloque_a_enviar, tam_isla_esclavo, mpi_tipo_individuo, dest, tag_init, MPI_COMM_WORLD, &init_requests[dest-1]);
        }

        MPI_Waitall(size - 1, init_requests, MPI_STATUSES_IGNORE);
        
        free(init_requests);
        free(poblacion_inicial); 
        
    } else { // ESCLAVOS (myrank > 0)
        // Bloqueante
        MPI_Recv(isla, tam_isla, mpi_tipo_individuo, source, tag_init, MPI_COMM_WORLD, &status);
    }
    /********************************* FIN INICIALIZACIÓN DE LA POBLACIÓN Y REPARTO ************************************/
        
    qsort(isla, tam_isla, sizeof(Individuo), comp_fitness); 

    MPI_Request send_request = MPI_REQUEST_NULL; // para que los esclavos envíen en la migración
    MPI_Request recv_request = MPI_REQUEST_NULL; // para que los esclavos reciban en la migración
    MPI_Request *send_requests = NULL; 
    MPI_Request *recv_requests = NULL; 
    bool mig_requests_pending = false; // indica si los esclavos tienen comunicaciones pendientes
    bool master_requests_pending = false; // indica si el maestro tiene peticiones pendientes
    Individuo *buffer_mezcla = NULL;     

    if (myrank == 0) {
        int total_migrantes = nem * size;
        buffer_mezcla = (Individuo *)malloc(total_migrantes * sizeof(Individuo));
        assert(buffer_mezcla);
        
        send_requests = (MPI_Request *)calloc(size - 1, sizeof(MPI_Request)); 
        recv_requests = (MPI_Request *)calloc(size - 1, sizeof(MPI_Request));
        assert(send_requests);
        assert(recv_requests);
    }

    // BUCLE EVOLUTIVO
    for (int g = 0; g < n_gen; g++)
    { 
        if (g > 0 && (g % ngm == 0)) 
        {
            if (myrank == 0) {
                // MAESTRO (P0)
                // Esperar a que finalicen comunicaciones de la migración anterior
                if (master_requests_pending) { // No entra en la primera migración
                    MPI_Waitall(size - 1, recv_requests, MPI_STATUSES_IGNORE); // nem individuos de cada esclavo recibidos de la migración anterior
                    MPI_Waitall(size - 1, send_requests, MPI_STATUSES_IGNORE); // Nos aseguramos de que el maestro haya enviado a todos los esclavos en la migración anterior
                }

                // Procesar datos recibidos
                // En la 1era migración p0 envía sus nem mejores individuos a los esclavos, no los nem mejores globales
                memcpy(buffer_mezcla, isla, nem * sizeof(Individuo)); 
                qsort(buffer_mezcla, nem * size, sizeof(Individuo), comp_fitness);

                // Iniciar envíos para esta migración
                for (dest = 1; dest < size; dest++) { 
                    MPI_Isend(buffer_mezcla, nem, mpi_tipo_individuo, dest, tag_migra, MPI_COMM_WORLD, &send_requests[dest-1]);
                } 

                // Iniciar recepciones para la siguiente migración
                for (int i = 0; i < size - 1; i++) {
                    MPI_Irecv(&buffer_mezcla[(i+1) * nem], nem, mpi_tipo_individuo, MPI_ANY_SOURCE, tag_migra, MPI_COMM_WORLD, &recv_requests[i]);
                }

                master_requests_pending = true;

                // Actualizar isla local
                memcpy(&isla[tam_isla - nem], buffer_mezcla, nem * sizeof(Individuo));
                qsort(isla, tam_isla, sizeof(Individuo), comp_fitness);

            } else { // ESCLAVOS (myrank > 0)
                
                // Esperar a que finalicen comunicaciones de la migración anterior
                if (mig_requests_pending) { // No entra en la primera migración
                    MPI_Wait(&send_request, MPI_STATUS_IGNORE); // Asegurar que los nem individuos de la migración anterior llegaron al maestro 
                    MPI_Wait(&recv_request, MPI_STATUS_IGNORE); // Asegurar que el esclavo recibió del maestro en la migración anterior
                }
                
                // Actualizar isla local con datos recibidos
                qsort(isla, tam_isla, sizeof(Individuo), comp_fitness);

                // Iniciar envío para esta ronda
                MPI_Isend(isla, nem, mpi_tipo_individuo, 0, tag_migra, MPI_COMM_WORLD, &send_request);
                
                // Iniciar recepción para la próxima ronda
                // Funciona como un buzón
                MPI_Irecv(&isla[tam_isla - nem], nem, mpi_tipo_individuo, 0, tag_migra, MPI_COMM_WORLD, &recv_request);

                mig_requests_pending = true; 
            }
        }
        
        for (int i = 0; i < (tam_isla / 2) - 1; i += 2) 
        {
            cruzar(&isla[i], &isla[i + 1], &isla[tam_isla / 2 + i], &isla[tam_isla / 2 + i + 1], n, m); 
        }

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
    
    // Cierra las comunicaciones pendientes de la última migración
    if (myrank == 0) {
        MPI_Waitall(size - 1, recv_requests, MPI_STATUSES_IGNORE);
        MPI_Waitall(size - 1, send_requests, MPI_STATUSES_IGNORE);

        free(buffer_mezcla);
        free(send_requests);
        free(recv_requests);
    } else { // Esclavo
        MPI_Wait(&send_request, MPI_STATUS_IGNORE);
        MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
    }

    // Despues de recibir todo lo pendiente, reordenar la isla final
    qsort(isla, tam_isla, sizeof(Individuo), comp_fitness); 

    // RECOLECCIÓN FINAL ASÍNCRONA
    double value;
    int tag_final = 30;   

    if (myrank == 0) {
        Individuo *finalistas = (Individuo *)malloc(size * sizeof(Individuo));
        assert(finalistas);
        memcpy(&finalistas[0], &isla[0], sizeof(Individuo));

        MPI_Request *final_reqs = (MPI_Request *)calloc(size - 1, sizeof(MPI_Request));
        assert(final_reqs);
        
        for (int i = 1; i < size; i++) {
            MPI_Irecv(&finalistas[i], 1, mpi_tipo_individuo, MPI_ANY_SOURCE, tag_final, MPI_COMM_WORLD, &final_reqs[i-1]);
        }
        MPI_Waitall(size - 1, final_reqs, MPI_STATUSES_IGNORE);
        free(final_reqs);

        qsort(finalistas, size, sizeof(Individuo), comp_fitness);

        qsort(finalistas[0].array_int, m, sizeof(int), comp_array_int);
        memmove(sol, finalistas[0].array_int, m * sizeof(int));
        value = finalistas[0].fitness; 

        free(finalistas); 

    } else { 
        MPI_Request final_send_req;
        MPI_Isend(&isla[0], 1, mpi_tipo_individuo, 0, tag_final, MPI_COMM_WORLD, &final_send_req);
        MPI_Wait(&final_send_req, MPI_STATUS_IGNORE); 
        
        value = isla[0].fitness; 
    }

    if (isla) free(isla);

    if (PRINT) {
        printf("Generaciones realizadas: %d\n", n_gen);
    }

    return value; 
}