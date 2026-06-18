import os
import math
import matplotlib.pyplot as plt

# ============================
# Rutas de entrada/salida
# ============================
SEC_DIR = "resumenes_sec1"
OMP8_DIR = "resumenes_sec_omp8"
MPI_DIR = os.path.join("resumenes_optimo_global", "nem5_ngm5")  # MPI síncrono (2 procesos)

OUT_DIR = "plots_comparativa_sec_omp_mpi"
os.makedirs(OUT_DIR, exist_ok=True)

# ============================
# Lectura robusta de un resumen
# ============================
def read_summary_row(path, target_np=None):
    """
    Lee 'resumen.txt' con formato:
      # ...
      np tiempo_medio_sec distance_media
      <np> <tiempo> <distance>
    Si target_np es None, toma la primera fila válida.
    Si target_np es int, devuelve la fila cuyo np == target_np (si existe).
    Devuelve (np, tiempo, distance) o None si no se encontró.
    """
    file = os.path.join(path, "resumen.txt")
    if not os.path.isfile(file):
        print(f"No se encuentra {file}")
        return None

    found = None
    with open(file, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#") or line.lower().startswith("np "):
                continue
            parts = line.split()
            if len(parts) < 3:
                continue
            try:
                np_val = int(parts[0])
                t_val = float(parts[1])
                d_val = float(parts[2])
            except ValueError:
                continue

            if target_np is None or np_val == target_np:
                found = (np_val, t_val, d_val)
                if target_np is not None:
                    break

    if found is None:
        who = f"{file} con np={target_np}" if target_np is not None else file
        print(f"No se encontró una fila válida en {who}")
    return found

# ============================
# Cargar los tres puntos
# ============================
sec = read_summary_row(SEC_DIR, target_np=1)
omp = read_summary_row(OMP8_DIR, target_np=8)
mpi = read_summary_row(MPI_DIR, target_np=2)

labels = []
tiempos = []
distancias = []
eficiencias = []

def add_point(name, row):
    if row is None:
        return
    _, t, d = row
    labels.append(name)
    tiempos.append(t)
    distancias.append(d)
    eficiencias.append(d / t if t != 0 else math.nan)

add_point("Secuencial (1 hilo)", sec)
add_point("OpenMP (8 hilos)", omp)
add_point("MPI síncrono (4 procesos)", mpi)

# Comprobación mínima
if not labels:
    raise RuntimeError("No se cargaron datos. Revisa las rutas de 'resumen.txt'.")

# ============================
# Función de plot (puntos)
# ============================
def scatter_cats_and_save(labels, values, ylabel, title, outname, fmt="{:.3f}"):
    x = list(range(len(labels)))
    plt.figure(figsize=(9, 5.5))
    plt.scatter(x, values, s=140, marker='o')  # sin fijar colores
    for xi, yi, lab in zip(x, values, labels):
        plt.annotate(fmt.format(yi), (xi, yi), textcoords="offset points",
                     xytext=(0, 10), ha='center', fontsize=10)
    plt.xticks(x, labels, rotation=0)
    plt.ylabel(ylabel)
    plt.title(title)
    plt.grid(True, linestyle='--', linewidth=0.6, alpha=0.6)
    plt.tight_layout()
    out_path = os.path.join(OUT_DIR, outname)
    plt.savefig(out_path, dpi=300)
    plt.close()
    print(f"Gráfica guardada en {out_path}")

# ============================
# 1) Tiempo
# ============================
scatter_cats_and_save(
    labels, tiempos,
    ylabel="Tiempo medio (s)",
    title="Comparativa Secuencial vs OpenMP vs MPI — Tiempo",
    outname="comparativa_tiempo.png",
    fmt="{:.3f}"
)

# ============================
# 2) Fitness
# ============================
scatter_cats_and_save(
    labels, distancias,
    ylabel="Fitness (distance) medio",
    title="Comparativa Secuencial vs OpenMP vs MPI — Fitness",
    outname="comparativa_fitness.png",
    fmt="{:.0f}"
)

# ============================
# 3) Eficiencia
# ============================
scatter_cats_and_save(
    labels, eficiencias,
    ylabel="Eficiencia (distance / tiempo)",
    title="Comparativa Secuencial vs OpenMP vs MPI — Eficiencia",
    outname="comparativa_eficiencia.png",
    fmt="{:.0f}"
)
