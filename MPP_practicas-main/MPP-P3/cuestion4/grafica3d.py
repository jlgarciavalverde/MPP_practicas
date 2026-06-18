import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
from scipy.interpolate import griddata
from matplotlib.patches import Rectangle 

# Datos 
data = {
    'h1': [1, 2, 4, 6, 1, 2, 4, 6, 1, 2, 4, 6, 1, 2, 4, 6],
    'h2': [1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 4, 4, 6, 6, 6, 6],
    'experimental': [8.51, 6.006, 4.108, 3.126, 4.712, 3.634, 2.448, 2.292, 2.824, 2.44, 2.184, 2.12, 2.838, 2.47, 2.128, 2.112],
    'teorico': [9.05, 5.00, 3.36, 3.16, 4.91, 3.01, 2.45, 2.61, 3.09, 2.27, 2.25, 2.59, 2.71, 2.25, 2.41, 2.81] 
}
df = pd.DataFrame(data)

# Preparación de los puntos y la malla
h1_points = df['h1'].values
h2_points = df['h2'].values
t_exp_points = df['experimental'].values
t_teo_points = df['teorico'].values 

h1_grid = np.linspace(h1_points.min(), h1_points.max(), 50) 
h2_grid = np.linspace(h2_points.min(), h2_points.max(), 50) 
H1, H2 = np.meshgrid(h1_grid, h2_grid)

# Interpolar los datos a la malla
T_EXP = griddata((h1_points, h2_points), t_exp_points, (H1, H2), method='cubic')
T_TEO = griddata((h1_points, h2_points), t_teo_points, (H1, H2), method='cubic')

# Crear la figura y el eje 3D
fig = plt.figure(figsize=(12, 10))
ax = fig.add_subplot(111, projection='3d')

# --- GRAFICAR SUPERFICIES CON COLORES SÓLIDOS (facecolor) ---

# Superficie Teórica
surf_teo = ax.plot_surface(H1, H2, T_TEO, 
                           facecolor='blue', # Color sólido
                           alpha=0.8,        # Transparencia
                           edgecolor='none', 
                           label='Tiempo Teórico') 

# Superficie Experimental 
surf_exp = ax.plot_surface(H1, H2, T_EXP, 
                           facecolor='red', # Color sólido
                           alpha=0.8,       # Más transparencia para ver a través
                           edgecolor='none', 
                           label='Tiempo Experimental') 

# --- ETIQUETAS Y LEYENDA ---

# Etiquetas de los ejes
ax.set_xlabel('Hilos h1')
ax.set_ylabel('Hilos h2')
ax.set_zlabel('Tiempo de Ejecución (s)')
ax.set_title('Comparación de Rendimiento: Experimental vs Teórico', fontsize=14)

# Leyenda
teo_proxy = Rectangle((0, 0), 1, 1, fc='blue', alpha=0.6)
exp_proxy = Rectangle((0, 0), 1, 1, fc='red', alpha=0.4)
ax.legend([teo_proxy, exp_proxy], ['Tiempo Teórico', 'Tiempo Experimental'], loc='upper right', fontsize=12)

# Rotar la vista para una mejor perspectiva inicial
ax.view_init(elev=30, azim=200) 

# Mostrar la gráfica
plt.show()