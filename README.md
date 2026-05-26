# Proyecto Final - Programación Paralela y Concurrente

Proyecto en C++ con OpenMP que cumple:

- Código secuencial base.
- Línea base paralela generada por IA con OpenMP.
- Optimización manual de OpenMP.
- Comparación de schedulers `static`, `dynamic` y `guided` con `chunk size`.
- Histograma con `atomic` vs variables locales alineadas para evitar false sharing.
- Filtro de convolución con estructura SPMD usando `#pragma omp simd`.
- Extra: medición con `OMP_PROC_BIND` y `OMP_PLACES`.

## 1. Preparar Ubuntu

```bash
sudo apt update
sudo apt install -y build-essential g++ make git python3 python3-pip linux-tools-common linux-tools-generic
pip3 install pandas matplotlib
```

Si `pip3 install` falla por permisos:

```bash
sudo apt install -y python3-pandas python3-matplotlib
```

## 2. Compilar

```bash
make clean
make all
```

## 3. Ejecutar una prueba rápida

```bash
W=1280 H=720 ITER=300 RADIUS=7 ./scripts/run_benchmarks.sh
python3 scripts/plot_results.py
```

## 4. Ejecutar prueba formal

Para 4K, recomendado si tu laptop no es muy potente:

```bash
W=3840 H=2160 ITER=1000 RADIUS=15 ./scripts/run_benchmarks.sh
python3 scripts/plot_results.py
```

Para 8K real, como pide el documento:

```bash
W=7680 H=4320 ITER=1000 RADIUS=15 ./scripts/run_benchmarks.sh
python3 scripts/plot_results.py
```

## 5. Recolectar hardware

```bash
./scripts/collect_hardware.sh
```

El archivo queda en `results/hardware_info.txt`.

## 6. Verificar vectorización

Al compilar `optimized_openmp.cpp`, el Makefile genera:

```bash
results/vectorization_report.txt
results/vectorization_missed.txt
```

Busca líneas donde diga `loop vectorized` o similar.

## 7. Commits sugeridos para GitHub

```bash
git init
git add src/sequential.cpp Makefile README.md
git commit -m "Commit 1: código secuencial base"

git add src/ai_parallel.cpp
git commit -m "Commit 2: línea base paralela generada por IA con OpenMP"

git add src/optimized_openmp.cpp scripts
git commit -m "Commit 3: optimizaciones manuales, schedulers, histograma y afinidad"

git add docs results
git commit -m "Commit 4: reporte técnico, resultados y gráficas"
```

Nota: si copias y pegas, corrige `ngit` por `git` en los últimos dos comandos. Lo dejé visible para que no ejecutes commits finales antes de generar resultados.

## 8. Subir a GitHub

```bash
git branch -M main
git remote add origin https://github.com/TU_USUARIO/proyecto-final-openmp.git
git push -u origin main
```

En el reporte pega el enlace del repositorio.
