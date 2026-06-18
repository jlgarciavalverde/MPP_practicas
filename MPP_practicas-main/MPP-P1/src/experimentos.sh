#!/bin/bash

# ============================
# Configuración
# ============================
THREADS=8
REPS=2

P1=1000
P2=400
P3=200
P4=100
MRATE=0.15
INPUT_FILE="input_1000_400_200_100.txt"  

OUTDIR="resultados_sec_omp${THREADS}"
SUMMARY_DIR="resumenes_sec_omp${THREADS}"

mkdir -p "$OUTDIR"
mkdir -p "$SUMMARY_DIR"

SUMMARY_FILE="${SUMMARY_DIR}/resumen.txt"

echo "# OpenMP solo. np=hilos (OMP_NUM_THREADS)" > "$SUMMARY_FILE"
echo "np tiempo_medio_sec distance_media" >> "$SUMMARY_FILE"

echo "============================================="
echo " Ejecutando SECUENCIAL OpenMP con ${THREADS} hilos"
echo " Comando: OMP_NUM_THREADS=${THREADS} ./sec ${P1} ${P2} ${P3} ${P4} ${MRATE} < ${INPUT_FILE}"
echo " Repeticiones: ${REPS}"
echo "============================================="

# Repeticiones
for REP in $(seq 1 $REPS); do
  OUTFILE="${OUTDIR}/run${REP}.txt"
  echo "  -> Ejecución ${REP}/${REPS}"
  OMP_NUM_THREADS=${THREADS} ./sec "${P1}" "${P2}" "${P3}" "${P4}" "${MRATE}" \
      < "${INPUT_FILE}" > "${OUTFILE}"
done

# Medias
MEDIA_TIEMPO=$(awk '/Execution Time:/ {sum+=$3; n++} END{if(n>0) printf "%.6f", sum/n; else print "NaN"}' "$OUTDIR"/run*.txt)
MEDIA_DISTANCE=$(awk '/Distance:/ {sum+=$2; n++} END{if(n>0) printf "%.6f", sum/n; else print "NaN"}' "$OUTDIR"/run*.txt)

# summary.txt individual
{
  echo "np = ${THREADS} (hilos OpenMP)"
  echo "mrate = ${MRATE}"
  echo "repeticiones = ${REPS}"
  echo "tiempo_medio_sec = ${MEDIA_TIEMPO}"
  echo "distance_media = ${MEDIA_DISTANCE}"
} > "${OUTDIR}/summary.txt"

echo "  -> Media tiempo:   ${MEDIA_TIEMPO} sec"
echo "  -> Media distance: ${MEDIA_DISTANCE}"

# Fila para resumen.txt (np = hilos)
echo "${THREADS} ${MEDIA_TIEMPO} ${MEDIA_DISTANCE}" >> "$SUMMARY_FILE"

echo "Resumen generado: ${SUMMARY_FILE}"
