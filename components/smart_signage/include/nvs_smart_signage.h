#pragma once

#include <cstdint>
#include <cstring>

#include "log.h"
#include "common.h"
#include "ctrl/ctrl_const.h"
#include "radar/radar_const.h"
#include "led/led_const.h"
#include "audio/audio_const.h"

// ESP-IDF NVS
#include "nvs.h"
#include "nvs_flash.h"

namespace esphome::smart_signage {

using ::esphome::smart_signage::ProfileName;

struct ProfileValues {
    uint32_t sessionMins{ctrl::kDefaultSessionMins};
    uint32_t radarRangeCm{radar::kDefaultRangeCm};
    uint8_t  audioVolPct{audio::kDefaultVolPct};
    uint8_t  ledBrightPct{led::kDefaultBrightPct};
};

struct ProfileBlob {
    uint16_t      version;
    uint16_t      pad;
    ProfileValues values;
    uint32_t      crc32;
};

class NvsSmartSignage {
  public:
    static constexpr const char *TAG          = "NvsSmartSignage";
    static constexpr const char *kNamespace   = "SmartSignage";
    static constexpr const char *kCurrProfile = "CurrProfile";
    static constexpr uint16_t    kBlobVersion = 1;

    static bool initNvs() {
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }
        if (err != ESP_OK) {
            SS_LOGE("%s: nvs_flash_init failed err=%d", TAG, (int) err);
            return false;
        }
        return true;
    }

    static bool loadProfile(const ProfileName &name, ProfileValues &out) {
        nvs_handle_t handle{};
        if (!open(handle)) return false;

        char key[16];
        makeKey(name, key, sizeof(key));

        ProfileBlob blob{};
        size_t      size = sizeof(blob);
        esp_err_t   err  = nvs_get_blob(handle, key, &blob, &size);

        if (err == ESP_OK && size == sizeof(blob)) {
            if (blob.version == kBlobVersion && checkCrc(blob)) {
                out = blob.values;
                nvs_close(handle);
                SS_LOGI("%s: loadProfile key=\"%s\" ok", TAG, key);
                return true;
            } else {
                SS_LOGW("%s: loadProfile key=\"%s\" bad version/crc", TAG, key);
            }
        } else {
            SS_LOGW("%s: loadProfile key=\"%s\" missing (%d)", TAG, key, (int) err);
        }

        out     = ProfileValues{};
        bool ok = saveProfile(name, out);
        nvs_close(handle);
        return ok;
    }

    static bool saveProfile(const ProfileName &name, const ProfileValues &in) {
        nvs_handle_t handle{};
        if (!open(handle)) return false;

        char key[16];
        makeKey(name, key, sizeof(key));

        ProfileBlob blob{};
        blob.version = kBlobVersion;
        blob.pad     = 0;
        blob.values  = in;
        blob.crc32   = calcCrc(&blob, offsetof(ProfileBlob, crc32));

        esp_err_t err = nvs_set_blob(handle, key, &blob, sizeof(blob));
        if (err != ESP_OK) {
            SS_LOGE("%s: saveProfile key=\"%s\" set_blob err=%d", TAG, key, (int) err);
            nvs_close(handle);
            return false;
        }
        err = nvs_commit(handle);
        nvs_close(handle);
        if (err != ESP_OK) {
            SS_LOGE("%s: saveProfile key=\"%s\" commit err=%d", TAG, key, (int) err);
            return false;
        }
        SS_LOGI("%s: saveProfile key=\"%s\" ok", TAG, key);
        return true;
    }

    static bool getCurrentProfile(ProfileName &out) {
        nvs_handle_t handle{};
        if (!open(handle)) return false;

        size_t    needed = 0;
        esp_err_t err    = nvs_get_str(handle, kCurrProfile, nullptr, &needed);
        if (err == ESP_OK && needed > 0) {
            char buf[64] = {0};
            if (needed >= sizeof(buf)) needed = sizeof(buf);
            err = nvs_get_str(handle, kCurrProfile, buf, &needed);
            if (err == ESP_OK) {
                out = buf;
                nvs_close(handle);
                SS_LOGI("%s: getCurrentProfile \"%s\"", TAG, buf);
                return true;
            }
        }
        nvs_close(handle);
        SS_LOGW("%s: getCurrentProfile missing", TAG);
        out.clear();
        return false;
    }

