CC = gcc
CFLAGS = -Wall -O3 -std=c99 -g

omp: main.c mh.c io.c
	$(CC) $(CFLAGS) main.c mh.c io.c -lm -fopenmp -o omp -DTIMEGenPobIni

run_omp:
	./omp 12000 10000 0 8 0.0 1 1 >> ../output/out
	./omp 12000 10000 0 8 0.0 1 2 >> ../output/out
	./omp 12000 10000 0 8 0.0 1 4 >> ../output/out
	./omp 12000 10000 0 8 0.0 1 6 >> ../output/out
	./omp 12000 10000 0 8 0.0 2 1 >> ../output/out
	./omp 12000 10000 0 8 0.0 2 2 >> ../output/out
	./omp 12000 10000 0 8 0.0 2 4 >> ../output/out
	./omp 12000 10000 0 8 0.0 2 6 >> ../output/out
	./omp 12000 10000 0 8 0.0 4 1 >> ../output/out
	./omp 12000 10000 0 8 0.0 4 2 >> ../output/out
	./omp 12000 10000 0 8 0.0 4 4 >> ../output/out
	./omp 12000 10000 0 8 0.0 4 6 >> ../output/out
	./omp 12000 10000 0 8 0.0 6 1 >> ../output/out
	./omp 12000 10000 0 8 0.0 6 2 >> ../output/out
	./omp 12000 10000 0 8 0.0 6 4 >> ../output/out
	./omp 12000 10000 0 8 0.0 6 6 >> ../output/out

clean:
	rm -f omp




