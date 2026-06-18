#ifndef _MH
#define _MH
#define L_MAX 4096 // MPI necesita un tamaño fijo de Individuo

	typedef struct {
		double fitness;
		int array_int[L_MAX];
	} Individuo;
	
	void cruzar(Individuo *, Individuo *, Individuo *, Individuo *, int, int);
	void mutar(Individuo *, int, int, double);
	void fitness(const double *, Individuo *, int, int);
	double aplicar_mh(const double *, int, int, int, int, double, int, int, int *, int, int);
#endif
