#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
if [[ ! -x ./build/cometa_game ]]; then
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCOMETA_REFERENCIAS=OFF
  cmake --build build --parallel 4
fi
exec ./build/cometa_game "$@"
