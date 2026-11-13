#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
if ! command -v em++ >/dev/null 2>&1; then
  echo "Emscripten (em++) is required. Install/activate emsdk first." >&2
  exit 1
fi
mkdir -p site
cp web/index.html site/index.html
em++ -std=c++17 -O2 -DFAYAWO_WEB \
  -I src/vdrift \
  web/physics_bridge.cpp \
  src/vdrift/carbrake.cpp \
  src/vdrift/caraero.cpp \
  src/vdrift/carclutch.cpp \
  src/vdrift/cartransmission.cpp \
  --no-entry \
  -s WASM=1 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME=createFayawoPhysics \
  -s ENVIRONMENT=web \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s EXPORTED_FUNCTIONS='["_fayawo_brake_torque","_fayawo_aero_drag","_fayawo_vector_length","_fayawo_clutch_torque","_fayawo_gearbox_torque","_fayawo_clutch_speed","_fayawo_physics_abi_version"]' \
  -o site/physics.js
echo "WebAssembly physics module built into site/"
