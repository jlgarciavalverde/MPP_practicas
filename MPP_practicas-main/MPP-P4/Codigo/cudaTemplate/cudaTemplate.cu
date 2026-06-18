#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// CUDA runtime
#include <cuda_runtime.h>

#include <timer.h>
#include <helper_string.h>

#include "helper_functions.h"  // helper for shared functions common to CUDA Samples
#include "helper_cuda.h"      // helper functions for CUDA error checking and initialization

#define SH_MEM_SIZE (48 * 1024) // 49152 bytes: ajustado para GT 1030
#define CT_MEM_SIZE (8)

#include "cudaTemplate_kernel.cu"
const int const_h[CT_MEM_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8};

////////////////////////////////
// Main Program
////////////////////////////////
int main(int argc, char *argv[])
{
    int dim_grid_x, dim_grid_y, dim_grid_z;    // MODIFICADO: grid dimensions (añadida Z)
    int dim_block_x, dim_block_y, dim_block_z; // MODIFICADO: block dimensions (añadida Z)
    
    int *gid_h = NULL; // host data
    int *gid_d = NULL; // device data
    
    long int nPos;
    size_t nBytes, shared_mem_size;
    
    float elapsed_time;
    cudaEvent_t start_event, stop_event;
    
    // process command line arguments
    dim_grid_x  = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "gsx");
    dim_grid_y  = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "gsy");
    // NUEVO: Lectura de Grid Z (si no existe, por defecto es 1)
    dim_grid_z  = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "gsz");
    if (dim_grid_z == 0) dim_grid_z = 1;

    dim_block_x = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "bsx");
    dim_block_y = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "bsy");
    // NUEVO: Lectura de Block Z (si no existe, por defecto es 1)
    dim_block_z = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "bsz");
    if (dim_block_z == 0) dim_block_z = 1;

    // MODIFICADO: Cálculo total de elementos incluyendo Z
    nPos = dim_grid_x * dim_grid_y * dim_grid_z * dim_block_x * dim_block_y * dim_block_z;
    nBytes = nPos * sizeof(int);

    // allocate host memory
    gid_h = (int *) malloc(nBytes);
    bzero(gid_h, nBytes);

    // Set the GPU to use
    checkCudaErrors(cudaSetDevice(0));
    
    // allocate device memory
    checkCudaErrors(cudaMalloc((void **) &gid_d, nBytes));
   
    // copy data from host memory to device memory
    checkCudaErrors(cudaMemcpy(gid_d, gid_h, nBytes, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemset((void *) gid_d, 0, nBytes));

    // initialize constant memory
    checkCudaErrors(cudaMemcpyToSymbol(const_d, const_h, CT_MEM_SIZE*sizeof(int), 0, cudaMemcpyHostToDevice));

    // create event
    checkCudaErrors(cudaEventCreate(&start_event));
    checkCudaErrors(cudaEventCreate(&stop_event));

    // using event
    cudaEventRecord(start_event, 0); // record in stream-0, to ensure that all previous CUDA calls have completed

    // setup execution parameters
    // MODIFICADO: dim3 ahora usa 3 dimensiones
    dim3 grid(dim_grid_x, dim_grid_y, dim_grid_z);
    dim3 block(dim_block_x, dim_block_y, dim_block_z);

    // execute the kernel
    // MODIFICADO: shared memory size incluye block.z
    shared_mem_size = block.x * block.y * block.z * sizeof(int);

    /*
     * Alternativa dinámica al tamaño de memoria compartida: obtener el límite de memoria compartida
     * del dispositivo en tiempo de ejecución y validar antes de lanzar el kernel.
     *
     * cudaDeviceProp deviceProp;
     * checkCudaErrors(cudaGetDeviceProperties(&deviceProp, 0));
     * size_t maxShared = deviceProp.sharedMemPerBlock;
     * if (shared_mem_size > maxShared) {
     *     fprintf(stderr, "Requested shared memory (%zu) exceeds device limit (%zu)\n", shared_mem_size, maxShared);
     *     exit(EXIT_FAILURE);
     * }
     */

    assert(shared_mem_size <= SH_MEM_SIZE);
    
    // MODIFICADO: Printf actualizado para mostrar Z
    printf("Running configuration: \t %ld threads\n\t\t\t grid of %d x %d x %d\n"
           "\t\t\t blocks of %d x %d x %d threads (%d threads with %lu bytes of shared memory per block)\n", 
           nPos, 
           dim_grid_x, dim_grid_y, dim_grid_z, 
           dim_block_x, dim_block_y, dim_block_z,
           dim_block_x * dim_block_y * dim_block_z, 
           shared_mem_size);
    
    foo<<<grid, block, shared_mem_size>>>(gid_d);

    // wait for thread completion
    cudaDeviceSynchronize();

    // get results back from device memory
    checkCudaErrors(cudaMemcpy(gid_h, gid_d, nBytes, cudaMemcpyDeviceToHost));
    
    // using event
    cudaEventRecord(stop_event, 0);    
    cudaEventSynchronize(stop_event);       // block until the event is recorded
    checkCudaErrors(cudaEventElapsedTime(&elapsed_time, start_event, stop_event));    
    printf("Processing Time: %.4f (ms)", elapsed_time);
    
    // check results
    for(int i = 0; i < nPos; i++) {
        // Nota: Asegúrate de que la lógica de verificación sigue siendo válida para tus datos
        assert(gid_h[i] += (i + const_h[i % CT_MEM_SIZE]));
    }
    
    // destroy events
    cudaEventDestroy(start_event);    
    cudaEventDestroy(stop_event);    
    
    // free device memory
    checkCudaErrors(cudaFree((void *) gid_d));
    
    // free host memory
    free(gid_h);
    
    printf("\nPASSED\n");
    cudaDeviceReset();
    return(0);
}


