#!/usr/bin/env bash
set -euo pipefail

# ---- config ----
YAML="${YAML:-smart_signage.yaml}"
IMG="${IMG:-esphome/esphome}"
PIO_VOL="${PIO_VOL:-esphome_pio}"
CACHE_VOL="${CACHE_VOL:-esphome_cache}"
DEFAULT_HOST="${DEFAULT_HOST:-smart-signage.local}"

detect_port() { ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null | head -n1 || true; }
normalize_target() {
  local t="${1:-$DEFAULT_HOST}"
  t="${t#http://}"; t="${t#https://}"; t="${t%/}"
  printf '%s' "$t"
}

run_compile() {
  docker run --rm -it \
    -v "$(pwd):/config" \
    -v "$PIO_VOL":/root/.platformio \
    -v "$CACHE_VOL":/root/.cache \
    "$IMG" compile "$YAML"
}

run_auto() {
  local PORT; PORT="$(detect_port || true)"
  if [ -n "${PORT:-}" ]; then
    echo "run: using SERIAL via $PORT"
    docker run --rm -it \
      --device="$PORT" \
      -v "$(pwd):/config" \
      -v "$PIO_VOL":/root/.platformio \
      -v "$CACHE_VOL":/root/.cache \
      --network host \
      "$IMG" run "$YAML" --device "$PORT"
  else
    local TARGET; TARGET="$(normalize_target "${1:-$DEFAULT_HOST}")"
    echo "run: no serial port found; using OTA via $TARGET"
    docker run --rm -it \
      -v "$(pwd):/config" \
      -v "$PIO_VOL":/root/.platformio \
      -v "$CACHE_VOL":/root/.cache \
      --network host \
      "$IMG" run "$YAML" --device "$TARGET"
  fi
}

run_logs_auto() {
  local PORT; PORT="$(detect_port || true)"
  if [ -n "${PORT:-}" ]; then
    echo "logs: using SERIAL via $PORT"
    docker run --rm -it \
      --device="$PORT" \
      -v "$(pwd):/config" \
      -v "$PIO_VOL":/root/.platformio \
      -v "$CACHE_VOL":/root/.cache \
      --network host \
      "$IMG" logs "$YAML" --device "$PORT"
  else
    local TARGET; TARGET="$(normalize_target "${1:-$DEFAULT_HOST}")"
    echo "logs: no serial port found; using OTA via $TARGET"
    docker run --rm -it \
      -v "$(pwd):/config" \
      -v "$PIO_VOL":/root/.platformio \
      -v "$CACHE_VOL":/root/.cache \
      --network host \
      "$IMG" logs "$YAML" --device "$TARGET"
  fi
}

run_clean() {
  docker run --rm -it \
    -v "$(pwd):/config" \
    -v "$PIO_VOL":/root/.platformio \
    -v "$CACHE_VOL":/root/.cache \
    "$IMG" clean "$YAML"
  echo "clean: project build artifacts removed (toolchains cached)."
}

run_nuke() {
  echo "nuke: removing project .esphome, PIO/cache volumes, and ESPHome containers/images"
  [ -d ".esphome" ] && { rm -rf .esphome 2>/dev/null || sudo rm -rf .esphome; }
  docker volume rm "$PIO_VOL" "$CACHE_VOL" 2>/dev/null || true
  mapfile -t CIDS < <(docker ps -aq --filter "ancestor=$IMG")
  [ "${#CIDS[@]}" -gt 0 ] && docker rm "${CIDS[@]}" 2>/dev/null || true
  mapfile -t IMGIDS < <(docker images -q "$IMG")
  for iid in "${IMGIDS[@]:-}"; do docker image rm "$iid" -f 2>/dev/null || true; done
  echo "nuke: done. next build will refetch everything."
}

case "${1:-}" in
  compile)        run_compile ;;
  run)            shift; run_auto "${1:-}" ;;      # unified build + flash (serial preferred, else OTA)
  logs)           shift; run_logs_auto "${1:-}" ;; # unified logs (serial preferred, else OTA)
  clean)          run_clean ;;
  nuke)           run_nuke ;;
  ""|help|-h|--help)
    cat <<EOF
Usage: $0 <command> [args]
  compile                    Build firmware only
  run [HOST]                 Build + flash; prefers SERIAL if found, else OTA to HOST
                             (default HOST: $DEFAULT_HOST)
  logs [HOST]                Logs; prefers SERIAL if found, else OTA to HOST
  clean                      esphome clean (keeps toolchains/cache)
  nuke                       delete .esphome + PIO/cache volumes + ESPHome containers/images

Env: YAML=smart_signage.yaml IMG=esphome/esphome PIO_VOL=esphome_pio CACHE_VOL=esphome_cache DEFAULT_HOST=$DEFAULT_HOST
EOF
    ;;
  *) echo "Unknown command: $1 (try: $0 help)"; exit 1;;
esac
