import os
import math
import matplotlib.pyplot as plt

# ============================
# Configuración de rutas
# ============================
SUMMARY_DIR = "resumenes_sec_omp8"
SUMMARY_FILE = os.path.join(SUMMARY_DIR, "resumen.txt")
PLOTS_DIR = "plots_sec_omp8"
os.makedirs(PLOTS_DIR, exist_ok=True)

# ============================
# Lectura del resumen
# Formato esperado:
#   # ...
#   np tiempo_medio_sec distance_media
#   8 4.735000 4044330.210000
# ============================
threads = []
tiempos = []
distancias = []
eficiencias = []

if not os.path.isfile(SUMMARY_FILE):
    raise FileNotFoundError(f"No se encuentra {SUMMARY_FILE}")

with open(SUMMARY_FILE, "r") as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith("#") or line.lower().startswith("np "):
            continue
        parts = line.split()
        if len(parts) < 3:
            continue
        th = int(parts[0])
        t = float(parts[1])
        d = float(parts[2])
        e = d / t if t != 0 else math.nan

        threads.append(th)
        tiempos.append(t)
        distancias.append(d)
        eficiencias.append(e)

# ============================
# Función auxiliar para plot
# ============================
def scatter_and_save(x, y, xlabel, ylabel, title, outname, fmt="{:.3f}"):
    if not x:
        print(f"Sin datos para {outname}")
        return
    plt.figure(figsize=(9, 5.5))
    plt.scatter(x, y, s=140, marker='o')  # sin especificar colores
    # Anotar cada punto
    for xi, yi in zip(x, y):
        plt.annotate(fmt.format(yi), (xi, yi), textcoords="offset points",
                     xytext=(0, 10), ha='center', fontsize=10)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.title(title)
    plt.grid(True, linestyle='--', linewidth=0.6, alpha=0.6)
    # ticks limpios para hilos
    if all(isinstance(v, int) for v in x):
        plt.xticks(sorted(x))
    plt.tight_layout()
    out_path = os.path.join(PLOTS_DIR, outname)
    plt.savefig(out_path, dpi=300)
    print(f"✅ Gráfica guardada en {out_path}")

# ============================
# 1) Tiempo (s)
# ============================
scatter_and_save(
    threads, tiempos,
    xlabel="Hilos OpenMP (OMP_NUM_THREADS)",
    ylabel="Tiempo medio (s)",
    title="OpenMP — Tiempo medio por nº de hilos",
    outname="omp_tiempo.png",
    fmt="{:.3f}"
)

# ============================
# 2) Fitness (distance)
# ============================
scatter_and_save(
    threads, distancias,
    xlabel="Hilos OpenMP (OMP_NUM_THREADS)",
    ylabel="Fitness (distance) medio",
    title="OpenMP — Fitness medio por nº de hilos",
    outname="omp_fitness.png",
    fmt="{:.0f}"
)

# ============================
# 3) Eficiencia (distance/tiempo)
# ============================
scatter_and_save(
    threads, eficiencias,
    xlabel="Hilos OpenMP (OMP_NUM_THREADS)",
    ylabel="Eficiencia (distance / tiempo)",
    title="OpenMP — Eficiencia por nº de hilos",
    outname="omp_eficiencia.png",
    fmt="{:.0f}"
)
