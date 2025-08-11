#!/usr/bin/env bash
set -euo pipefail

# littlefs_flash.sh
# Build a LittleFS image from a folder and flash it to an ESP32 chip.
#
# Examples:
#   ./littlefs_flash.sh --fs-dir ./test --csv custom_partitions.csv --port /dev/ttyACM0
#   ./littlefs_flash.sh --fs-dir ./www --offset 0x47D000 --fs-size 0x200000 --port /dev/ttyACM0
#
# Notes:
#   * If --csv is given, --offset/--fs-size will be auto-filled from the 'littlefs' row (if present).
#   * Installs missing tools (littlefs-python, esptool) into the user site (e.g. ~/.local/bin).
#
# Options:
#   --fs-dir DIR          Directory containing files to pack into LittleFS (required)
#   --img FILE            Output image file (default: littlefs.img)
#   --block-size N        Flash block size (default: 4096)
#   --fs-size N|0xHEX     Size of the FS partition (e.g. 0x200000). If omitted and --csv given, parsed from CSV
#   --offset  N|0xHEX     Flash offset for LittleFS (e.g. 0x47D000). If omitted and --csv given, parsed from CSV
#   --csv FILE            Partition CSV to auto-detect littlefs offset/size
#   --port PORT           Serial port for esptool (e.g. /dev/ttyACM0). If omitted, we only build the image
#   --baud N              Baudrate for flashing (default: 921600)
#   --chip NAME           Chip for esptool (default: esp32s3)
#   --dry-run             Show the commands but don't execute flashing
#   -h|--help             Show help
#
# Requires:
#   python3, pip, littlefs-python, esptool (auto-installed if missing)
#

FS_DIR=""
IMG="littlefs.img"
BLOCK_SIZE="4096"
FS_SIZE=""
OFFSET=""
CSV=""
PORT=""
BAUD="921600"
CHIP="esp32s3"
DRY=0

usage() { sed -n '1,80p' "$0" | sed -n '1,80p' | sed -n '1,80p' >/dev/null; cat "$0" | awk 'NR>=1 && NR<=80{print}'; exit 0; }

need_cmd() { command -v "$1" >/dev/null 2>&1 || return 1; }

ensure_python_tools() {
  if ! need_cmd python3; then
    echo "python3 not found. Please install Python 3." >&2
    exit 1
  fi
  # ensure pip exists
  if ! python3 -m pip --version >/dev/null 2>&1; then
    echo "pip for python3 not found. Trying to ensurepip..."
    python3 -m ensurepip --upgrade || true
  fi
  # install littlefs-python if missing
  if ! need_cmd littlefs-python; then
    echo "Installing littlefs-python to user site..."
    python3 -m pip install --user --upgrade littlefs-python
    export PATH="$HOME/.local/bin:$PATH"
  fi
  # install esptool if missing (only needed if port provided)
  if [ -n "$PORT" ] && ! need_cmd esptool.py; then
    echo "Installing esptool to user site..."
    python3 -m pip install --user --upgrade esptool
    export PATH="$HOME/.local/bin:$PATH"
  fi
}

# Trim spaces helper
trim() { awk '{$1=$1;print}' <<<"$1"; }

