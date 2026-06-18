#include <stdlib.h>   // malloc, free
#include <assert.h>   // assert
#include <string.h>   // memmove
#include "../include/mh.h"   // definición de Individuo


void mezclar(Individuo **poblacion, int izq, int med, int der)
{	
	int i, j, k;
	
	// Reservamos memoria temporal para los punteros
	Individuo **pob = (Individuo **) malloc((der - izq) * sizeof(Individuo *));
	assert(pob);
	
	for (i = 0; i < (der - izq); i++) {
		pob[i] = NULL; // Inicializamos a NULL por seguridad
	}
	
	k = 0;
	i = izq;
	j = med;

	// Mezclamos ambas mitades ya ordenadas
	while ((i < med) && (j < der)) {
		if (poblacion[i]->fitness > poblacion[j]->fitness) { // > porque queremos maximizar la distancia, no minimizar
			// copiar poblacion[i++] en pob[k++]
			memmove(&pob[k++], &poblacion[i++], sizeof(Individuo *));
		}
		else {
			// copiar poblacion[j++] en pob[k++]
			memmove(&pob[k++], &poblacion[j++], sizeof(Individuo *));
		}
	}
	
	// Copiamos lo que quede de la primera mitad
	for (; i < med; i++) {
		memmove(&pob[k++], &poblacion[i], sizeof(Individuo *));
	}
	
	// Copiamos lo que quede de la segunda mitad
	for (; j < der; j++) {
		memmove(&pob[k++], &poblacion[j], sizeof(Individuo *));
	}
	
	// Copiamos de vuelta al array original
	i = 0;
	while (i < (der - izq)) {
		memmove(&poblacion[i + izq], &pob[i], sizeof(Individuo *));
		i++;
	}
	
	// Liberamos el buffer temporal
	free(pob);
}
