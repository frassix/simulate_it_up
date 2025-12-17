import re
import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import make_interp_spline

# Nome del file di input
file_path = "/home/frassa/MEGAlib/docker/Percentile_max.txt"

# Dizionario per contenere i dati:
# struttura: dati[pitch] = {"energy": [], "p90": [], "p95": [], "p99": []}
dati = {}

# Regex per riconoscere le righe di pitch e le energie
pitch_pattern = re.compile(r'^Pitch\s+([\d.]+)um', re.IGNORECASE)
energy_pattern = re.compile(r'^(\d+(?:\.\d+)?)\s*keV', re.IGNORECASE)
percent_pattern = re.compile(r'(\d+)\w*\s+Percentile:\s*([0-9.]+)', re.IGNORECASE)

with open(file_path, "r", encoding="utf-8") as f:
    lines = [line.strip() for line in f if line.strip()]

current_pitch = None
i = 0
while i < len(lines):
    line = lines[i]

    # 1) Riconosciamo il pitch
    m_pitch = pitch_pattern.match(line)
    if m_pitch:
        current_pitch = m_pitch.group(1)  # es. "31.25", "62.50", "125"
        if current_pitch not in dati:
            dati[current_pitch] = {"energy": [], "p90": [], "p95": [], "p99": []}
        i += 1
        continue

    # 2) Riconosciamo l'energia (es. "100 keV" o "500keV")
    m_energy = energy_pattern.match(line)
    if m_energy and current_pitch is not None:
        energy_val = float(m_energy.group(1))

        # Leggiamo le tre righe successive con i percentili
        if i + 3 > len(lines):
            raise ValueError("Formato file inaspettato: mancano righe di percentile.")

        p90_line = lines[i + 1]
        p95_line = lines[i + 2]
        p99_line = lines[i + 3]

        # Estrazione dei numeri dai percentili
        m_p90 = percent_pattern.search(p90_line)
        m_p95 = percent_pattern.search(p95_line)
        m_p99 = percent_pattern.search(p99_line)

        if not (m_p90 and m_p95 and m_p99):
            raise ValueError(f"Formato percentile inaspettato vicino a: {line}")

        p90_val = float(m_p90.group(2))
        p95_val = float(m_p95.group(2))
        p99_val = float(m_p99.group(2))

        # Salviamo nei vettori del pitch corrente
        dati[current_pitch]["energy"].append(energy_val)
        dati[current_pitch]["p90"].append(p90_val)
        dati[current_pitch]["p95"].append(p95_val)
        dati[current_pitch]["p99"].append(p99_val)

        # Avanziamo oltre energia + 3 righe di percentile
        i += 4
        continue

    # Se la riga non è né Pitch né energia, passa avanti
    i += 1

# --- Plot dei risultati ---

# Ordiniamo i pitch in ordine numerico
pitches_ordinati = sorted(dati.keys(), key=lambda x: float(x))

for pitch in pitches_ordinati:
    energy = dati[pitch]["energy"]
    p90 = dati[pitch]["p90"]
    p95 = dati[pitch]["p95"]
    p99 = dati[pitch]["p99"]

    # Ordiniamo i punti per energia (nel caso non fossero in ordine)
    zipped = sorted(zip(energy, p90, p95, p99), key=lambda x: x[0])
    energy, p90, p95, p99 = zip(*zipped)

    plt.figure()
    plt.plot(energy, p90, marker="o", label="90° percentile")
    plt.plot(energy, p95, marker="s", label="95° percentile")
    plt.plot(energy, p99, marker="^", label="99° percentile")

    plt.title(f"Pitch {pitch} um", fontsize=22, fontweight="bold")
    plt.xlabel("Energia [keV]", fontsize=18, fontweight="bold")
    plt.ylabel("Valore percentile", fontsize=18, fontweight="bold")
    plt.xscale('log')
    
    plt.grid(True)
    plt.legend(fontsize=15)
    plt.tight_layout()

# --- Nuovo plot: media dei 99° percentile vs pitch + spline ---

# Calcoliamo la media del 99° percentile per ciascun pitch
pitches_num = []
mean_99_list = []

for pitch in pitches_ordinati:
    p99_vals = np.array(dati[pitch]["p99"], dtype=float)
    mean_99 = p99_vals[3:].mean()
    pitches_num.append(float(pitch))
    mean_99_list.append(mean_99)

pitches_num = np.array(pitches_num)
mean_99_array = np.array(mean_99_list)

# Creiamo una spline per interpolare i punti
# k=2 (spline quadratica) va bene per 3 punti
x_smooth = np.linspace(pitches_num.min(), pitches_num.max(), 200)
spline = make_interp_spline(pitches_num, mean_99_array, k=2)
y_smooth = spline(x_smooth)

plt.figure()
# punti originali
plt.scatter(pitches_num, mean_99_array, label="Media 99° percentile", zorder=3)
# curva interpolata
plt.plot(x_smooth, y_smooth, label="Spline interpolante", zorder=2)

plt.title("Media del 99° percentile in funzione del pitch", fontsize=22, fontweight="bold")
plt.xlabel("Pitch [µm]", fontsize=18, fontweight="bold")
plt.ylabel("99° percentile [keV]", fontsize=18, fontweight="bold")
plt.grid(True)
plt.legend(fontsize=15)
plt.tight_layout()

plt.show()