/*

#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// CUDA runtime
#include <cuda_runtime.h>

#include <timer.h>
#include <helper_string.h>

#include "helper_functions.h"  // helper for shared functions common to CUDA Samples
#include "helper_cuda.h"      // helper functions for CUDA error checking and initialization

#define SH_MEM_SIZE (16 * 1024)
#define CT_MEM_SIZE (8)

#include "cudaTemplate_kernel.cu"
const int const_h[CT_MEM_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8};

////////////////////////////////
// Main Program
////////////////////////////////
int main(int argc, char *argv[])
{
    int dim_grid_x, dim_grid_y;		// grid  dimensions
    int dim_block_x, dim_block_y;	// block dimensions
	
    int *gid_h = NULL; // host data
    int *gid_d = NULL; // device data
	
    long int nPos;
    size_t nBytes, shared_mem_size;
    
    float elapsed_time;
    cudaEvent_t start_event, stop_event;
	
    // process command line arguments
    dim_grid_x  = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "gsx");
    dim_grid_y  = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "gsy");
    dim_block_x = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "bsx");
    dim_block_y = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "bsy");

    nPos = dim_grid_x * dim_grid_y * dim_block_x * dim_block_y;
    nBytes = nPos * sizeof(int);

    // allocate host memory
    gid_h = (int *) malloc(nBytes);
    bzero(gid_h, nBytes);

	// Set the GPU to use
	checkCudaErrors(cudaSetDevice(0));
	
    // allocate device memory
    checkCudaErrors(cudaMalloc((void **) &gid_d, nBytes));
   
    // copy data from host memory to device memory
    checkCudaErrors(cudaMemcpy(gid_d, gid_h, nBytes, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemset((void *) gid_d, 0, nBytes));

    // initialize constant memory
    checkCudaErrors(cudaMemcpyToSymbol(const_d, const_h, CT_MEM_SIZE*sizeof(int), 0, cudaMemcpyHostToDevice));

    // create event
    checkCudaErrors(cudaEventCreate(&start_event));
    checkCudaErrors(cudaEventCreate(&stop_event));

    // using event
    cudaEventRecord(start_event, 0); // record in stream-0, to ensure that all previous CUDA calls have completed

    // setup execution parameters
    dim3 grid(dim_grid_x, dim_grid_y);
    dim3 block(dim_block_x, dim_block_y);

    // execute the kernel
    shared_mem_size = block.x * block.y * sizeof(int);
    assert(shared_mem_size <= SH_MEM_SIZE);
	
    printf("Running configuration: \t %ld threads\n\t\t\t grid of %d x %d\n"
           "\t\t\t blocks of %d x %d threads (%d threads with %lu bytes of shared memory per block)\n", 
           nPos, dim_grid_x, dim_grid_y, dim_block_x, dim_block_y, dim_block_x * dim_block_y, shared_mem_size);
    
    foo<<<grid, block, shared_mem_size>>>(gid_d);

    // wait for thread completion
    cudaDeviceSynchronize();

    // get results back from device memory
    checkCudaErrors(cudaMemcpy(gid_h, gid_d, nBytes, cudaMemcpyDeviceToHost));
    
    // using event
    cudaEventRecord(stop_event, 0);    
    cudaEventSynchronize(stop_event);		// block until the event is recorded
    checkCudaErrors(cudaEventElapsedTime(&elapsed_time, start_event, stop_event));    
    printf("Processing Time: %.4f (ms)", elapsed_time);
    
    // check results
    for(int i = 0; i < nPos; i++) {
        assert(gid_h[i] += (i + const_h[i % CT_MEM_SIZE]));
	}
	
    // destroy events
    cudaEventDestroy(start_event);    
    cudaEventDestroy(stop_event);    
    
    // free device memory
    checkCudaErrors(cudaFree((void *) gid_d));
	
    // free host memory
    free(gid_h);
	
    printf("\nPASSED\n");
    cudaDeviceReset();
    return(0);
}


*/