CC = mpicc
CFLAGS = -O3 -std=c99 -g -Wall

mpi: main.c mh.c io.c 
	$(CC) $(CFLAGS) main.c mh.c io.c -lm -o mpi -DTIME

run_mpi:
	mpirun -np 2 ./mpi 5000 1000 1 500 0.0 1 10 >> ../output/out
	mpirun -np 4 ./mpi 5000 1000 1 500 0.0 1 10 >> ../output/out
	mpirun --oversubscribe -np 6 ./mpi 5000 1000 1 500 0.0 1 10 >> ../output/out
	mpirun --oversubscribe -np 12 ./mpi 5000 1000 1 500 0.0 1 10 >> ../output/out
	mpirun --oversubscribe -np 18 ./mpi 5000 1000 1 500 0.0 1 10 >> ../output/out
	mpirun --oversubscribe -np 24 ./mpi 5000 1000 1 500 0.0 1 10 >> ../output/out
	mpirun --oversubscribe -np 30 ./mpi 5000 1000 1 500 0.0 1 10 >> ../output/out

clean:
	rm -f mpi
