// #pragma once

// #include <cstring>
// #include "nvs_flash.h"
// #include "nvs.h"

// #include "ctrl/ctrl_const.h"
// #include "radar/radar_const.h"
// #include "imu/imu_const.h"
// #include "led/led_const.h"
// #include "audio/audio_const.h"

// #include "log.h" // Assuming your LOGD macro is here

// namespace esphome::smart_signage {

// class Config {
//   public:
//     // Singleton access
//     static Config &instance() {
//         static Config instance; // Guaranteed to be lazy-initialized and thread-safe (C++11+)
//         return instance;
//     }

//     // Delete copy constructor and assignment
//     Config(const Config &)            = delete;
//     Config &operator=(const Config &) = delete;

//     bool getSessionMins(const char *ns, uint32_t &value) {
//         LOGD("getSessionMins from ns='%s'", ns);
//         return getValue(ns, kSessionMinsKey, value, ctrl::kDefaultSessionMins);
//     }
//     bool setSessionMins(const char *ns, uint32_t value) {
//         LOGD("setSessionMins to %u in ns='%s'", value, ns);
//         return setValue(ns, kSessionMinsKey, value);
//     }

//     bool getRadarRangeCm(const char *ns, uint32_t &value) {
//         LOGD("getRadarRangeCm from ns='%s'", ns);
//         return getValue(ns, kRadarRangeCmKey, value, radar::kDefaultRangeCm);
//     }
//     bool setRadarRangeCm(const char *ns, uint32_t value) {
//         LOGD("setRadarRangeCm to %u in ns='%s'", value, ns);
//         return setValue(ns, kRadarRangeCmKey, value);
//     }

//     bool getLedBrightPct(const char *ns, uint32_t &value) {
//         LOGD("getLedBrightPct from ns='%s'", ns);
//         return getValue(ns, kLedBrightPctKey, value, led::kDefaultBrightPct);
//     }
//     bool setLedBrightPct(const char *ns, uint32_t value) {
//         LOGD("setLedBrightPct to %u in ns='%s'", value, ns);
//         return setValue(ns, kLedBrightPctKey, value);
//     }

//     bool getAudioVolPct(const char *ns, uint32_t &value) {
//         LOGD("getAudioVolPct from ns='%s'", ns);
//         return getValue(ns, kAudioVolPctKey, value, audio::kDefaultVolPct);
//     }
//     bool setAudioVolPct(const char *ns, uint32_t value) {
//         LOGD("setAudioVolPct to %u in ns='%s'", value, ns);
//         return setValue(ns, kAudioVolPctKey, value);
//     }

//     bool getCurrProfile(const char *ns, char *buffer, size_t bufferSize) {
//         if (bufferSize == 0) {
//             LOGD("getCurrProfile called with zero bufferSize");
//             return false;
//         }
//         nvs_handle_t handle;
//         if (nvs_open(ns, NVS_READONLY, &handle) != ESP_OK) {
//             LOGD("Failed to open NVS for getCurrProfile in ns='%s'", ns);
//             buffer[0] = '\0'; // Clear buffer on error
//             return false;
//         }
//         size_t    requiredSize = bufferSize;
//         esp_err_t err          = nvs_get_str(handle, kCurrProfileKey, buffer, &requiredSize);
//         nvs_close(handle);
//         if (err == ESP_OK) {
//             buffer[bufferSize - 1] = '\0'; // Ensure null termination
//             LOGD("getCurrProfile success, ns='%s', profile='%s'", ns, buffer);
//             return true;
//         } else {
//             LOGD("Failed to getCurrProfile string, err=%d, ns='%s'", err, ns);
//             buffer[0] = '\0';
//             return false;
//         }
//     }

//     bool setCurrProfile(const char *ns, const char *profile) {
//         if (profile == nullptr) {
//             LOGD("setCurrProfile called with nullptr profile");
//             return false;
//         }
//         size_t len = std::strlen(profile);
//         if (len > kMaxProfileLen) {
//             LOGD("setCurrProfile rejected profile too long (len=%zu)", len);
//             return false;
//         }

//         nvs_handle_t handle;
//         if (nvs_open(ns, NVS_READWRITE, &handle) != ESP_OK) {
//             LOGD("Failed to open NVS for setCurrProfile in ns='%s'", ns);
//             return false;
//         }

//         esp_err_t err = nvs_set_str(handle, kCurrProfileKey, profile);
//         if (err == ESP_OK) {
//             nvs_commit(handle);
//             LOGD("setCurrProfile success, ns='%s', profile='%s'", ns, profile);
//         } else {
//             LOGD("Failed to setCurrProfile string, err=%d, ns='%s'", err, ns);
//         }
//         nvs_close(handle);
//         return err == ESP_OK;
//     }

//   private:
//     Config() = default; // Private constructor

//     static constexpr char TAG[] = "Config";

//     static constexpr size_t kMaxProfileLen = 15; // NVS_KEY_NAME_MAX_SIZE

//     static constexpr char kCurrProfileKey[]  = "CurrProfile";
//     static constexpr char kSessionMinsKey[]  = "sessionMins";
//     static constexpr char kRadarRangeCmKey[] = "radarRangeCm";
//     static constexpr char kLedBrightPctKey[] = "ledBrightPct";
//     static constexpr char kAudioVolPctKey[]  = "audioVolPct";

//     bool setValue(const char *ns, const char *key, uint32_t value) {
//         nvs_handle_t handle;
//         if (nvs_open(ns, NVS_READWRITE, &handle) != ESP_OK) {
//             LOGD("Failed to open NVS for setValue key='%s' ns='%s'", key, ns);
//             return false;
//         }
//         esp_err_t err = nvs_set_u32(handle, key, value);
//         if (err == ESP_OK) {
//             nvs_commit(handle);
//             LOGD("setValue success key='%s' value=%u ns='%s'", key, value, ns);
//         } else {
//             LOGD("Failed to setValue key='%s' err=%d ns='%s'", key, err, ns);
//         }
//         nvs_close(handle);
//         return err == ESP_OK;
//     }

//     bool getValue(const char *ns, const char *key, uint32_t &value) {
//         nvs_handle_t handle;
//         if (nvs_open(ns, NVS_READONLY, &handle) != ESP_OK) {
//             LOGD("Failed to open NVS for getValue key='%s' ns='%s'", key, ns);
//             return false;
//         }
//         esp_err_t err = nvs_get_u32(handle, key, &value);
//         nvs_close(handle);
//         if (err == ESP_OK) {
//             LOGD("getValue success key='%s' value=%u ns='%s'", key, value, ns);
//             return true;
//         } else {
//             LOGD("Failed to getValue key='%s' err=%d ns='%s'", key, err, ns);
//             return false;
//         }
//     }

//     bool getValue(const char *ns, const char *key, uint32_t &value, uint32_t defaultValue) {
//         if (getValue(ns, key, value)) { return true; }
//         LOGD("Key '%s' not found in ns='%s', setting default=%u", key, ns, defaultValue);
//         if (!setValue(ns, key, defaultValue)) {
//             LOGD("Failed to write default for key='%s' ns='%s'", key, ns);
//             return false;
//         }
//         value = defaultValue;
//         return true;
//     }
// };

// } // namespace esphome::smart_signage
