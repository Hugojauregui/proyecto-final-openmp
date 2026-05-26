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



## 3. Ejecutar prueba formal

Para 4K:

```bash
W=3840 H=2160 ITER=1000 RADIUS=15 ./scripts/run_benchmarks.sh
python3 scripts/plot_results.py
```

Para 8K real:

```bash
W=7680 H=4320 ITER=1000 RADIUS=15 ./scripts/run_benchmarks.sh
python3 scripts/plot_results.py
```

## 4. Recolectar hardware

```bash
./scripts/collect_hardware.sh
```


## 5. Verificar vectorización

Al compilar `optimized_openmp.cpp`, el Makefile genera:

```bash
results/vectorization_report.txt
results/vectorization_missed.txt
```

