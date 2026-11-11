#!/usr/bin/env bash
set -e

cd "$(dirname "$0")/.."

PORT="${PORT:-4242}"
CLIENTS="${1:-2}"

cmake --build build --parallel "$(nproc)"

./build/bin/r-type_server -p "$PORT" &
SERVER_PID=$!
trap 'kill $SERVER_PID 2>/dev/null' EXIT

sleep 0.5

CLIENT_PIDS=()
for _ in $(seq 1 "$CLIENTS"); do
    ./build/bin/r-type_client -h 127.0.0.1 -p "$PORT" &
    CLIENT_PIDS+=($!)
done

wait "${CLIENT_PIDS[@]}"
