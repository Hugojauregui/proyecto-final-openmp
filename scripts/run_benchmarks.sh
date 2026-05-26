#!/usr/bin/env bash
set -euo pipefail
mkdir -p results images bin
make all

W=${W:-3840}       # Cambia a 7680 para 8K real si tu compu aguanta.
H=${H:-2160}       # Cambia a 4320 para 8K real.
ITER=${ITER:-1000}
RADIUS=${RADIUS:-15}
LOGICAL=$(nproc)
MAX_THREADS=$((LOGICAL*2))

echo "version,threads,scheduler,chunk,width,height,taskA,taskB,hist_atomic,hist_local,total,affinity" > results/benchmark.csv

# Secuencial una vez
./bin/sequential "$W" "$H" "$ITER" "$RADIUS" | awk -F, '{for(i=1;i<=NF;i+=2)a[$i]=$(i+1); print a["version"]","a["threads"]","a["scheduler"]","a["chunk"]","a["width"]","a["height"]","a["taskA"]","a["taskB"]",,,"a["total"]",none"}' >> results/benchmark.csv

# IA paralela y optimizado: 1 hilo hasta doble de hilos lógicos
for t in $(seq 1 "$MAX_THREADS"); do
  export OMP_NUM_THREADS=$t
  ./bin/ai_parallel "$W" "$H" "$ITER" "$RADIUS" | awk -F, '{for(i=1;i<=NF;i+=2)a[$i]=$(i+1); print a["version"]","a["threads"]","a["scheduler"]","a["chunk"]","a["width"]","a["height"]","a["taskA"]","a["taskB"]",,,"a["total"]",none"}' >> results/benchmark.csv
  ./bin/optimized "$W" "$H" "$ITER" "$RADIUS" static 32 | awk -F, '{for(i=1;i<=NF;i+=2)a[$i]=$(i+1); print a["version"]","a["threads"]","a["scheduler"]","a["chunk"]","a["width"]","a["height"]","a["taskA"]","a["taskB"]","a["hist_atomic"]","a["hist_local"]","a["total"]",none"}' >> results/benchmark.csv
done

# Prueba de schedulers solamente para Tarea A, usando varios chunk sizes
: > results/scheduler_raw.txt
echo "version,threads,scheduler,chunk,width,height,taskA,taskB,hist_atomic,hist_local,total,affinity" > results/scheduler.csv
export OMP_NUM_THREADS=$LOGICAL
for sched in static dynamic guided; do
  for chunk in 1 4 8 16 32 64 128; do
    ./bin/optimized "$W" "$H" "$ITER" "$RADIUS" "$sched" "$chunk" | tee -a results/scheduler_raw.txt | awk -F, '{for(i=1;i<=NF;i+=2)a[$i]=$(i+1); print a["version"]","a["threads"]","a["scheduler"]","a["chunk"]","a["width"]","a["height"]","a["taskA"]","a["taskB"]","a["hist_atomic"]","a["hist_local"]","a["total"]",none"}' >> results/scheduler.csv
  done
done

# 10 puntos extra: afinidad de hilos
for bind in false close spread; do
  export OMP_PROC_BIND=$bind
  export OMP_PLACES=cores
  export OMP_NUM_THREADS=$LOGICAL
  ./bin/optimized "$W" "$H" "$ITER" "$RADIUS" guided 32 | awk -F, -v aff="PROC_BIND='$bind' PLACES=cores" '{for(i=1;i<=NF;i+=2)a[$i]=$(i+1); print a["version"]","a["threads"]","a["scheduler"]","a["chunk"]","a["width"]","a["height"]","a["taskA"]","a["taskB"]","a["hist_atomic"]","a["hist_local"]","a["total"]","aff}' >> results/benchmark.csv
done

echo "Listo. CSV en results/benchmark.csv y results/scheduler.csv"
