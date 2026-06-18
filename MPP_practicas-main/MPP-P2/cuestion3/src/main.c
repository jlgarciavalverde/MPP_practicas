#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <mpi.h>
#include "../include/io.h"
#include "../include/mh.h"

// Firma externa adaptada para incluir nem, ngm, myrank, y size (11 argumentos)
extern double aplicar_mh(const double *, int, int, int, int, double, int, int, int *, int, int );

static double mseconds() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec*1000 + t.tv_usec/1000;
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

    // Distribucion de los argumentos (MPI_SSEND/MPI_RECV)
    int params_int[6];
    int num_int_params = 6;

    int tag_int = 1;    
    int tag_double = 2; 

    if (myrank == 0) { 
        // MAESTRO (P0): Carga los parámetros y envía
        params_int[0] = n; 
        params_int[1] = m;
        params_int[2] = n_gen; 
        params_int[3] = tam_pob;
        params_int[4] = nem; 
        params_int[5] = ngm;
        
        for (dest = 1; dest < size; dest++) {
            MPI_Ssend(params_int, num_int_params, MPI_INT, dest, tag_int, MPI_COMM_WORLD);
            MPI_Ssend(&m_rate, 1, MPI_DOUBLE, dest, tag_double, MPI_COMM_WORLD);
        }
    } else {
        // ESCLAVOS (myrank > 0): Reciben los parámetros del Maestro (P0).
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
        for (dest = 1; dest < size; dest++) {
            MPI_Ssend(d, tam_d, MPI_DOUBLE, dest, tag_d, MPI_COMM_WORLD);
        }
    } else {
        d = (double *) malloc(tam_d * sizeof(double));
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
    
    double value = aplicar_mh(d, n, m, n_gen, tam_pob, m_rate, nem, ngm, sol, myrank, size);
    
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

    // 3. FINALIZACIÓN DE MPI
    MPI_Finalize();
    return(EXIT_SUCCESS);
}