#!/bin/bash

# ==========================================================
# CONFIGURACIÓN DEL EXPERIMENTO
# ==========================================================

# Parámetros del Problema (FIJOS)
PARAMS="1000 400 200 100 0.15 5 20"
INPUT_FILE="../input/input_1000_400_200_100.txt"
EXECUTABLE="./par"

# Parámetros de Paralelismo (VARIABLES)
N_PROCS=(4)      # Número de procesos MPI
N_HILOS_PASOS=(10 12 14 16 20)  # Número de hilos OpenMP por proceso MPI

OUTPUT_FILE="../output/resultados_hibridoo.dat"
N_REPETICIONES=3 # Número de veces a ejecutar para el promedio

# ==========================================================
# EJECUCIÓN
# ==========================================================

# Encabezado formateado
printf "# %-5s %-5s %-12s %-18s %s\n" "P" "H" "T_EJEC_AVG" "FITNESS_AVG" "EFICIENCIA_I" > $OUTPUT_FILE
echo "Iniciando pruebas híbridas..."

# Bucle sobre el número de procesos MPI
for N_PROC in "${N_PROCS[@]}"; do
    
    # Determinar si es necesario el flag --oversubscribe
    OVER_FLAG=""
    if [ "$N_PROC" -gt 4 ]; then
        OVER_FLAG="--oversubscribe"
    fi

    # Bucle sobre el número de hilos OpenMP
    for N_HILOS in "${N_HILOS_PASOS[@]}"; do
        
        TIEMPOS_SUMA=0.0
        FITNESS_SUMA=0.0
        
        echo -n "  Probando P=$N_PROC, H=$N_HILOS... "
        
        # Bucle para realizar N_REPETICIONES y calcular la media
        for i in $(seq 1 $N_REPETICIONES); do
            
            # Ejecución de la versión híbrida: AÑADIENDO $OVER_FLAG
            OUTPUT=$(OMP_NUM_THREADS=$N_HILOS mpirun -np $N_PROC $OVER_FLAG $EXECUTABLE $PARAMS < $INPUT_FILE 2>&1)
            
            # Captura y limpieza del tiempo (Execution Time: X.XX sec)
            TIME_RAW=$(echo "$OUTPUT" | grep "Execution Time" | awk '{print $3}')
            
            # Captura y limpieza del fitness (Distance: X.XX)
            FITNESS_RAW=$(echo "$OUTPUT" | grep "Distance:" | awk '{print $2}')
            
            # Suma de resultados (usando 'bc' para aritmética de punto flotante)
            if [[ "$TIME_RAW" =~ ^[0-9]+(\.[0-9]+)?$ ]] && [[ "$FITNESS_RAW" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
                TIEMPOS_SUMA=$(echo "scale=4; $TIEMPOS_SUMA + $TIME_RAW" | bc)
                FITNESS_SUMA=$(echo "scale=4; $FITNESS_SUMA + $FITNESS_RAW" | bc)
            else
                echo "ERROR: Captura fallida. Tiempo: $TIME_RAW, Fitness: $FITNESS_RAW. Saltando."
                TIEMPOS_SUMA=0.0
                FITNESS_SUMA=0.0
                break # Sale del bucle de repeticiones si hay error
            fi
        done
        
        # Calcula promedios
        if [ "$(echo "$TIEMPOS_SUMA > 0" | bc)" -eq 1 ]; then
            T_AVG=$(echo "scale=4; $TIEMPOS_SUMA / $N_REPETICIONES" | bc)
            F_AVG=$(echo "scale=4; $FITNESS_SUMA / $N_REPETICIONES" | bc)
            
            # CALCULO DE LA EFICIENCIA (Fitness / Tiempo)
            EFICIENCIA_I=$(echo "scale=6; $F_AVG / $T_AVG" | bc)

            # Guarda los resultados usando printf para el formato alineado
            printf "%-5s %-5s %-12s %-18s %s\n" "$N_PROC" "$N_HILOS" "$T_AVG" "$F_AVG" "$EFICIENCIA_I" >> $OUTPUT_FILE
            echo "Media T: $T_AVG s, Media F: $F_AVG, Eficiencia I: $EFICIENCIA_I"
        else
            echo "Error en la ejecución de la serie."
        fi
    done
done

echo "Pruebas finalizadas. Resultados guardados en $OUTPUT_FILE."