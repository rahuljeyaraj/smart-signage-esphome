#include "smart_signage.h"
#include "ctrl/ctrl_ao.h"
#include "radar/radar_ao.h"
#include "imu/imu_ao.h"
#include "led/led_ao.h"
#include "audio/audio_ao.h"
#include "fsm_logger.h"
#include "esp_littlefs.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <cstring>

namespace esphome::smart_signage {

/*───────── Queues ─────────*/
ctrl::Q  ctrlQ;
radar::Q radarQ;
imu::Q   imuQ;
led::Q   ledQ;
audio::Q audioQ;

/*───────── FSMs ───────────*/
ctrl::FSM  ctrlFsm(radarQ, imuQ, ledQ, audioQ);
radar::FSM radarFsm(ctrlQ);
imu::FSM   imuFsm(ctrlQ);
led::FSM   ledFsm(ctrlQ);
audio::FSM audioFsm(ctrlQ);

/*───────── Loggers ────────*/
FsmLogger ctrlFsmLogger("ctrlFsmLogger");
FsmLogger radarFsmLogger("radarFsmLogger");
FsmLogger imuFsmLogger("imuFsmLogger");
FsmLogger ledFsmLogger("ledFsmLogger");
FsmLogger audioFsmLogger("audioFsmLogger");

/*───────── Active Objects (Tasks) ─────────*/
ctrl::AO  ctrlAo(ctrlQ, ctrlFsm, ctrlFsmLogger, "ctrlTask", 8192, tskIDLE_PRIORITY + 2, 1);
radar::AO radarAo(radarQ, radarFsm, radarFsmLogger, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1);
imu::AO   imuAo(imuQ, imuFsm, imuFsmLogger, "imuTask", 8192, tskIDLE_PRIORITY + 2, 1);
led::AO   ledAo(ledQ, ledFsm, ledFsmLogger, "ledTask", 8192, tskIDLE_PRIORITY + 2, 1);
audio::AO audioAo(audioQ, audioFsm, audioFsmLogger, "audioTask", 8192, tskIDLE_PRIORITY + 2, 1);
/*──────────────────────────────────────────*/

void print_littlefs_tree(const char *dirpath, int depth = 0) {
    static constexpr char TAG[] = "ss";
    DIR                  *dir   = opendir(dirpath);
    if (!dir) {
        LOGE("Could not open directory: %s", dirpath);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        // Construct full path
        char fullpath[256];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);

        // Check if entry is a directory or file
        struct stat st;
        if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) {
            LOGI("[DIR]  %s", fullpath);
            // Recursive call for subdirectory
            print_littlefs_tree(fullpath, depth + 1);
        } else {
            LOGI("\t[FILE] %s", fullpath);
        }
    }
    closedir(dir);
}

void SmartSignage::setup() {
    LOGI("SmartSignage setup");
    LOGI("SmartSignage setup: mounting LittleFS...");

    esp_vfs_littlefs_conf_t conf = {
        .base_path              = "/littlefs", // mount point
        .partition_label        = "littlefs",  // must match your partition table
        .format_if_mount_failed = true,
        .dont_mount             = false,
    };
    esp_err_t err = esp_vfs_littlefs_register(&conf);
    if (err != ESP_OK) {
        LOGE("Failed to mount LittleFS (%s)", esp_err_to_name(err));
    } else {
        LOGI("Mounted LittleFS successfully at /littlefs");
    }
}

void SmartSignage::loop() {

    LOGI("SmartSignage loop: Listing LittleFS tree...");
    print_littlefs_tree("/littlefs");
    vTaskDelay(pdMS_TO_TICKS(2000));

    // LOGI("SmartSignage loop");
    // ctrlQ.post(ctrl::CmdSetup{});
    // vTaskDelay(pdMS_TO_TICKS(100));
    // ctrlQ.post(ctrl::CmdStart{});
    // vTaskDelay(pdMS_TO_TICKS(100));
    // ctrlQ.post(ctrl::CmdStop{});
    // vTaskDelay(pdMS_TO_TICKS(100));
    // ctrlQ.post(ctrl::CmdTeardown{});
    // vTaskDelay(pdMS_TO_TICKS(500));
}

void SmartSignage::dump_config() { LOGI("SmartSignage component loaded"); }

} // namespace esphome::smart_signage
