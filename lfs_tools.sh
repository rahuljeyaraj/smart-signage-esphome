#!/usr/bin/env bash
set -euo pipefail

# Standalone LittleFS helper (build & flash) — no Docker
# - Creates a local Python venv for tools (default: .lfstools)
# - Builds an image from FS_DIR (default: ./data)
# - Gets offset/size from CSV (data, spiffs|littlefs), or env overrides
# - eg: sudo ./lfs_tools.sh upload
#
# Commands:
#   build                 -> create image only
#   flash [PORT]          -> flash existing image to device
#   upload [PORT]         -> build + flash
#   info                  -> print resolved params
#   clean                 -> remove image (and optionally venv with CLEAN_VENV=1)
#
# Env overrides:
#   FS_DIR=data           # folder to pack
#   CSV=...               # partition table CSV to parse
#   FS_IMG=.esphome/littlefs.img
#   BLOCK=4096
#   FS_OFFSET=0x....      # override offset (hex or dec)
#   FS_SIZE=0x....        # override size   (hex or dec)
#   CHIP=esp32s3
#   BAUD=921600
#   PORT=/dev/ttyACM0     # default serial port (if not detected/passed)
#   VENV_DIR=.lfstools    # tool venv location
#   YAML=smart_signage.yaml # (optional) to auto-find .esphome/build/<name>/partitions.csv

FS_DIR="${FS_DIR:-data}"
CSV="${CSV:-./custom_partitions.csv}"
FS_IMG="${FS_IMG:-build/littlefs.img}"
BLOCK="${BLOCK:-4096}"
CHIP="${CHIP:-esp32s3}"
BAUD="${BAUD:-921600}"
PORT="${PORT:-}"
VENV_DIR="${VENV_DIR:-.lfstools}"
YAML="${YAML:-smart_signage.yaml}"

detect_port() { ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null | head -n1 || true; }

ensure_tools() {
  if [ ! -x "$VENV_DIR/bin/python3" ]; then
    python3 -m venv "$VENV_DIR"
    "$VENV_DIR/bin/pip" install --upgrade pip >/dev/null
  fi
  if ! "$VENV_DIR/bin/littlefs-python" --version >/dev/null 2>&1; then
    "$VENV_DIR/bin/pip" install --upgrade littlefs-python >/dev/null
  fi
  if ! "$VENV_DIR/bin/esptool" --help >/dev/null 2>&1; then
    "$VENV_DIR/bin/pip" install --upgrade esptool >/dev/null
  fi
}

# -------- CSV parsing without awk (pure bash) --------
shopt -s extglob

_trim() {
  # trim leading/trailing whitespace
  local s="$1"
  s="${s##+([[:space:]])}"
  s="${s%%+([[:space:]])}"
  printf '%s' "$s"
}

_lower() {
  printf '%s' "$1" | tr '[:upper:]' '[:lower:]'
}

# Echo "OFFSET SIZE" if a data,spiffs|littlefs row is found
parse_csv() {
  local csv="$1"
  [ -f "$csv" ] || return 1
  while IFS= read -r line || [ -n "$line" ]; do
    # strip comments
    line="${line%%#*}"
    line="$(_trim "$line")"
    [ -z "$line" ] && continue
    IFS=',' read -r f1 f2 f3 f4 f5 _rest <<<"$line"
    f1="$(_trim "${f1:-}")"
    f2="$(_trim "${f2:-}")"
    f3="$(_trim "${f3:-}")"
    f4="$(_trim "${f4:-}")"
    f5="$(_trim "${f5:-}")"
    local t2="$(_lower "$f2")"
    local t3="$(_lower "$f3")"
    if [[ "$t2" == "data" && ( "$t3" == "spiffs" || "$t3" == "littlefs" ) ]]; then
      echo "$f4 $f5"
      return 0
    fi
  done < "$csv"
  return 1
}

find_csv() {
  if [ -n "${CSV:-}" ] && [ -f "$CSV" ]; then
    echo "$CSV"; return 0
  fi
  local gen=".esphome/build/$(basename "${YAML%%.*}")/partitions.csv"
  if [ -f "$gen" ]; then
    echo "$gen"; return 0
  fi
  echo ""; return 1
}

