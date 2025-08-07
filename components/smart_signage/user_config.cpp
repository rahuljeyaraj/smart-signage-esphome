// #include "user_config.h"
// #include "esphome/core/log.h"

// namespace esphome::smart_signage {

// static const char *TAG = "UserConfig";

// UserConfig::UserConfig(select::Select *currProfile, number::Number *sessionMins,
//     number::Number *radarRangeCm, number::Number *audioVolPct, number::Number *ledBrightPct,
//     button::Button *startButton, select::Select *knobFn, adc_oneshot_unit_handle_t *knobHandle)
//     : curr_profile_(currProfile), session_mins_(sessionMins), radar_range_cm_(radarRangeCm),
//       audio_vol_pct_(audioVolPct), led_bright_pct_(ledBrightPct), start_button_(startButton),
//       knob_fn_(knobFn), knob_handle_(knobHandle) {}

// // Delete copy constructor and assignment
// Config(const Config &)            = delete;
// Config &operator=(const Config &) = delete;

// bool getSessionMins(const char *ns, uint32_t &value) {
//     LOGD("getSessionMins from ns='%s'", ns);
//     return getValue(ns, kSessionMinsKey, value, ctrl::kDefaultSessionMins);
// }
// bool setSessionMins(const char *ns, uint32_t value) {
//     LOGD("setSessionMins to %u in ns='%s'", value, ns);
//     return setValue(ns, kSessionMinsKey, value);
// }

// bool getRadarRangeCm(const char *ns, uint32_t &value) {
//     LOGD("getRadarRangeCm from ns='%s'", ns);
//     return getValue(ns, kRadarRangeCmKey, value, radar::kDefaultRangeCm);
// }
// bool setRadarRangeCm(const char *ns, uint32_t value) {
//     LOGD("setRadarRangeCm to %u in ns='%s'", value, ns);
//     return setValue(ns, kRadarRangeCmKey, value);
// }

// bool getLedBrightPct(const char *ns, uint32_t &value) {
//     LOGD("getLedBrightPct from ns='%s'", ns);
//     return getValue(ns, kLedBrightPctKey, value, led::kDefaultBrightPct);
// }
// bool setLedBrightPct(const char *ns, uint32_t value) {
//     LOGD("setLedBrightPct to %u in ns='%s'", value, ns);
//     return setValue(ns, kLedBrightPctKey, value);
// }

// bool getAudioVolPct(const char *ns, uint32_t &value) {
//     LOGD("getAudioVolPct from ns='%s'", ns);
//     return getValue(ns, kAudioVolPctKey, value, audio::kDefaultVolPct);
// }
// bool setAudioVolPct(const char *ns, uint32_t value) {
//     LOGD("setAudioVolPct to %u in ns='%s'", value, ns);
//     return setValue(ns, kAudioVolPctKey, value);
// }

// bool getCurrProfile(const char *ns, char *buffer, size_t bufferSize) {
//     if (bufferSize == 0) {
//         LOGD("getCurrProfile called with zero bufferSize");
//         return false;
//     }
//     nvs_handle_t handle;
//     if (nvs_open(ns, NVS_READONLY, &handle) != ESP_OK) {
//         LOGD("Failed to open NVS for getCurrProfile in ns='%s'", ns);
//         buffer[0] = '\0'; // Clear buffer on error
//         return false;
//     }
//     size_t    requiredSize = bufferSize;
//     esp_err_t err          = nvs_get_str(handle, kCurrProfileKey, buffer, &requiredSize);
//     nvs_close(handle);
//     if (err == ESP_OK) {
//         buffer[bufferSize - 1] = '\0'; // Ensure null termination
//         LOGD("getCurrProfile success, ns='%s', profile='%s'", ns, buffer);
//         return true;
//     } else {
//         LOGD("Failed to getCurrProfile string, err=%d, ns='%s'", err, ns);
//         buffer[0] = '\0';
//         return false;
//     }
// }

// bool setCurrProfile(const char *ns, const char *profile) {
//     if (profile == nullptr) {
//         LOGD("setCurrProfile called with nullptr profile");
//         return false;
//     }
//     size_t len = std::strlen(profile);
//     if (len > kMaxProfileLen) {
//         LOGD("setCurrProfile rejected profile too long (len=%zu)", len);
//         return false;
//     }

//     nvs_handle_t handle;
//     if (nvs_open(ns, NVS_READWRITE, &handle) != ESP_OK) {
//         LOGD("Failed to open NVS for setCurrProfile in ns='%s'", ns);
//         return false;
//     }

//     esp_err_t err = nvs_set_str(handle, kCurrProfileKey, profile);
//     if (err == ESP_OK) {
//         nvs_commit(handle);
//         LOGD("setCurrProfile success, ns='%s', profile='%s'", ns, profile);
//     } else {
//         LOGD("Failed to setCurrProfile string, err=%d, ns='%s'", err, ns);
//     }
//     nvs_close(handle);
//     return err == ESP_OK;
// }

// private:
// Config() = default; // Private constructor

// static constexpr char TAG[] = "Config";

// static constexpr size_t kMaxProfileLen = 15; // NVS_KEY_NAME_MAX_SIZE

// static constexpr char kCurrProfileKey[]  = "CurrProfile";
// static constexpr char kSessionMinsKey[]  = "sessionMins";
// static constexpr char kRadarRangeCmKey[] = "radarRangeCm";
// static constexpr char kLedBrightPctKey[] = "ledBrightPct";
// static constexpr char kAudioVolPctKey[]  = "audioVolPct";

// } // namespace esphome::smart_signage