# Parse CSV for littlefs row (name in first column), returning offset+size if present
parse_csv() {
  local csv="$1"
  if [ ! -f "$csv" ]; then
    echo "CSV not found: $csv" >&2
    exit 1
  fi
  # Extract the 'littlefs' row (ignore comments), split by comma
  # Fields: Name,Type,SubType,Offset,Size
  local row
  row=$(awk -F',' '
    BEGIN{IGNORECASE=1}
    /^[[:space:]]*#/ {next}
    NF>=5 {
      name=$1; gsub(/^[[:space:]]+|[[:space:]]+$/, "", name);
      if (tolower(name)=="littlefs") {
        off=$4; size=$5;
        gsub(/^[[:space:]]+|[[:space:]]+$/, "", off);
        gsub(/^[[:space:]]+|[[:space:]]+$/, "", size);
        print off "," size;
        exit
      }
    }' "$csv")
  if [ -z "$row" ]; then
    echo "Could not find a 'littlefs' row in CSV: $csv" >&2
    return 1
  fi
  local off="${row%%,*}"
  local size="${row#*,}"
  if [ -z "$OFFSET" ] && [ -n "$off" ]; then OFFSET="$off"; fi
  if [ -z "$FS_SIZE" ] && [ -n "$size" ]; then FS_SIZE="$size"; fi
}

# Normalize a hex/decimal size string to hex with 0x prefix (for esptool)
normalize_hex() {
  local val="$(trim "$1")"
  if [[ "$val" =~ ^0x[0-9A-Fa-f]+$ ]]; then
    echo "$val"
  elif [[ "$val" =~ ^[0-9]+$ ]]; then
    # decimal -> hex
    printf "0x%X" "$val"
  else
    echo "$val"  # pass-through (may fail later)
  fi
}

# Cross-platform file size
filesize_bytes() {
  python3 - "$1" <<'PY'
import os,sys
p=sys.argv[1]
try:
  print(os.path.getsize(p))
except FileNotFoundError:
  print(-1)
PY
}

# ---- arg parse ----
while [ $# -gt 0 ]; do
  case "$1" in
    --fs-dir)        FS_DIR="${2:-}"; shift ;;
    --img)           IMG="${2:-}"; shift ;;
    --block-size)    BLOCK_SIZE="${2:-}"; shift ;;
    --fs-size)       FS_SIZE="${2:-}"; shift ;;
    --offset)        OFFSET="${2:-}"; shift ;;
    --csv)           CSV="${2:-}"; shift ;;
    --port)          PORT="${2:-}"; shift ;;
    --baud)          BAUD="${2:-}"; shift ;;
    --chip)          CHIP="${2:-}"; shift ;;
    --dry-run)       DRY=1 ;;
    -h|--help)       usage ;;
    *) echo "Unknown arg: $1"; usage ;;
  esac
  shift || true
done

if [ -z "$FS_DIR" ]; then
  echo "--fs-dir is required (folder to pack into LittleFS)" >&2
  exit 1
fi

# Auto-fill from CSV if provided
if [ -n "$CSV" ]; then
  parse_csv "$CSV" || true
fi

if [ -z "$FS_SIZE" ]; then
  echo "--fs-size is required (or provide --csv with a 'littlefs' row that has Size)" >&2
  exit 1
fi
if [ -z "$OFFSET" ] && [ -n "$PORT" ]; then
  echo "--offset is required for flashing (or provide --csv with an Offset)" >&2
  exit 1
fi

# Ensure tools
ensure_python_tools

# Normalize values
FS_SIZE_NORM="$(normalize_hex "$FS_SIZE")"
OFFSET_NORM="$( [ -n "$OFFSET" ] && normalize_hex "$OFFSET" || echo "" )"

echo "== LittleFS build =="
echo "  fs-dir     : $FS_DIR"
echo "  image      : $IMG"
echo "  block-size : $BLOCK_SIZE"
echo "  fs-size    : $FS_SIZE_NORM"
[ -n "$CSV" ] && echo "  csv        : $CSV"
[ -n "$PORT" ] && {
  echo "== Flashing =="
  echo "  chip       : $CHIP"
  echo "  port       : $PORT"
  echo "  baud       : $BAUD"
  echo "  offset     : $OFFSET_NORM"
}

# Create image
littlefs-python create --block-size "$BLOCK_SIZE" --fs-size "$FS_SIZE_NORM" "$FS_DIR" "$IMG"

# Show image size
IMG_BYTES="$(filesize_bytes "$IMG")"
if [ "$IMG_BYTES" -gt 0 ]; then
  echo "Created $IMG ($IMG_BYTES bytes)"
fi

# Flash if port provided
if [ -n "$PORT" ]; then
  CMD=( esptool.py --chip "$CHIP" --port "$PORT" --baud "$BAUD" write_flash "$OFFSET_NORM" "$IMG" )
  echo "Running: ${CMD[*]}"
  if [ "$DRY" -eq 0 ]; then
    "${CMD[@]}"
  else
    echo "(dry-run: skipping flash)"
  fi
else
  echo "No --port specified; skipping flash step."
fi

echo "Done."
