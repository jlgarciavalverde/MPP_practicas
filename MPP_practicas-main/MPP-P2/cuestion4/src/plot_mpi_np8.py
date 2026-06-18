import os
import matplotlib.pyplot as plt

# ==============================
# Configuración de carpetas
# ==============================
SUMMARY_DIR = "resumenes_np8"
PLOTS_DIR = "plots_np8"

os.makedirs(PLOTS_DIR, exist_ok=True)

# ==============================
# Estructuras de datos
# ==============================
tiempos = {}
distancias = {}
eficiencia = {}

# ==============================
# Lectura de datos
# ==============================
for subdir in os.listdir(SUMMARY_DIR):
    full_path = os.path.join(SUMMARY_DIR, subdir)
    if not os.path.isdir(full_path):
        continue

    # Carpeta tipo "nem5_ngm20"
    try:
        parts = subdir.split("_")
        nem_val = int(parts[0].replace("nem", ""))
        ngm_val = int(parts[1].replace("ngm", ""))
    except Exception:
        continue

    resumen_file = os.path.join(full_path, "resumen.txt")
    if not os.path.isfile(resumen_file):
        continue

    np_list, tiempo_list, dist_list, I_list = [], [], [], []

    with open(resumen_file, "r") as f:
        for line in f:
            line = line.strip()
            # Saltar comentarios y cabeceras
            if (not line) or line.startswith("#") or line.startswith("np "):
                continue

            cols = line.split()
            if len(cols) < 3:
                continue

            np_val = int(cols[0])
            t_val = float(cols[1])
            d_val = float(cols[2])

            I_val = d_val / t_val if t_val != 0 else float("nan")

            np_list.append(np_val)
            tiempo_list.append(t_val)
            dist_list.append(d_val)
            I_list.append(I_val)

    tiempos[(nem_val, ngm_val)] = sorted(zip(np_list, tiempo_list))
    distancias[(nem_val, ngm_val)] = sorted(zip(np_list, dist_list))
    eficiencia[(nem_val, ngm_val)] = sorted(zip(np_list, I_list))

# ==============================
# Función genérica de plot
# ==============================
def plot_metric(data_dict, ylabel, title, filename):
    plt.figure(figsize=(10, 6))

    for (nem_val, ngm_val), pairs in data_dict.items():
        x_np = [p[0] for p in pairs]   # esto será [2] para todos en este bash
        y_val = [p[1] for p in pairs]

        etiqueta = f"NEM={nem_val}, NGM={ngm_val}"

        # Como normalmente sólo hay un punto por (NEM,NGM), usamos scatter
        plt.scatter(x_np, y_val, label=etiqueta, marker="o")

        # Texto al lado del punto
        plt.text(
            x_np[-1] + 0.05,
            y_val[-1],
            f"NEM={nem_val}\nNGM={ngm_val}",
            fontsize=8,
            va='center'
        )

    plt.xlabel("Número de procesos (np)", fontsize=12)
    plt.ylabel(ylabel, fontsize=12)
    plt.title(title, fontsize=14, pad=10)
    plt.grid(True, linestyle="--", alpha=0.7)

    # Leyenda lateral (opcional, pero la dejo)
    plt.legend(
        bbox_to_anchor=(1.02, 0.5),
        loc="center left",
        borderaxespad=0.,
        fontsize=9,
        framealpha=0.9
    )

    plt.tight_layout(rect=[0, 0, 0.85, 1])

    output_path = os.path.join(PLOTS_DIR, filename)
    plt.savefig(output_path, dpi=300)
    plt.close()
    print(f"Gráfica guardada en {output_path}")

# ==============================
# Generar las 3 gráficas
# ==============================
plot_metric(
    tiempos,
    ylabel="Tiempo medio de ejecución (s)",
    title="np=8: Tiempo medio vs configuración (NEM,NGM)",
    filename="tiempo_vs_np8.png"
)

plot_metric(
    distancias,
    ylabel="Fitness medio (distance_media)",
    title="np=8: Fitness medio vs configuración (NEM,NGM)",
    filename="distance_vs_np8.png"
)

plot_metric(
    eficiencia,
    ylabel="Índice de eficiencia I = fitness/tiempo",
    title="np=8: Eficiencia I vs configuración (NEM,NGM)",
    filename="eficiencia_vs_np8.png"
)
