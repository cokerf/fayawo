# Fayawo genuine WebAssembly port — milestone 1

This branch begins a source-level port of the C++ Stunt Rally 3 game.

## What is already real
- The code in `src/vdrift/carbrake.cpp`, `src/vdrift/caraero.cpp`, `src/vdrift/carclutch.cpp`, `src/vdrift/cartransmission.cpp` and `src/vdrift/mathvector.h` is compiled from the original repository, not rewritten in JavaScript.
- `physics_bridge.cpp` is a narrow C ABI exposing methods from those original physics classes.
- Only debug-print and native precompiled-header dependencies are conditionally excluded from the four original source files for `FAYAWO_WEB`.
- Native smoke tests assert numerical results for brakes, drag, clutch, gears and vectors (run in CI; inspect the workflow result before treating them as passed).
- `build.sh` compiles those original files to `physics.wasm` and the Emscripten JS loader. The `site/index.html` is a diagnostic page, **not** a playable game.

## Build locally
Install and activate the Emscripten SDK (emsdk), then:

```bash
bash web/build.sh
python3 -m http.server 8000 -d site
```

Visit http://localhost:8000/. Use HTTP, not `file://`, because WebAssembly modules are fetched by the browser.

The GitHub Actions workflow also builds artifacts and runs a native smoke test. Check the Actions run for verification.

## Verification status

Code has been committed, but a successful Emscripten build or browser run has **not yet been confirmed**. Consult the PR Checks tab and the `Build Fayawo WebAssembly physics milestone` workflow. This is not a playable game or a finished port.

## Important next milestones
1. Compile the full Bullet-coupled `CARDYNAMICS` and surrounding original simulation as WebAssembly, replacing Ogre-bound data types at the boundary.
2. Make existing asset paths and configuration portable; add original cars, tire configs and track assets to a streaming/preloaded filesystem.
3. Adapt or replace Ogre-Next rendering with a WebGL2/WebGPU-compatible layer without changing game rules and gameplay.
4. Wire browser input/audio, game loop, and real vehicles.
5. Tune performance, load a track and complete the first genuinely drivable stage. Only then change the root landing page to load the real game.

The existing root `index.html` on `main` is a separate JavaScript prototype and is **not used by this milestone**. Do not mistake the test page or any prototype for a complete port.

Preserve the upstream GPLv3 license and comply with third-party asset licenses when shipping.
