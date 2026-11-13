# Fayawo — browser edition

The repository contains a native C++ copy of Stunt Rally 3. The new root [index.html](../index.html) is a **separate, original, lightweight browser rally prototype**. It is **not** a WebAssembly compilation or complete browser port of the C++ Stunt Rally game. The original C++ source and desktop build remain unchanged.

## Run

Open `index.html` in a modern browser. No install step, dependencies, or build server required. For local HTTP development, run `python -m http.server 8000` from the repository root and open `http://localhost:8000`.

## Controls

- W / Up arrow: accelerate
- S / Down arrow: brake
- A,D / Left,Right arrows: steer
- Space: handbrake
- P / Escape: pause
- On touch screens: use the on-screen controls

## Deploy

Netlify reads `netlify.toml`, copies only `index.html` into the `site/` publish directory, and serves it statically. The full native game sources are **not** deployed to the website. No runtime environment variables required.

## Scope and future native port

The browser prototype includes a procedural gravel course, keyboard/touch controls, arcade handling and a simple rally HUD. It does not include original Stunt Rally vehicles/tracks, VDrift physics, Ogre-Next renderer, editor, multiplayer, or desktop assets. A full port would involve evaluating the Ogre-Next and native dependencies against Emscripten/WebGL/WebGPU, introducing browser platform adapters for input, audio, storage and networking, building a compressed asset pipeline, then optimizing and testing on target browsers. Simply adding HTML to C++ sources cannot perform that conversion.

Original project acknowledgements and licenses are in the repository [Readme.md](../Readme.md) and [License.txt](../License.txt).
