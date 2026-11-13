#!/usr/bin/env bash
set -euo pipefail

# Netlify's build image does not ship with the Emscripten toolchain.
# This is a BUILD-TIME install, not a browser dependency.
if ! command -v em++ >/dev/null 2>&1; then
  EMSDK_VERSION="4.0.18"
  EMSDK_ROOT="${NETLIFY_CACHE_DIR:-${HOME}/.cache}/fayawo-emsdk"
  mkdir -p "$(dirname "$EMSDK_ROOT")"
  if [[ ! -x "$EMSDK_ROOT/emsdk" ]]; then
    git clone --depth 1 https://github.com/emscripten-core/emsdk.git "$EMSDK_ROOT"
  fi
  pushd "$EMSDK_ROOT" >/dev/null
  ./emsdk install "$EMSDK_VERSION"
  ./emsdk activate "$EMSDK_VERSION"
  # shellcheck source=/dev/null
  source ./emsdk_env.sh
  popd >/dev/null
fi

bash web/build.sh
