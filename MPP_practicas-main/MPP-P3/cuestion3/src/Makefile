CC = gcc
CFLAGS = -Wall -O3 -std=c99 -g

omp: main.c mh.c io.c
	$(CC) $(CFLAGS) main.c mh.c io.c -lm -fopenmp -o omp -DTIMEGenPobIni

run_omp:
	./omp 12000 10000 0 10 0.0 1 1 >> ../output/out
	./omp 12000 10000 0 10 0.0 2 1 >> ../output/out
	./omp 12000 10000 0 10 0.0 4 1 >> ../output/out
	./omp 12000 10000 0 10 0.0 6 1 >> ../output/out
	./omp 12000 10000 0 10 0.0 8 1 >> ../output/out
	./omp 12000 10000 0 10 0.0 10 1 >> ../output/out
	./omp 12000 10000 0 10 0.0 12 1 >> ../output/out
	./omp 12000 10000 0 10 0.0 14 1 >> ../output/out
	./omp 12000 10000 0 10 0.0 16 1 >> ../output/out
	./omp 12000 10000 0 10 0.0 18 1 >> ../output/out
	./omp 12000 10000 0 10 0.0 20 1 >> ../output/out

clean:
	rm -f omp




