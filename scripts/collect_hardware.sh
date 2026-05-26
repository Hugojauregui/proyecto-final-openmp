#!/usr/bin/env bash
set -euo pipefail
mkdir -p results
{
  echo "===== CPU ====="
  lscpu
  echo
  echo "===== RAM ====="
  free -h
  echo
  echo "===== CACHE SYSFS ====="
  for i in /sys/devices/system/cpu/cpu0/cache/index*; do
    echo "--- $i ---"
    cat "$i/level" "$i/type" "$i/size" 2>/dev/null || true
  done
} | tee results/hardware_info.txt