# Normalize to 0xHEX if it's a plain decimal
norm_hex() {
  local v="$1"
  case "$v" in
    0x[0-9A-Fa-f]*) echo "$v" ;;
    *[!0-9]*|"")    echo "$v" ;;
    *)              printf "0x%X" "$v" ;;
  esac
}

resolve_params() {
  local off="" size="" csv=""
  csv="$(find_csv || true)"
  if [ -n "$csv" ]; then
    set -- $(parse_csv "$csv" || echo "")
    off="${1:-}"; size="${2:-}"
  fi
  off="${FS_OFFSET:-$off}"
  size="${FS_SIZE:-$size}"
  if [ -z "$size" ]; then
    echo "ERR" "Could not resolve FS size; set FS_SIZE or provide a CSV with a data/spiffs row." >&2
    return 1
  fi
  echo "$(norm_hex "$off") $(norm_hex "$size")"
}

cmd_info() {
  local off size csv
  csv="$(find_csv || true)"
  read -r off size <<<"$(resolve_params)"
  echo "FS_DIR    : $FS_DIR"
  echo "FS_IMG    : $FS_IMG"
  echo "CSV       : ${CSV:-$csv}"
  echo "BLOCK     : $BLOCK"
  echo "OFFSET    : ${off:-<none>}"
  echo "SIZE      : $size"
  echo "CHIP      : $CHIP"
  echo "BAUD      : $BAUD"
  echo "PORT      : ${PORT:-$(detect_port)}"
  echo "VENV_DIR  : $VENV_DIR"
}

cmd_build() {
  ensure_tools
  read -r _off size <<<"$(resolve_params)"
  mkdir -p "$(dirname "$FS_IMG")"
  echo "Building LittleFS image:"
  echo "  dir   : $FS_DIR"
  echo "  image : $FS_IMG"
  echo "  size  : $size"
  echo "  block : $BLOCK"
  "$VENV_DIR/bin/littlefs-python" create \
    --block-size "$BLOCK" \
    --fs-size "$size" \
    "$FS_DIR" \
    "$FS_IMG"
  ls -lh "$FS_IMG"
}

cmd_flash() {
  ensure_tools
  local p="${1:-$PORT}"
  [ -z "$p" ] && p="$(detect_port)"
  [ -z "$p" ] && { echo "No serial port found. Pass PORT or arg."; exit 1; }
  [ -f "$FS_IMG" ] || { echo "Image not found: $FS_IMG (run: $0 build)"; exit 1; }
  read -r off _size <<<"$(resolve_params)"
  [ -n "$off" ] || { echo "Offset is empty; set FS_OFFSET or provide CSV"; exit 1; }
  echo "Flashing LittleFS:"
  echo "  image : $FS_IMG"
  echo "  offset: $off"
  echo "  port  : $p"
  echo "  chip  : $CHIP"
  echo "  baud  : $BAUD"
  "$VENV_DIR/bin/esptool" --chip "$CHIP" --port "$p" --baud "$BAUD" \
    write-flash "$off" "$FS_IMG"
}

cmd_upload() {
  cmd_build
  cmd_flash "${1:-}"
}

cmd_clean() {
  [ -f "$FS_IMG" ] && rm -f "$FS_IMG" && echo "Removed $FS_IMG" || true
  if [ "${CLEAN_VENV:-0}" = "1" ] && [ -d "$VENV_DIR" ]; then
    rm -rf "$VENV_DIR"
    echo "Removed venv $VENV_DIR"
  fi
}

case "${1:-}" in
  build)   cmd_build ;;
  flash)   shift; cmd_flash "${1:-}" ;;
  upload)  shift; cmd_upload "${1:-}" ;;
  info)    cmd_info ;;
  clean)   cmd_clean ;;
  ""|help|-h|--help)
    sed -n '1,120p' "$0"
    ;;
  *) echo "Unknown command: $1 (try: $0 help)"; exit 1;;
esac
