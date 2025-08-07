#include "smart_signage.h"
#include "log.h"
#include <nvs.h>
#include <nvs_flash.h>

namespace esphome::smart_signage {

SmartSignage::SmartSignage(const UiHandles &ui)
    : nvsConfigManager_(), userIntf_(nvsConfigManager_, ui) {};

void SmartSignage::setup() { userIntf_.registerCallbacks(); }
void SmartSignage::loop() {}
void SmartSignage::dump_config() { LOGI("SmartSignage config:"); }

} // namespace esphome::smart_signage

// /*──────── event map (unchanged) ───────*/
// void SmartSignage::add_event_map(
//     const std::string &profile, const std::string &evt, const PathList &paths) {
//     profiles_[profile][evt] = paths;
// }

// /*──────── NVS helpers ─────────────────*/
// bool SmartSignage::load_from_nvs(const std::string &profile, UserSettings &out) {
//     nvs_handle_t h;
//     if (nvs_open(profile.c_str(), NVS_READONLY, &h) != ESP_OK) return false;

//     uint32_t tmp;
//     if (nvs_get_u32(h, "radius_cm", &tmp) == ESP_OK) out.radius_m = tmp / 100.0f;
//     if (nvs_get_u32(h, "duration_s", &tmp) == ESP_OK) out.duration_s = tmp;
//     if (nvs_get_u32(h, "volume", &tmp) == ESP_OK) out.volume = tmp;
//     if (nvs_get_u32(h, "brightness", &tmp) == ESP_OK) out.brightness = tmp;
//     nvs_close(h);
//     return true;
// }

// void SmartSignage::save_to_nvs(const std::string &profile, const UserSettings &in) {
//     nvs_handle_t h;
//     if (nvs_open(profile.c_str(), NVS_READWRITE, &h) != ESP_OK) return;
//     nvs_set_u32(h, "radius_cm", uint32_t(in.radius_m * 100));
//     nvs_set_u32(h, "duration_s", in.duration_s);
//     nvs_set_u32(h, "volume", in.volume);
//     nvs_set_u32(h, "brightness", in.brightness);
//     nvs_commit(h);
//     nvs_close(h);
// }

// /*──────── dashboard sync ──────────────*/
// void SmartSignage::publish_to_dashboard(const UserSettings &s) {
//     if (radius_num_) radius_num_->publish_state(s.radius_m);
//     if (duration_num_) duration_num_->publish_state(s.duration_s / 3600.0f);
//     if (volume_num_) volume_num_->publish_state(s.volume);
//     if (brightness_num_) brightness_num_->publish_state(s.brightness);
// }

// /*──────── setup ───────────────────────*/
// void SmartSignage::setup() {
//     nvs_flash_init();
//     if (profile_sel_) current_profile_ = profile_sel_->state;
//     load_from_nvs(current_profile_, settings_);
//     cached_[current_profile_] = settings_;
//     publish_to_dashboard(settings_);
//     ESP_LOGI("ss", "Boot profile → %s", current_profile_.c_str());
// }

// /*──────── profile switch ──────────────*/
// void SmartSignage::set_profile(const std::string &p) {
//     save_to_nvs(current_profile_, settings_);
//     cached_[current_profile_] = settings_;

//     current_profile_ = p;
//     ESP_LOGI("ss", "Profile → %s", p.c_str());

//     auto it = cached_.find(p);
//     if (it != cached_.end())
//         settings_ = it->second;
//     else
//         load_from_nvs(p, settings_);

//     publish_to_dashboard(settings_);
// }

// /*──────── live setters ────────────────*/
// void SmartSignage::set_radius(float v) {
//     settings_.radius_m = v;
//     save_to_nvs(current_profile_, settings_);
//     ESP_LOGI("ss", "Radius  → %.2f m", v);
// }
// void SmartSignage::set_duration(float v_h) {
//     settings_.duration_s = static_cast<uint32_t>(v_h * 3600);
//     save_to_nvs(current_profile_, settings_);
//     ESP_LOGI("ss", "Period  → %.1f h", v_h);
// }
// void SmartSignage::set_volume(float v) {
//     settings_.volume = static_cast<uint8_t>(v);
//     save_to_nvs(current_profile_, settings_);
//     ESP_LOGI("ss", "Volume  → %u %%", settings_.volume);
// }
// void SmartSignage::set_brightness(float v) {
//     settings_.brightness = static_cast<uint8_t>(v);
//     save_to_nvs(current_profile_, settings_);
//     ESP_LOGI("ss", "LED Brt → %u %%", settings_.brightness);
// }
// void SmartSignage::on_start_button() { ESP_LOGI("ss", "Start button"); }

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
