#!/usr/bin/env bash
set -euo pipefail

# ---- config ----
YAML="${YAML:-smart_signage.yaml}"
IMG="${IMG:-esphome/esphome}"
PIO_VOL="${PIO_VOL:-esphome_pio}"
CACHE_VOL="${CACHE_VOL:-esphome_cache}"

detect_port() { ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null | head -n1 || true; }

run_compile() {
  docker run --rm -it \
    -v "$(pwd):/config" \
    -v "$PIO_VOL":/root/.platformio \
    -v "$CACHE_VOL":/root/.cache \
    "$IMG" compile "$YAML"
}

run_upload_serial() {
  local PORT="${1:-$(detect_port)}"
  [ -z "$PORT" ] && { echo "No serial port found"; exit 1; }
  docker run --rm -it \
    --device="$PORT" \
    -v "$(pwd):/config" \
    -v "$PIO_VOL":/root/.platformio \
    -v "$CACHE_VOL":/root/.cache \
    --network host \
    "$IMG" run "$YAML" --device "$PORT"
}

run_upload_ota() {
  local IP="${1:-}"; [ -z "$IP" ] && { echo "Usage: $0 upload-ota <device_ip>"; exit 1; }
  docker run --rm -it \
    -v "$(pwd):/config" \
    -v "$PIO_VOL":/root/.platformio \
    -v "$CACHE_VOL":/root/.cache \
    --network host \
    "$IMG" run "$YAML" --upload-port "$IP"
}

run_logs_serial() {
  local PORT="${1:-$(detect_port)}"
  [ -z "$PORT" ] && { echo "No serial port found"; exit 1; }
  docker run --rm -it \
    --device="$PORT" \
    -v "$(pwd):/config" \
    -v "$PIO_VOL":/root/.platformio \
    -v "$CACHE_VOL":/root/.cache \
    --network host \
    "$IMG" logs "$YAML" --device "$PORT"
}

run_logs_ota() {
  local IP="${1:-}"; [ -z "$IP" ] && { echo "Usage: $0 logs-ota <device_ip>"; exit 1; }
  docker run --rm -it \
    -v "$(pwd):/config" \
    -v "$PIO_VOL":/root/.platformio \
    -v "$CACHE_VOL":/root/.cache \
    --network host \
    "$IMG" logs "$YAML" --address "$IP"
}

run_clean() {
  # Clean ESPHome build artifacts (keeps PIO/cache volumes)
  docker run --rm -it \
    -v "$(pwd):/config" \
    -v "$PIO_VOL":/root/.platformio \
    -v "$CACHE_VOL":/root/.cache \
    "$IMG" clean "$YAML"
  echo "clean: project build artifacts removed (toolchains cached)."
}

run_nuke() {
  echo "nuke: removing project .esphome, PIO/cache volumes, and ESPHome containers/images"
  # 1) remove project build folder (might be root-owned in prior runs)
  if [ -d ".esphome" ]; then
    rm -rf .esphome 2>/dev/null || sudo rm -rf .esphome
  fi
  # 2) remove PIO/cache volumes
  docker volume rm "$PIO_VOL" "$CACHE_VOL" 2>/dev/null || true
  # 3) remove stopped containers derived from the ESPHome image
  mapfile -t CIDS < <(docker ps -aq --filter "ancestor=$IMG")
  [ "${#CIDS[@]}" -gt 0 ] && docker rm "${CIDS[@]}" 2>/dev/null || true
  # 4) remove dangling layers for the ESPHome image only (safer than global prune)
  mapfile -t IMGIDS < <(docker images -q "$IMG")
  for iid in "${IMGIDS[@]:-}"; do
    docker image rm "$iid" -f 2>/dev/null || true
  done
  echo "nuke: done. next build will refetch everything."
}

case "${1:-}" in
  compile)        run_compile ;;
  upload-serial)  shift; run_upload_serial "${1:-}" ;;
  upload-ota)     shift; run_upload_ota "${1:-}" ;;
  logs-serial)    shift; run_logs_serial "${1:-}" ;;
  logs-ota)       shift; run_logs_ota "${1:-}" ;;
  clean)          run_clean ;;
  nuke)           run_nuke ;;
  ""|help|-h|--help)
    cat <<EOF
Usage: $0 <command> [args]
  compile                    Build firmware only
  upload-serial [PORT]       Build + flash via serial
  upload-ota <IP>            Build + flash OTA
  logs-serial [PORT]         Serial logs
  logs-ota <IP>              OTA logs
  clean                      esphome clean (keeps toolchains/cache)
  nuke                       delete .esphome + PIO/cache volumes + ESPHome containers/images

Env: YAML=smart_signage.yaml IMG=esphome/esphome PIO_VOL=esphome_pio CACHE_VOL=esphome_cache
EOF
    ;;
  *) echo "Unknown command: $1 (try: $0 help)"; exit 1;;
esac
