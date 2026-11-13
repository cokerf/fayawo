#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
if ! command -v em++ >/dev/null 2>&1; then
  echo "Emscripten (em++) is required. Install/activate emsdk first." >&2
  exit 1
fi
mkdir -p site
cp web/index.html site/index.html
cp web/tire-plot.js site/tire-plot.js
cp web/rigidbody-ui.js site/rigidbody-ui.js
cp web/contact-ui.js site/contact-ui.js
em++ -std=c++17 -O2 -DFAYAWO_WEB \
  -I src/vdrift \
  web/physics_bridge.cpp \
  web/original_rigidbody.cpp \
  src/vdrift/carbrake.cpp \
  src/vdrift/caraero.cpp \
  src/vdrift/carclutch.cpp \
  src/vdrift/cartransmission.cpp \
  src/vdrift/cardifferential.cpp \
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
  -s EXPORTED_FUNCTIONS='["_fayawo_brake_torque","_fayawo_aero_drag","_fayawo_vector_length","_fayawo_clutch_torque","_fayawo_gearbox_torque","_fayawo_clutch_speed","_fayawo_engine_torque","_fayawo_wheel_rolling_resistance","_fayawo_wheel_inertia","_fayawo_suspension_force","_fayawo_suspension_step","_fayawo_car_config_load","_fayawo_car_config_number","_fayawo_car_config_exists","_fayawo_selected_gearbox_torque","_fayawo_selected_brake_torque","_fayawo_selected_suspension_force","_fayawo_selected_engine_torque","_fayawo_tire_config_load","_fayawo_tire_longitudinal_force","_fayawo_tire_lateral_force","_fayawo_original_vehicle_mass","_fayawo_original_body_reset","_fayawo_original_body_step","_fayawo_original_body_x","_fayawo_original_body_z","_fayawo_original_body_vx","_fayawo_original_body_vz","_fayawo_original_body_yaw_rate","_fayawo_selected_differential_side","_fayawo_physics_abi_version"]' \
  -o site/physics.js
# The same Bullet C++ dependency used by the native game, compiled for WASM.
em++ -std=c++17 -O2 -DFAYAWO_WEB -I src/vdrift -sUSE_BULLET=1 --no-entry \
  web/bullet_probe.cpp \
  web/coupled_contact.cpp \
  src/vdrift/carsuspension.cpp \
  src/vdrift/cartire.cpp \
  src/vdrift/configfile.cpp \
  -s MODULARIZE=1 \
  -s EXPORT_NAME=createFayawoBullet \
  -s EXPORTED_RUNTIME_METHODS='["cwrap"]' \
  -s ENVIRONMENT=web \
  -s EXPORTED_FUNCTIONS='["_fayawo_bullet_drop_height","_fayawo_bullet_probe_version","_fayawo_contact_load_car","_fayawo_contact_load_tire","_fayawo_contact_reset","_fayawo_contact_step","_fayawo_contact_y","_fayawo_contact_z","_fayawo_contact_vz","_fayawo_contact_mass","_fayawo_contact_normal","_fayawo_contact_traction","_fayawo_contact_count"]' \
  -o site/bullet.js
python3 web/prepare-assets.py
echo "WebAssembly physics module and original car configs built into site/"
