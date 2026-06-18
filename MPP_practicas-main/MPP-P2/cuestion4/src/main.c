#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <mpi.h>
#include <stddef.h> // para 'offsetof'
#include "../include/io.h"
#include "../include/mh.h"

extern double aplicar_mh(const double *, int, int, int, int, double, int, int, int *, int, int, MPI_Datatype);

static double mseconds() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec*1000 + t.tv_usec/1000;
}

/**
 * Crea un nuevo tipo de datos derivado en MPI
 * Necesario para el envio/recepcion de mensajes con datos de tipo Individuo
 **/
void crear_tipo_datos(MPI_Datatype *individuo_type)
{
    // Definir los 2 bloques: {1 double, L_MAX ints}
    int blocklen[2] = {1, L_MAX};
    
    // Definir los tipos: {MPI_DOUBLE, MPI_INT} (Fitness primero, array segundo)
    MPI_Datatype dtype[2] = { MPI_DOUBLE, MPI_INT };
    
    // Definir dónde empieza cada bloque (desplazamiento/offset)
    MPI_Aint disp[2];
    disp[0] = offsetof(Individuo, fitness);
    disp[1] = offsetof(Individuo, array_int);
    
    MPI_Type_create_struct(2, blocklen, disp, dtype, individuo_type); 
    MPI_Type_commit(individuo_type);
}


int main(int argc, char **argv)
{
    // Declaracion variables MPI
    int myrank, size;
    int dest, source = 0;
    MPI_Status status;
    
    // INICIALIZACIÓN DE MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    MPI_Datatype mpi_tipo_individuo;
    crear_tipo_datos(&mpi_tipo_individuo);

    int n, m, n_gen, tam_pob, nem, ngm;
    double m_rate;

    // P0 lee los argumentos pasados desde la terminal
    if (myrank == 0) { 

        if(argc < 8) { 
            fprintf(stderr,"Ayuda:\n"); 
            fprintf(stderr,"  ./programa n m nGen tamPob m_rate nem ngm\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        n_gen = atoi(argv[3]);
        tam_pob = atoi(argv[4]);
        m_rate = atof(argv[5]);
        nem = atoi(argv[6]); 
        ngm = atoi(argv[7]);
        
        // Validaciones
        if (m >= n) {
            fprintf(stderr, "Error: 'm' debe ser menor que 'n'.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        if (m > L_MAX) { 
            fprintf(stderr, "Error: m=%d excede L_MAX=%d\n", m, L_MAX);
            MPI_Abort(MPI_COMM_WORLD, 1); 
        }
    }

    // Distribucion de los argumentos
    int params_int[6];
    int num_int_params = 6;

    int tag_int = 1;    
    int tag_double = 2; 

    if (myrank == 0) { 
        params_int[0] = n; 
        params_int[1] = m;
        params_int[2] = n_gen; 
        params_int[3] = tam_pob;
        params_int[4] = nem; 
        params_int[5] = ngm;
        
        // Se necesitan 2 envíos por esclavo (int y double)
        MPI_Request *requests = (MPI_Request *)calloc(2 * (size - 1), sizeof(MPI_Request));
        assert(requests);
        int req_count = 0;

        for (dest = 1; dest < size; dest++) {
            MPI_Isend(params_int, num_int_params, MPI_INT, dest, tag_int, MPI_COMM_WORLD, &requests[req_count++]);
            MPI_Isend(&m_rate, 1, MPI_DOUBLE, dest, tag_double, MPI_COMM_WORLD, &requests[req_count++]);
        }
        // Esperamos a que todos los envíos no bloqueantes terminen
        MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);
        free(requests);

    } else {
        // La recepción es bloqueante: el esclavo no puede hacer nada sin los params
        MPI_Recv(params_int, num_int_params, MPI_INT, source, tag_int, MPI_COMM_WORLD, &status);
        MPI_Recv(&m_rate, 1, MPI_DOUBLE, source, tag_double, MPI_COMM_WORLD, &status);
                
        n = params_int[0]; 
        m = params_int[1];
        n_gen = params_int[2]; 
        tam_pob = params_int[3];
        nem = params_int[4]; 
        ngm = params_int[5];
    }

    // P0 lee y envía la matriz d al resto de procesos
    double *d = NULL; 
    int tam_d = (n * n - n) / 2; 
    int tag_d = 20; 
    
    if (myrank == 0) {
        d = read_distances(n); 
        
        MPI_Request *d_requests = (MPI_Request *)calloc(size - 1, sizeof(MPI_Request));
        assert(d_requests);
        
        for (dest = 1; dest < size; dest++) {
            MPI_Isend(d, tam_d, MPI_DOUBLE, dest, tag_d, MPI_COMM_WORLD, &d_requests[dest-1]);
        }
        MPI_Waitall(size - 1, d_requests, MPI_STATUSES_IGNORE);
        free(d_requests);

    } else {
        d = (double *) malloc(tam_d * sizeof(double));
        // Recepción bloqueante
        MPI_Recv(d, tam_d, MPI_DOUBLE, source, tag_d, MPI_COMM_WORLD, &status);      
    }
    
    int *sol = NULL;
    if (myrank == 0) { 
        sol = (int *) malloc(m * sizeof(int)); 
    }
    
    #ifdef TIME
        double ti = 0.0;
        if (myrank == 0) { ti = mseconds(); }
        MPI_Barrier(MPI_COMM_WORLD); 
    #endif
    
    double value = aplicar_mh(d, n, m, n_gen, tam_pob, m_rate, nem, ngm, sol, myrank, size, mpi_tipo_individuo);
    
    #ifdef TIME
        MPI_Barrier(MPI_COMM_WORLD); 
        if (myrank == 0) {
            double tf = mseconds();
            printf("Execution Time: %.2lf sec\n", (tf - ti)/1000);
        }
    #endif
    
    #ifdef DEBUG
        if (myrank == 0) {
            print_solution(n, m, sol, value); 
        }
    #endif

    // Liberación de memoria
    if (myrank == 0) { free(sol); }
    if (d != NULL) free(d); 
    MPI_Type_free(&mpi_tipo_individuo);

    MPI_Finalize();
    return(EXIT_SUCCESS);
}