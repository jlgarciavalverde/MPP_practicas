#!/bin/bash

# ============================
# Configuración
# ============================
REPS=2

P1=1000
P2=400
P3=200
P4=100
MRATE=0.15
INPUT_FILE="../input/input_1000_400_200_100.txt"

OUTDIR="resultados_sec1"
SUMMARY_DIR="resumenes_sec1"

mkdir -p "$OUTDIR"
mkdir -p "$SUMMARY_DIR"

SUMMARY_FILE="${SUMMARY_DIR}/resumen.txt"

# Cabecera del resumen
echo "# Ejecución completamente secuencial (1 hilo, sin OpenMP ni MPI)" > "$SUMMARY_FILE"
echo "np tiempo_medio_sec distance_media" >> "$SUMMARY_FILE"

echo "============================================="
echo " Ejecutando versión SECUENCIAL (sin OpenMP ni MPI)"
echo " Comando: ./sec ${P1} ${P2} ${P3} ${P4} ${MRATE} < ${INPUT_FILE}"
echo " Repeticiones: ${REPS}"
echo "============================================="

# ============================
# Repeticiones
# ============================
for REP in $(seq 1 $REPS); do
  OUTFILE="${OUTDIR}/run${REP}.txt"
  echo "  -> Ejecución ${REP}/${REPS}"
  ./sec "${P1}" "${P2}" "${P3}" "${P4}" "${MRATE}" < "${INPUT_FILE}" > "${OUTFILE}"
done

# ============================
# Cálculo de medias
# ============================
MEDIA_TIEMPO=$(awk '/Execution Time:/ {sum+=$3; n++} END{if(n>0) printf "%.6f", sum/n; else print "NaN"}' "$OUTDIR"/run*.txt)
MEDIA_DISTANCE=$(awk '/Distance:/ {sum+=$2; n++} END{if(n>0) printf "%.6f", sum/n; else print "NaN"}' "$OUTDIR"/run*.txt)

# summary.txt individual
{
  echo "np = 1 (secuencial puro)"
  echo "mrate = ${MRATE}"
  echo "repeticiones = ${REPS}"
  echo "tiempo_medio_sec = ${MEDIA_TIEMPO}"
  echo "distance_media = ${MEDIA_DISTANCE}"
} > "${OUTDIR}/summary.txt"

echo "  -> Media tiempo:   ${MEDIA_TIEMPO} sec"
echo "  -> Media distance: ${MEDIA_DISTANCE}"

# ============================
# Fila para resumen.txt (np = 1)
# ============================
echo "1 ${MEDIA_TIEMPO} ${MEDIA_DISTANCE}" >> "$SUMMARY_FILE"

echo "✅ Resumen generado: ${SUMMARY_FILE}"
