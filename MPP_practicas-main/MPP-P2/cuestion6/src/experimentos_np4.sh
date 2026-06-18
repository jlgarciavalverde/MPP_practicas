#!/bin/bash

# ============================
# Configuración de parámetros
# ============================

PROCS_LIST=(4)
MRATE=0.15
NEM_LIST=(3 5 10 12 25) 
NGM_LIST=(5 10 15 20 50)
REPS=2

P1=1000
P2=400
P3=200
P4=100
INPUT_FILE="../input/input_1000_400_200_100.txt"

OUTDIR="resultados_np4"
SUMMARY_DIR="resumenes_np4"

mkdir -p "$OUTDIR"
mkdir -p "$SUMMARY_DIR"

# ============================
# Ejecución principal
# ============================

for NEM in "${NEM_LIST[@]}"; do
  for NGM in "${NGM_LIST[@]}"; do

    # Crear carpeta para este conjunto de parámetros (sin np)
    CONFIG_SUMMARY_DIR="${SUMMARY_DIR}/nem${NEM}_ngm${NGM}"
    mkdir -p "$CONFIG_SUMMARY_DIR"
    SUMMARY_FILE="${CONFIG_SUMMARY_DIR}/resumen.txt"

    # Escribir cabecera del resumen
    echo "# NEM = ${NEM}, NGM = ${NGM}" > "$SUMMARY_FILE"
    echo "np tiempo_medio_sec distance_media" >> "$SUMMARY_FILE"

    for NP in "${PROCS_LIST[@]}"; do
      CONFIG_DIR="${OUTDIR}/np${NP}_nem${NEM}_ngm${NGM}"
      mkdir -p "$CONFIG_DIR"

      echo "============================================="
      echo " Ejecutando configuración:"
      echo "   np=${NP}, NEM=${NEM}, NGM=${NGM}, mrate=${MRATE}"
      echo "   -> Carpeta: ${CONFIG_DIR}"
      echo "============================================="

      # 1. Ejecutar repeticiones
      for REP in $(seq 1 $REPS); do
        OUTFILE="${CONFIG_DIR}/run${REP}.txt"
        echo "  -> Ejecución ${REP}/${REPS}"
        mpirun -np "${NP}" --oversubscribe ./par \
            "${P1}" "${P2}" "${P3}" "${P4}" "${MRATE}" "${NEM}" "${NGM}" \
            < "${INPUT_FILE}" > "${OUTFILE}"
      done

      # 2. Calcular medias con awk
      MEDIA_TIEMPO=$(awk '/Execution Time:/ {sum+=$3; n++} END{if(n>0) printf "%.6f", sum/n; else print "NaN"}' "$CONFIG_DIR"/run*.txt)
      MEDIA_DISTANCE=$(awk '/Distance:/ {sum+=$2; n++} END{if(n>0) printf "%.6f", sum/n; else print "NaN"}' "$CONFIG_DIR"/run*.txt)

      # 3. Guardar summary.txt individual
      {
        echo "np = ${NP}"
        echo "NEM = ${NEM}"
        echo "NGM = ${NGM}"
        echo "mrate = ${MRATE}"
        echo "repeticiones = ${REPS}"
        echo "tiempo_medio_sec = ${MEDIA_TIEMPO}"
        echo "distance_media = ${MEDIA_DISTANCE}"
      } > "${CONFIG_DIR}/summary.txt"

      echo "  -> Media tiempo: ${MEDIA_TIEMPO} sec"
      echo "  -> Media distance: ${MEDIA_DISTANCE}"

      # 4. Añadir fila al resumen de esta configuración (para gráfica)
      echo "${NP} ${MEDIA_TIEMPO} ${MEDIA_DISTANCE}" >> "$SUMMARY_FILE"

    done

    echo "Resumen generado: ${SUMMARY_FILE}"
    echo

  done
done
