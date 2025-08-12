#pragma once

#include <Arduino.h>       // Serial
#include <FS.h>            // fs::FS, File
#include <LittleFS.h>      // LittleFS object
#include <esp_partition.h> // esp_partition_*
#include <esp_err.h>
#include "log.h"

// ─── Print Partition Table ───────────────────────────────────────────────────
inline void print_partition_table() {
    static const char       *TAG = "PartitionTable";
    esp_partition_iterator_t it =
        esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
    if (!it) {
        SS_LOGE("No partitions found!");
        return;
    }

    while (it != NULL) {
        const esp_partition_t *part = esp_partition_get(it);
        if (part) {
            SS_LOGD("Name: %-10s Type: 0x%02X SubType: 0x%02X Offset: 0x%06X Size: %u KB",
                part->label,
                part->type,
                part->subtype,
                part->address,
                part->size / 1024);
        }
        it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);
}

// ─── Directory Listing Helper ────────────────────────────────────────────────
static void print_indent_(int depth) {
    for (int i = 0; i < depth; ++i) { Serial.print("  "); }
}

static void list_dir_(fs::FS &fs, const char *dir, int depth = 0) {
    static constexpr char TAG[] = "LFS";

    File root = fs.open(dir);
    if (!root) {
        SS_LOGE("open '%s' failed", dir);
        return;
    }
    if (!root.isDirectory()) {
        SS_LOGE("'%s' is not a dir", dir);
        return;
    }

    for (File f = root.openNextFile(); f; f = root.openNextFile()) {
        const bool is_dir = f.isDirectory();
        String     child  = String(dir);
        if (!child.endsWith("/")) child += "/";
        child += f.name();

        print_indent_(depth);
        if (is_dir) {
            SS_LOGD("<DIR> %s", child.c_str());
            list_dir_(fs, child.c_str(), depth + 1);
        } else {
            SS_LOGD("%8u %s", (unsigned) f.size(), child.c_str());
        }
    }
}

inline void list_all_files() {
// optional: show usage stats (if available on your core)
#if defined(ARDUINO) && defined(ARDUINO_ESP32_RELEASE)
    size_t total = LittleFS.totalBytes();
    size_t used  = LittleFS.usedBytes();
    SS_LOGI("LittleFS usage: %u / %u bytes", (unsigned) used, (unsigned) total);
#endif
    list_dir_(LittleFS, "/");
}

#include <cstring>
#include "esp_ota_ops.h"

static void pin_current_boot_partition_if_factory() {
    static const char     *TAG     = "PinBoot";
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (!running) return;

    // Pin only if we're on the factory app, so serial flashes stick,
    // while OTA flows (ota_0/ota_1) remain under OTA control.
    if (std::strncmp(running->label, "ota_", 4) != 0) {
        if (esp_ota_set_boot_partition(running) == ESP_OK) {
            SS_LOGI("Pinned boot partition to: %s", running->label);
        } else {
            SS_LOGE("Failed to pin boot partition");
        }
    } else {
        SS_LOGD("Running from %s; leaving OTA selection unchanged", running->label);
    }
}