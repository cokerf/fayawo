# Fayawo genuine WebAssembly port — milestone 1

This branch begins a source-level port of the C++ Stunt Rally 3 game.

## What is already real
- The code in `src/vdrift/carbrake.cpp`, `src/vdrift/caraero.cpp`, `src/vdrift/carclutch.cpp`, `src/vdrift/cartransmission.cpp`, `src/vdrift/carengine.cpp`, `src/vdrift/carwheel.cpp` and the original vector/rotational-frame headers is compiled from the original repository, not rewritten in JavaScript.
- `physics_bridge.cpp` is a narrow C ABI exposing methods from those original physics classes.
- Only desktop precompiled-header and debug-print dependencies are conditionally excluded from the six original implementation files for `FAYAWO_WEB`.
- Native smoke tests assert numerical results for the original ES.car parser, ES vehicle-specific suspension/engine/brakes/gearbox, baseline suspension forces, engine torque, wheel resistance, aero, clutch, and vectors (run in CI; inspect the workflow result before treating them as passed).
- `build.sh` compiles those original files to `physics.wasm` and the Emscripten JS loader. The `site/index.html` is a diagnostic page, **not** a playable game.

## Build locally
Install and activate the Emscripten SDK (emsdk), then:

```bash
bash web/build.sh
python3 -m http.server 8000 -d site
```

Visit http://localhost:8000/. Use HTTP, not `file://`, because WebAssembly modules are fetched by the browser.

The GitHub Actions workflow is configured to build artifacts and run a native smoke test. **Do not claim these tests pass until a completed run has been inspected.**

## Verification status

Code has been committed, but a successful Emscripten build or browser run has **not yet been confirmed**. Consult the PR Checks tab and the `Build Fayawo WebAssembly physics milestone` workflow. This is not a playable game or a finished port.

The C++ `CONFIGFILE` parser is also compiled from `src/vdrift/configfile.cpp`. The browser can load a selected original `.car` file and inspect its real engine RPM limit, first gear ratio, and front suspension travel **using the original C++ parser**. The selected vehicle also supplies the actual config values to original C++ subsystem calculations for engine torque, front brake torque, front spring force, and gearbox torque. This remains subsystem-level verification, **not** full vehicle simulation. Original `CARSUSPENSION` spring and damping force calculations are exposed through the same WebAssembly bridge.

The asset staging command copies the original `data/carsim/normal/cars/*.car` vehicle configuration files into `site/assets/cars/`, plus a JSON manifest. The original C++ parser loads the vehicle configuration and feeds select physics subsystems, but **full CARDYNAMICS construction, terrain physics and 3D rendering are not implemented**. The diagnostic shows selected values only.

## Netlify branch preview

The `feat/webassembly-port` branch has its own `netlify.toml`. It runs `web/build-netlify.sh`, which installs pinned Emscripten **during the build**, compiles the original VDrift C++ WebAssembly module, and publishes `site/index.html`, `site/physics.js`, `site/physics.wasm` and original car configuration files.

**This is a physics/configuration diagnostic, not a playable port.** The root `index.html` belongs to the earlier substitute prototype and is **not** published from this branch. Netlify installs may be lengthy and could fail due to external tooling or build limits; verify deployment logs before sharing a working preview.

## Original tyre physics milestone

The `src/vdrift/cartire.cpp` Pacejka calculations now join the browser WebAssembly target. `web/prepare-assets.py` stages original `data/carsim/normal/tires/*.tire` assets alongside original `.car` files. The diagnostic loads an original `.tire` through the original `CONFIGFILE` parser and plots longitudinal and lateral force from original `CARTIRE` methods. Force plots are diagnostic only: they do not yet constitute coupled tyre/suspension/Bullet vehicle dynamics.

The browser build also copies `web/tire-plot.js` for interactive diagnostic curves. This script only draws samples obtained by calling the compiled C++ functions.

## Bullet world proof of integration

`web/bullet_probe.cpp` creates a Bullet C++ dynamics world, static ground and a 950 kg box rigid body, then steps gravity and collisions on a fixed timestep. This compiles against Emscripten's Bullet port (`-sUSE_BULLET=1`) into `site/bullet.js` and `site/bullet.wasm`. The browser diagnostic exposes a simulated drop height.

This is deliberately an isolated integration probe. **Fayawo's own `CARDYNAMICS` vehicle chassis, wheel contacts, tyres and terrain are not yet wired to Bullet**; it is not a playable stage. The native Bullet physics dependency is compiled rather than reimplemented in JS.

## Original CARDYNAMICS rigid-body dependency

`web/original_rigidbody.cpp` now instantiates and steps **the original `src/vdrift/rigidbody.h` RIGIDBODY class**. Its LINEARFRAME and ROTATIONALFRAME handle momentum, gravity, positional integration, angular momentum and yaw torque, just as in the source game. The test mass is derived from the selected original `.car` values (engine, driver, wheels, mass particles and declared fuel density). A simplified diagonal inertia estimate is used at this stage, **not** the full game's mass-particle inertia tensor.

The WebAssembly browser diagnostic uses the original class through C ABI functions and shows free-body position and velocity after force/torque application. **The probe deliberately has no Bullet collision, wheel contacts, suspension attachment or ground constraint**, so its height falls below zero with sufficient simulated time. Those systems must be coupled through the original `CARDYNAMICS` to make the car drivable. No replacement game or fake physics engine is provided.

## Important next milestones
1. Compile the full Bullet-coupled `CARDYNAMICS` and surrounding original simulation as WebAssembly, replacing Ogre-bound data types at the boundary.
2. Make existing asset paths and configuration portable; add original cars, tire configs and track assets to a streaming/preloaded filesystem.
3. Adapt or replace Ogre-Next rendering with a WebGL2/WebGPU-compatible layer without changing game rules and gameplay.
4. Wire browser input/audio, game loop, and real vehicles.
5. Tune performance, load a track and complete the first genuinely drivable stage. Only then change the root landing page to load the real game.

The existing root `index.html` on `main` is a separate JavaScript prototype and is **not used by this milestone**. Do not mistake the test page or any prototype for a complete port.

Preserve the upstream GPLv3 license and comply with third-party asset licenses when shipping.