    static bool setCurrentProfile(const ProfileName &name) {
        nvs_handle_t handle{};
        if (!open(handle)) return false;

        const char *cstr = name.c_str();
        esp_err_t   err  = nvs_set_str(handle, kCurrProfile, cstr);
        if (err != ESP_OK) {
            SS_LOGE("%s: setCurrentProfile set_str err=%d", TAG, (int) err);
            nvs_close(handle);
            return false;
        }
        err = nvs_commit(handle);
        nvs_close(handle);
        if (err != ESP_OK) {
            SS_LOGE("%s: setCurrentProfile commit err=%d", TAG, (int) err);
            return false;
        }
        SS_LOGI("%s: setCurrentProfile \"%s\"", TAG, cstr);
        return true;
    }

    static bool loadCurrentVal(ProfileValues &valsOut) {
        ProfileName name;
        getCurrentProfile(name);
        return loadProfile(name, valsOut);
    }

    static bool storeCurrentVal(ProfileValues vals) {
        ProfileName name;
        getCurrentProfile(name);
        return saveProfile(name, vals);
    }

    static bool loadCurrentOrDefault(
        const ProfileName &defaultName, ProfileName &nameOut, ProfileValues &valsOut) {
        getCurrentProfile(nameOut);
        if (nameOut.empty()) {
            nameOut = defaultName;
            setCurrentProfile(nameOut);
            SS_LOGW("%s: current profile absent, defaulted to \"%s\"", TAG, nameOut.c_str());
        }
        return loadProfile(nameOut, valsOut);
    }

    static bool eraseProfile(const ProfileName &name) {
        nvs_handle_t handle{};
        if (!open(handle)) return false;

        char key[16];
        makeKey(name, key, sizeof(key));

        esp_err_t err = nvs_erase_key(handle, key);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
            SS_LOGE("%s: eraseProfile key=\"%s\" err=%d", TAG, key, (int) err);
            nvs_close(handle);
            return false;
        }
        err = nvs_commit(handle);
        nvs_close(handle);
        if (err != ESP_OK) {
            SS_LOGE("%s: eraseProfile key=\"%s\" commit err=%d", TAG, key, (int) err);
            return false;
        }
        SS_LOGI("%s: eraseProfile key=\"%s\" ok", TAG, key);
        return true;
    }

  private:
    static bool open(nvs_handle_t &h) {
        esp_err_t err = nvs_open(kNamespace, NVS_READWRITE, &h);
        if (err != ESP_OK) {
            SS_LOGE("%s: nvs_open ns=\"%s\" err=%d", TAG, kNamespace, (int) err);
            return false;
        }
        return true;
    }

    static void makeKey(const ProfileName &name, char *out, size_t outSz) {
        if (outSz == 0) return;
        size_t maxLen = (outSz - 1);
        size_t n      = 0;
        for (const char *p = name.c_str(); *p && n < maxLen; ++p) {
            char c = *p;
            if (c >= 'A' && c <= 'Z') c = (char) (c - 'A' + 'a');
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_') {
                out[n++] = c;
            } else {
                out[n++] = '_';
            }
        }
        if (n == 0) {
            const char *def = "profile";
            while (*def && n < maxLen) out[n++] = *def++;
        }
        out[n] = '\0';
    }

    static uint32_t calcCrc(const void *data, size_t len) {
        uint32_t       crc = 0xFFFFFFFFu;
        const uint8_t *p   = static_cast<const uint8_t *>(data);
        while (len--) {
            crc ^= *p++;
            for (int i = 0; i < 8; ++i) {
                uint32_t mask = -(crc & 1u);
                crc           = (crc >> 1) ^ (0xEDB88320u & mask);
            }
        }
        return ~crc;
    }

    static bool checkCrc(const ProfileBlob &b) {
        uint32_t expect = calcCrc(&b, offsetof(ProfileBlob, crc32));
        return expect == b.crc32 && b.version == kBlobVersion;
    }
};

} // namespace esphome::smart_signage
