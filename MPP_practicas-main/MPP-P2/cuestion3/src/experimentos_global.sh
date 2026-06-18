#!/bin/bash

# ============================
# Configuración de parámetros
# ============================

MRATE=0.15
REPS=2

P1=1000
P2=400
P3=200
P4=100
INPUT_FILE="../input/input_1000_400_200_100.txt"

# Carpeta global de resultados
OUTDIR_BASE="salida_optimo_global"

# Carpeta de resúmenes para el plot
SUMMARY_DIR_BASE="resumenes_optimo_global"

mkdir -p "${OUTDIR_BASE}"
mkdir -p "${SUMMARY_DIR_BASE}"

# ============================
# Configuraciones específicas (np, NEM, NGM)
# ============================
# np=2  → NEM=5,  NGM=5
# np=4  → NEM=5,  NGM=20
# np=8  → NEM=3,  NGM=50
CONFIGS=(
  "2 5 5"
  "4 3 10"
  "8 5 10"
)

# ============================
# Ejecución principal
# ============================

for CFG in "${CONFIGS[@]}"; do
  NP=$(echo "$CFG" | awk '{print $1}')
  NEM=$(echo "$CFG" | awk '{print $2}')
  NGM=$(echo "$CFG" | awk '{print $3}')

  CONFIG_DIR="${OUTDIR_BASE}/np${NP}_nem${NEM}_ngm${NGM}"
  mkdir -p "${CONFIG_DIR}"

  CONFIG_SUMMARY_DIR="${SUMMARY_DIR_BASE}/nem${NEM}_ngm${NGM}"
  mkdir -p "${CONFIG_SUMMARY_DIR}"
  SUMMARY_FILE="${CONFIG_SUMMARY_DIR}/resumen.txt"

  # Crear cabecera si no existe
  if [ ! -f "${SUMMARY_FILE}" ]; then
    echo "# NEM = ${NEM}, NGM = ${NGM}" > "${SUMMARY_FILE}"
    echo "np tiempo_medio_sec distance_media" >> "${SUMMARY_FILE}"
  fi

  echo "============================================="
  echo " Ejecutando configuración:"
  echo "   np=${NP}, NEM=${NEM}, NGM=${NGM}, mrate=${MRATE}"
  echo "   -> Carpeta runs: ${CONFIG_DIR}"
  echo "   -> Carpeta resumen: ${CONFIG_SUMMARY_DIR}"
  echo "============================================="

  # Ejecutar repeticiones
  for REP in $(seq 1 $REPS); do
    OUTFILE="${CONFIG_DIR}/run${REP}.txt"
    echo "  -> Ejecución ${REP}/${REPS}"
    mpirun -np "${NP}" --oversubscribe ./par \
        "${P1}" "${P2}" "${P3}" "${P4}" "${MRATE}" "${NEM}" "${NGM}" \
        < "${INPUT_FILE}" > "${OUTFILE}"
  done

  # Calcular medias
  MEDIA_TIEMPO=$(awk '/Execution Time:/ {sum+=$3; n++} END{if(n>0) printf "%.6f", sum/n; else print "NaN"}' "$CONFIG_DIR"/run*.txt)
  MEDIA_DISTANCE=$(awk '/Distance:/ {sum+=$2; n++} END{if(n>0) printf "%.6f", sum/n; else print "NaN"}' "$CONFIG_DIR"/run*.txt)

  echo "  -> Media tiempo: ${MEDIA_TIEMPO} sec"
  echo "  -> Media distance: ${MEDIA_DISTANCE}"

  # Guardar summary individual
  {
    echo "np = ${NP}"
    echo "NEM = ${NEM}"
    echo "NGM = ${NGM}"
    echo "mrate = ${MRATE}"
    echo "repeticiones = ${REPS}"
    echo "tiempo_medio_sec = ${MEDIA_TIEMPO}"
    echo "distance_media = ${MEDIA_DISTANCE}"
  } > "${CONFIG_DIR}/summary.txt"

  # Añadir fila al resumen para el plot
  echo "${NP} ${MEDIA_TIEMPO} ${MEDIA_DISTANCE}" >> "${SUMMARY_FILE}"

  echo "Resumen actualizado: ${SUMMARY_FILE}"
  echo
done
