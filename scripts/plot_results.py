#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path
Path('results').mkdir(exist_ok=True)

df = pd.read_csv('results/benchmark.csv')
# limpiar filas incompletas
df['threads'] = pd.to_numeric(df['threads'], errors='coerce')
df['total'] = pd.to_numeric(df['total'], errors='coerce')
df = df.dropna(subset=['threads','total'])

base = df[(df.version == 'optimized') & (df.threads == 1) & (df.affinity == 'none')]['total'].min()
if pd.isna(base):
    base = df[df.threads == 1]['total'].min()
df['speedup'] = base / df['total']

for version in ['ai_parallel','optimized']:
    sub = df[(df.version == version) & (df.affinity == 'none')].sort_values('threads')
    if not sub.empty:
        plt.figure()
        plt.plot(sub['threads'], sub['total'], marker='o')
        plt.xlabel('Número de hilos')
        plt.ylabel('Tiempo total (s)')
        plt.title(f'Tiempo vs hilos - {version}')
        plt.grid(True)
        plt.savefig(f'results/tiempo_vs_hilos_{version}.png', dpi=160, bbox_inches='tight')
        plt.close()

        plt.figure()
        plt.plot(sub['threads'], sub['speedup'], marker='o')
        plt.xlabel('Número de hilos')
        plt.ylabel('Speedup')
        plt.title(f'Speedup vs hilos - {version}')
        plt.grid(True)
        plt.savefig(f'results/speedup_vs_hilos_{version}.png', dpi=160, bbox_inches='tight')
        plt.close()

sched_path = Path('results/scheduler.csv')
if sched_path.exists():
    sd = pd.read_csv(sched_path)
    sd['taskA'] = pd.to_numeric(sd['taskA'], errors='coerce')
    sd['chunk'] = pd.to_numeric(sd['chunk'], errors='coerce')
    sd = sd.dropna(subset=['taskA','chunk'])
    plt.figure()
    for sched, sub in sd.groupby('scheduler'):
        sub = sub.sort_values('chunk')
        plt.plot(sub['chunk'], sub['taskA'], marker='o', label=sched)
    plt.xlabel('Chunk size')
    plt.ylabel('Tiempo Tarea A Mandelbrot (s)')
    plt.title('Comparación de schedulers OpenMP')
    plt.legend()
    plt.grid(True)
    plt.savefig('results/schedulers_taskA.png', dpi=160, bbox_inches='tight')
    plt.close()

print('Gráficas generadas en results/*.png')
