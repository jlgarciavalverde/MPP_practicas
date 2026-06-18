////////////////////////////////////////////////////////////////////////////////
// CUDA Kernel
////////////////////////////////////////////////////////////////////////////////

__constant__ int const_d[CT_MEM_SIZE];

__global__ void foo(int *gid_d)
{
    extern __shared__ int shared_mem[];

    // 1. Tamaño del bloque (total threads en el bloque)
    // Antes: int blockSize = blockDim.x * blockDim.y;
    int blockSize = blockDim.x * blockDim.y * blockDim.z;

    // 2. ID local del thread (aplanando 3D a 1D)
    // Antes: int tidb = (blockDim.x * threadIdx.y + threadIdx.x);
    // Fórmula estándar: z * (dim_y * dim_x) + y * dim_x + x
    int tidb = threadIdx.z * (blockDim.x * blockDim.y) + 
               threadIdx.y * blockDim.x + 
               threadIdx.x;

    // 3. ID global del thread
    // Primero calculamos el ID global del bloque
    int blockId = blockIdx.z * (gridDim.x * gridDim.y) + 
                  blockIdx.y * gridDim.x + 
                  blockIdx.x;
                  
    // Luego el ID global del thread
    int tidg = blockId * blockSize + tidb;

    // El resto del código permanece igual
    shared_mem[tidb] = gid_d[tidg];
    
    // __syncthreads(); // Opcional según respuesta (c)

    shared_mem[tidb] += (tidg + const_d[tidg % CT_MEM_SIZE]);

    // __syncthreads(); // Opcional según respuesta (c)

    gid_d[tidg] = shared_mem[tidb];
}

/*

////////////////////////////////////////////////////////////////////////////////
// CUDA Kernel
////////////////////////////////////////////////////////////////////////////////

__constant__ int const_d[CT_MEM_SIZE];

__global__ void foo(int *gid_d)
{
    extern __shared__ int shared_mem[];

	// size of the block
    int blockSize = blockDim.x * blockDim.y;

    // global thread ID in thread block
    int tidb = (blockDim.x * threadIdx.y + threadIdx.x);

    // global thread ID in grid
    int tidg = (blockIdx.y * gridDim.x * blockSize + blockIdx.x * blockSize + tidb);

    shared_mem[tidb] = gid_d[tidg];
    
    __syncthreads();

	// shared memory 
    shared_mem[tidb] += (tidg + const_d[tidg % CT_MEM_SIZE]);

    __syncthreads();

    gid_d[tidg] = shared_mem[tidb];
} 

*/