GCC  = g++
NVCC = nvcc

CUDA_DIR = /usr/local/cuda
CUDA_LIB = -L$(CUDA_DIR)/lib64
CUDA_INC = -I$(CUDA_DIR)/include
CUDA_INC_COMMON = -I../Common

C_FLAGS = -O3
NVCC_FLAGS = -m64 -arch=sm_61
LD_LIBS = -lcudart -lm
GENCODE_FLAGS = -Wno-deprecated-gpu-targets

FILE = 

$(FILE): $(FILE).o
	$(GCC) $(C_FLAGS) $< -o $(FILE) $(CUDA_LIB) $(LD_LIBS)

$(FILE).o: $(FILE).cu
	$(NVCC) $(NVCC_FLAGS) $(CUDA_INC) $(CUDA_INC_COMMON) $(GENCODE_FLAGS) -o $@ -c $<

# Pruebas originales (2D -> Z=1)
run: 
	@echo "--- RUN 2D: Compatibilidad (Z=1) ---"
	./$(FILE) --gsx=4 --gsy=4 --gsz=1 --bsx=2 --bsy=2 --bsz=1
	./$(FILE) --gsx=8 --gsy=8 --gsz=1 --bsx=2 --bsy=2 --bsz=1
	./$(FILE) --gsx=8 --gsy=8 --gsz=1 --bsx=4 --bsy=4 --bsz=1
	./$(FILE) --gsx=16 --gsy=16 --gsz=1 --bsx=2 --bsy=2 --bsz=1
	./$(FILE) --gsx=16 --gsy=16 --gsz=1 --bsx=4 --bsy=4 --bsz=1
	./$(FILE) --gsx=16 --gsy=16 --gsz=1 --bsx=8 --bsy=8 --bsz=1
	./$(FILE) --gsx=32 --gsy=32 --gsz=1 --bsx=2 --bsy=2 --bsz=1
	./$(FILE) --gsx=32 --gsy=32 --gsz=1 --bsx=4 --bsy=4 --bsz=1
	./$(FILE) --gsx=32 --gsy=32 --gsz=1 --bsx=8 --bsy=8 --bsz=1
	./$(FILE) --gsx=32 --gsy=32 --gsz=1 --bsx=16 --bsy=16 --bsz=1

# Pruebas nuevas (3D -> Z varia)
# Siguen la misma progresión de dificultad que el run normal
run3d:
	@echo "--- RUN 3D: Pruebas Volumétricas ---"
	@echo "1. Pequeño (Grid 4x4x2, Bloque 2x2x2)"
	./$(FILE) --gsx=4 --gsy=4 --gsz=2 --bsx=2 --bsy=2 --bsz=2
	
	@echo "2. Grid medio, Bloque pequeño (Grid 8x8x2, Bloque 2x2x2)"
	./$(FILE) --gsx=8 --gsy=8 --gsz=2 --bsx=2 --bsy=2 --bsz=2
	
	@echo "3. Grid medio, Bloque cúbico (Grid 8x8x4, Bloque 4x4x2)"
	./$(FILE) --gsx=8 --gsy=8 --gsz=4 --bsx=4 --bsy=4 --bsz=2
	
	@echo "4. Grid grande, Bloque plano (Grid 16x16x2, Bloque 2x2x2)"
	./$(FILE) --gsx=16 --gsy=16 --gsz=2 --bsx=2 --bsy=2 --bsz=2
	
	@echo "5. Grid grande, Bloque medio (Grid 16x16x4, Bloque 4x4x4)"
	./$(FILE) --gsx=16 --gsy=16 --gsz=4 --bsx=4 --bsy=4 --bsz=4
	
	@echo "6. Grid grande, Bloque grande (Grid 16x16x4, Bloque 8x8x2)"
	./$(FILE) --gsx=16 --gsy=16 --gsz=4 --bsx=8 --bsy=8 --bsz=2
	
	@echo "7. Grid muy grande, Bloque pequeño (Grid 32x32x2, Bloque 2x2x2)"
	./$(FILE) --gsx=32 --gsy=32 --gsz=2 --bsx=2 --bsy=2 --bsz=2
	
	@echo "8. Grid muy grande, Bloque medio (Grid 32x32x4, Bloque 4x4x4)"
	./$(FILE) --gsx=32 --gsy=32 --gsz=4 --bsx=4 --bsy=4 --bsz=4
	
	@echo "9. Carga alta (Grid 32x32x8, Bloque 8x8x4)"
	./$(FILE) --gsx=32 --gsy=32 --gsz=8 --bsx=8 --bsy=8 --bsz=4
	
	@echo "10. Carga Máxima (Grid 32x32x8, Bloque 16x8x4 - 512 hilos)"
	./$(FILE) --gsx=32 --gsy=32 --gsz=8 --bsx=16 --bsy=8 --bsz=4

clean:
	rm -f *.o $(FILE)