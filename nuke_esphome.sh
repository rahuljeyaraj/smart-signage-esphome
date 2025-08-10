#!/usr/bin/env bash
set -e

echo "🚨 Nuking ESPHome build cache and Docker volumes..."

# Go to the project root (edit if needed)
PROJECT_DIR="$(pwd)"

# 1. Remove .esphome from project (force if root-owned)
if [ -d "$PROJECT_DIR/.esphome" ]; then
    echo "🗑 Removing $PROJECT_DIR/.esphome ..."
    sudo rm -rf "$PROJECT_DIR/.esphome"
else
    echo "✅ No .esphome folder found."
fi

# 2. Remove named Docker volumes (ignore errors if missing)
echo "🗑 Removing Docker volumes: esphome_pio, esphome_cache ..."
docker volume rm esphome_pio esphome_cache 2>/dev/null || true

# 3. Remove any dangling ESPHome-related containers/images
echo "🗑 Removing stopped containers..."
docker rm $(docker ps -aq) 2>/dev/null || true

echo "🗑 Removing dangling images..."
docker rmi $(docker images -q -f dangling=true) 2>/dev/null || true

echo "✅ All ESPHome caches and Docker volumes removed."
