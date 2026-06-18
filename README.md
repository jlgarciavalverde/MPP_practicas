# MPP Prácticas — Modelos de Programación Paralela

Serie de prácticas de programación paralela en C que implementan y optimizan una **metaheurística evolutiva** (algoritmo genético) para resolver un problema de optimización combinatoria con distancias. Las prácticas progresan desde la versión secuencial hasta la paralelización con OpenMP y MPI.

## Tecnologías

![C](https://img.shields.io/badge/C-17-00599C?logo=c)
![OpenMP](https://img.shields.io/badge/OpenMP-4.5-blue)
![MPI](https://img.shields.io/badge/MPI-OpenMPI-brightgreen)
![GCC](https://img.shields.io/badge/Compiler-GCC-A42E2B?logo=gnu)

## Problema abordado

Dado un conjunto de `n` elementos con distancias entre ellos, se selecciona un subconjunto de `m` elementos que **maximiza la diversidad** (suma de distancias entre los seleccionados). Se resuelve mediante un algoritmo genético con:

- **Población** de `tam_pob` individuos
- **Generaciones** iterativas con cruce y mutación
- **Tasa de mutación** configurable (`m_rate`)

## Estructura de prácticas

| Práctica | Descripción |
|----------|-------------|
| `MPP-P0` | Implementación secuencial de referencia |
| `MPP-P1` | Paralelización con **OpenMP** (inicialización y evaluación fitness) |
| `MPP-P2` | Optimización OpenMP avanzada; análisis de speedup y eficiencia |
| `MPP-P3` | Paralelización con **MPI** (distribución de la población entre procesos) |
| `MPP-P4` | Híbrido MPI + OpenMP y análisis de rendimiento final |

Cada práctica incluye:
- Código fuente (`src/`)
- Resultados de ejecución comparativos (`resultados_sec_omp8/`)
- Memoria en PDF con análisis de speedup y eficiencia

## Compilación

```bash
cd MPP-P1/src

# Versión secuencial
gcc -O2 -o programa main.c io.c mh.c

# Con OpenMP
gcc -O2 -fopenmp -o programa main.c io.c mh.c

# Con MPI (P3+)
mpicc -O2 -o programa main.c io.c mh.c
```

## Ejecución

```bash
./programa <n> <m> <nGen> <tamPob> <m_rate> <n_hilos_ini> <n_hilo_fit>
```

| Parámetro | Descripción |
|-----------|-------------|
| `n` | Número total de elementos |
| `m` | Tamaño del subconjunto a seleccionar |
| `nGen` | Número de generaciones |
| `tamPob` | Tamaño de la población |
| `m_rate` | Tasa de mutación (0.0 – 1.0) |
| `n_hilos_ini` | Hilos OpenMP para inicialización |
| `n_hilo_fit` | Hilos OpenMP para evaluación de fitness |

### Ejemplo

```bash
./programa 1000 400 200 100 0.1 8 8
```

Los ficheros de entrada con las matrices de distancias están en `input/`.

## Resultados

Las carpetas `resultados_sec_omp8/` contienen comparativas entre la versión secuencial y la paralelizada con 8 hilos, con métricas de tiempo de ejecución, speedup y eficiencia.

## Autores

Proyecto académico — **Universidad de Murcia**, asignatura **Modelos de Programación Paralela (MPP)**.

- José Luis García Valverde
