#include "smart_signage.h"
#include "log.h"

namespace esphome::smart_signage {

/*────────────────────────────── Helpers ──────────────────────────────*/
static inline void log_paths(const char *evt, const SmartSignage::PathList &paths) {
    ESP_LOGI("ss", "  %s:", evt);
    for (auto &p : paths) ESP_LOGI("ss", "    %s", p.c_str());
}

/*──────────────────────────── Callbacks ──────────────────────────────*/
void SmartSignage::set_profile(const std::string &p) {
    current_profile_ = p;
    ESP_LOGI("ss", "Profile → %s", p.c_str());

    auto it = profiles_.find(p);
    if (it == profiles_.end()) {
        ESP_LOGW("ss", "Profile not found!");
        return;
    }
    for (auto &kv : it->second) log_paths(kv.first.c_str(), kv.second);
}

void SmartSignage::set_radius(float v) { settings_.radius_m = v; }
void SmartSignage::set_duration(float v) { settings_.duration_s = v; }
void SmartSignage::set_volume(float v) { settings_.volume = v; }
void SmartSignage::set_brightness(float v) { settings_.brightness = v; }
void SmartSignage::on_start_button() { ESP_LOGI("ss", "Start"); }

/*──────────────────────────── Lifecycle ─────────────────────────────*/
void SmartSignage::setup() { ESP_LOGI("ss", "setup()"); }
void SmartSignage::loop() { /* your FSM work here */ }
void SmartSignage::dump_config() { ESP_LOGCONFIG("ss", "SmartSignage loaded"); }

} // namespace esphome::smart_signage

// #include "smart_signage.h"
// #include "ctrl/ctrl_ao.h"
// #include "radar/radar_ao.h"
// #include "imu/imu_ao.h"
// #include "led/led_ao.h"
// #include "audio/audio_ao.h"
// #include "fsm_logger.h"
// #include "esp_littlefs.h"
// #include <dirent.h>
// #include <stdio.h>
// #include <string.h>
// #include <sys/stat.h>
// #include <cstring>
// #include "config.h"

// namespace esphome::smart_signage {

// /*───────── Queues ─────────*/
// ctrl::Q  ctrlQ;
// radar::Q radarQ;
// imu::Q   imuQ;
// led::Q   ledQ;
// audio::Q audioQ;

// /*───────── FSMs ───────────*/
// ctrl::FSM  ctrlFsm(radarQ, imuQ, ledQ, audioQ);
// radar::FSM radarFsm(ctrlQ);
// imu::FSM   imuFsm(ctrlQ);
// led::FSM   ledFsm(ctrlQ);
// audio::FSM audioFsm(ctrlQ);

// /*───────── Loggers ────────*/
// FsmLogger ctrlFsmLogger("ctrlFsmLogger");
// FsmLogger radarFsmLogger("radarFsmLogger");
// FsmLogger imuFsmLogger("imuFsmLogger");
// FsmLogger ledFsmLogger("ledFsmLogger");
// FsmLogger audioFsmLogger("audioFsmLogger");

// /*───────── Active Objects (Tasks) ─────────*/
// ctrl::AO  ctrlAo(ctrlQ, ctrlFsm, ctrlFsmLogger, "ctrlTask", 8192, tskIDLE_PRIORITY + 2, 1);
// radar::AO radarAo(radarQ, radarFsm, radarFsmLogger, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1);
// imu::AO   imuAo(imuQ, imuFsm, imuFsmLogger, "imuTask", 8192, tskIDLE_PRIORITY + 2, 1);
// led::AO   ledAo(ledQ, ledFsm, ledFsmLogger, "ledTask", 8192, tskIDLE_PRIORITY + 2, 1);
// audio::AO audioAo(audioQ, audioFsm, audioFsmLogger, "audioTask", 8192, tskIDLE_PRIORITY + 2, 1);
// /*──────────────────────────────────────────*/

// // void print_littlefs_tree(const char *dirpath, int depth = 0) {
// //     static constexpr char TAG[] = "ss";
// //     DIR                  *dir   = opendir(dirpath);
// //     if (!dir) {
// //         LOGE("Could not open directory: %s", dirpath);
// //         return;
// //     }

// //     struct dirent *entry;
// //     while ((entry = readdir(dir)) != nullptr) {
// //         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

// //         // Construct full path
// //         char fullpath[256];
// //         snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);

// //         // Check if entry is a directory or file
// //         struct stat st;
// //         if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) {
// //             LOGI("[DIR]  %s", fullpath);
// //             // Recursive call for subdirectory
// //             print_littlefs_tree(fullpath, depth + 1);
// //         } else {
// //             LOGI("\t[FILE] %s", fullpath);
// //         }
// //     }
// //     closedir(dir);
// // }

// Config config;

// void SmartSignage::setup() {
//     LOGI("SmartSignage setup");

//     // bool ok = Config::set_radarDist("ss0", 6);
//     // LOGI("set_radarDist ok=%d", ok);

//     // LOGI("SmartSignage setup: mounting LittleFS...");

//     // esp_vfs_littlefs_conf_t conf = {
//     //     .base_path              = "/littlefs", // mount point
//     //     .partition_label        = "littlefs",  // must match your partition table
//     //     .format_if_mount_failed = true,
//     //     .dont_mount             = false,
//     // };
//     // esp_err_t err = esp_vfs_littlefs_register(&conf);
//     // if (err != ESP_OK) {
//     //     LOGE("Failed to mount LittleFS (%s)", esp_err_to_name(err));
//     // } else {
//     //     LOGI("Mounted LittleFS successfully at /littlefs");
//     // }
// }

// void SmartSignage::loop() {
//     // uint16_t dist = 0;
//     // bool     ok   = Config::get_radarDist("ss0", dist);
//     // LOGI("get_radarDist ok=%d, dist=%d", ok, dist);
//     // vTaskDelay(pdMS_TO_TICKS(2000));

//     // LOGI("SmartSignage loop: Listing LittleFS tree...");
//     // print_littlefs_tree("/littlefs");
//     // vTaskDelay(pdMS_TO_TICKS(2000));

//     // LOGI("SmartSignage loop");
//     // ctrlQ.post(ctrl::CmdSetup{});
//     // vTaskDelay(pdMS_TO_TICKS(100));
//     // ctrlQ.post(ctrl::CmdStart{});
//     // vTaskDelay(pdMS_TO_TICKS(100));
//     // ctrlQ.post(ctrl::CmdStop{});
//     // vTaskDelay(pdMS_TO_TICKS(100));
//     // ctrlQ.post(ctrl::CmdTeardown{});
//     // vTaskDelay(pdMS_TO_TICKS(500));
// }

// void SmartSignage::dump_config() { LOGI("SmartSignage component loaded"); }

// } // namespace esphome::smart_signage
