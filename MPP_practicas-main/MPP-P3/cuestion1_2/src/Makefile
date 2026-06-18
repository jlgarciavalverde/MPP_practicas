CC = mpicc 
CFLAGS = -O3 -Wall -std=c99 -g -D_POSIX_C_SOURCE=200112L -fopenmp

EXEC = par  
OUTFILE = ../output/out
C_FILES = main.c io.c mh.c 

par: $(C_FILES) 
	$(CC) $(CFLAGS) $(C_FILES) -o $(EXEC) -DTIME -DDEBUG

test_par:  
	OMP_NUM_THREADS=8 mpirun -np 4 ./$(EXEC) 1000 400 200 100 0.15 5 20 < ../input/input_1000_400_200_100.txt

clean:
	rm -f $(EXEC)