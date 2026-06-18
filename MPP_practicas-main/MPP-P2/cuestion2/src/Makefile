CC = mpicc  # ⬅️ CORRECCIÓN: Usar el compilador wrapper de MPI
CFLAGS = -O3 -Wall -std=c99 -g

N = 
M = 
N_GEN = 
T_POB = 
M_RATE = 
NEM = 
NGM = 

EXEC = par  # ⬅️ CORRECCIÓN: Nombre del ejecutable paralelo
OUTFILE = ../output/out
C_FILES = main.c io.c mh.c

par: $(C_FILES)  # ⬅️ CORRECCIÓN: El objetivo es 'par'
	$(CC) $(CFLAGS) $(C_FILES) -o $(EXEC) -DTIME -DDEBUG

test_par:  # ⬅️ CORRECCIÓN: Objetivo para testear la versión paralela
	# NOTA: Para ejecutar la versión paralela, debes usar mpirun
	mpirun -np 4 ./$(EXEC) $(N) $(M) $(N_GEN) $(T_POB) $(M_RATE) $(NEM) $(NGM) >> $(OUTFILE)_$(N)_$(M)_$(N_GEN)_$(T_POB)_$(M_RATE)_$(NEM)_$(NGM).txt

clean:
	rm -f $(EXEC)