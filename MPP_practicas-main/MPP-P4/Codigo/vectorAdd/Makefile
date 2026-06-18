GCC  = g++
NVCC = nvcc

CUDA_DIR = /usr/local/cuda
CUDA_LIB = -L$(CUDA_DIR)/lib64
CUDA_INC = -I$(CUDA_DIR)/include
CUDA_INC_COMMON = -I../Common

C_FLAGS = -O3
NVCC_FLAGS = -m64 -arch=sm_61
LD_LIBS	= -lcudart -lm
GENCODE_FLAGS = -Wno-deprecated-gpu-targets

FILE = 

$(FILE): $(FILE).o
		 $(GCC) $(C_FLAGS) $< -o $(FILE) $(CUDA_LIB) $(LD_LIBS)

$(FILE).o: $(FILE).cu
		   $(NVCC) $(NVCC_FLAGS) $(CUDA_INC) $(CUDA_INC_COMMON) $(GENCODE_FLAGS) -o $@ -c $<


run: 
	@echo "--- Experimento 1: 1 Millón de elementos (4 MB) ---"
	./$(FILE) --nelem=1000000 --tpb=32
	./$(FILE) --nelem=1000000 --tpb=128
	./$(FILE) --nelem=1000000 --tpb=256
	./$(FILE) --nelem=1000000 --tpb=512
	./$(FILE) --nelem=1000000 --tpb=1024

	@echo "\n--- Experimento 2: 10 Millones de elementos (40 MB) ---"
	./$(FILE) --nelem=10000000 --tpb=32
	./$(FILE) --nelem=10000000 --tpb=128
	./$(FILE) --nelem=10000000 --tpb=256
	./$(FILE) --nelem=10000000 --tpb=512
	./$(FILE) --nelem=10000000 --tpb=1024

	@echo "\n--- Experimento 3: 50 Millones de elementos (200 MB) ---"
	./$(FILE) --nelem=50000000 --tpb=32
	./$(FILE) --nelem=50000000 --tpb=256
	./$(FILE) --nelem=50000000 --tpb=1024

clean:
	rm -f *.o $(FILE)

