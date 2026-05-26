# 🚀 Proyecto Final - Programación Paralela y Concurrente (OpenMP)

![OpenMP](https://img.shields.io/badge/OpenMP-Parallel%20Programming-blue?style=for-the-badge)
![C++](https://img.shields.io/badge/C%2B%2B-17-orange?style=for-the-badge)
![Linux](https://img.shields.io/badge/Ubuntu-Linux-E95420?style=for-the-badge&logo=ubuntu)
![Status](https://img.shields.io/badge/Status-Completed-success?style=for-the-badge)

---

# 📌 Descripción

Este proyecto implementa una solución de **procesamiento paralelo** utilizando **OpenMP en C++** para:

✅ Generar fractales Mandelbrot en resolución ultra alta (8K)  
✅ Aplicar filtros de convolución 2D pesados  
✅ Evaluar rendimiento multinúcleo  
✅ Analizar schedulers OpenMP  
✅ Reducir false sharing  
✅ Forzar vectorización SIMD  
✅ Implementar afinidad de hilos

---

# 🧠 Tecnologías Utilizadas

- ⚙️ C++
- 🧵 OpenMP
- 🐧 Ubuntu Linux
- 📊 Python
- 🌳 Git / GitHub
- ⚡ GCC

---

# 📂 Estructura del Proyecto

```bash
src/
├── sequential.cpp
├── ai_parallel.cpp
└── optimized_openmp.cpp

scripts/
├── benchmark.sh
├── scheduler_test.sh
└── affinity_test.sh

results/
├── *.csv
├── *.png
└── hardware_info.txt

docs/
├── Reporte_Final_OpenMP_Completo.pdf
└── Reporte_Final_OpenMP_Completo.docx

images/
└── optimized_final.ppm
