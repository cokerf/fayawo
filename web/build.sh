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
  src/vdrift/carengine.cpp \
  src/vdrift/carwheel.cpp \
  src/vdrift/carsuspension.cpp \
  src/vdrift/configfile.cpp \
  src/vdrift/cartire.cpp \
  --no-entry \
  -s WASM=1 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME=createFayawoPhysics \
  -s ENVIRONMENT=web \
  -s EXPORTED_RUNTIME_METHODS='["cwrap"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s EXPORTED_FUNCTIONS='["_fayawo_brake_torque","_fayawo_aero_drag","_fayawo_vector_length","_fayawo_clutch_torque","_fayawo_gearbox_torque","_fayawo_clutch_speed","_fayawo_engine_torque","_fayawo_wheel_rolling_resistance","_fayawo_wheel_inertia","_fayawo_suspension_force","_fayawo_suspension_step","_fayawo_car_config_load","_fayawo_car_config_number","_fayawo_car_config_exists","_fayawo_selected_gearbox_torque","_fayawo_selected_brake_torque","_fayawo_selected_suspension_force","_fayawo_selected_engine_torque","_fayawo_tire_config_load","_fayawo_tire_longitudinal_force","_fayawo_tire_lateral_force","_fayawo_physics_abi_version"]' \
  -o site/physics.js
python3 web/prepare-assets.py
echo "WebAssembly physics module and original car configs built into site/"
