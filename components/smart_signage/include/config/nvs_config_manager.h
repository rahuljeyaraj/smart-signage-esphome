// nvs_config_manager.h
#pragma once

#include "config_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "log.h" // ASCII‐only LOGD(fmt, ...)

static inline constexpr char TAG[] = "NVSConfigMgr";

class NVSConfigManager : public ConfigManager {
  public:
    NVSConfigManager() = default;

    // --- Integer impls ---
    bool setValue(const Namespace &ns, const Key &key, uint32_t value) override {
        nvs_handle_t h;
        if (nvs_open(ns.c_str(), NVS_READWRITE, &h) != ESP_OK) {
            LOGD("open ns='%s' key='%s' failed", ns.c_str(), key.c_str());
            return false;
        }
        esp_err_t err = nvs_set_u32(h, key.c_str(), value);
        if (err == ESP_OK) {
            nvs_commit(h);
            LOGD("set ns='%s' key='%s' val=%u", ns.c_str(), key.c_str(), value);
        } else {
            LOGD("set ns='%s' key='%s' err=%d", ns.c_str(), key.c_str(), err);
        }
        nvs_close(h);
        return err == ESP_OK;
    }

    bool getValue(const Namespace &ns, const Key &key, uint32_t &value) override {
        nvs_handle_t h;
        if (nvs_open(ns.c_str(), NVS_READONLY, &h) != ESP_OK) {
            LOGD("open ns='%s' key='%s' failed", ns.c_str(), key.c_str());
            return false;
        }
        esp_err_t err = nvs_get_u32(h, key.c_str(), &value);
        nvs_close(h);
        if (err == ESP_OK) {
            LOGD("got ns='%s' key='%s' val=%u", ns.c_str(), key.c_str(), value);
            return true;
        }
        LOGD("get ns='%s' key='%s' err=%d", ns.c_str(), key.c_str(), err);
        return false;
    }

    bool getValue(
        const Namespace &ns, const Key &key, uint32_t &value, uint32_t defaultValue) override {
        if (getValue(ns, key, value)) return true;
        LOGD("key='%s' missing ns='%s', default=%u", key.c_str(), ns.c_str(), defaultValue);
        if (!setValue(ns, key, defaultValue)) {
            LOGD("default write failed key='%s' ns='%s'", key.c_str(), ns.c_str());
            return false;
        }
        value = defaultValue;
        return true;
    }

    // --- String impls ---
    bool setString(const Namespace &ns, const Key &key, const ValueString &value) override {
        nvs_handle_t h;
        if (nvs_open(ns.c_str(), NVS_READWRITE, &h) != ESP_OK) {
            LOGD("open ns='%s' key='%s' failed", ns.c_str(), key.c_str());
            return false;
        }
        esp_err_t err = nvs_set_str(h, key.c_str(), value.c_str());
        if (err == ESP_OK) {
            nvs_commit(h);
            LOGD("setStr ns='%s' key='%s' val='%s'", ns.c_str(), key.c_str(), value.c_str());
        } else {
            LOGD("setStr ns='%s' key='%s' err=%d", ns.c_str(), key.c_str(), err);
        }
        nvs_close(h);
        return err == ESP_OK;
    }

    bool getString(const Namespace &ns, const Key &key, ValueString &value) override {
        nvs_handle_t h;
        if (nvs_open(ns.c_str(), NVS_READONLY, &h) != ESP_OK) {
            LOGD("open ns='%s' key='%s' failed", ns.c_str(), key.c_str());
            return false;
        }
        size_t    required = 0;
        esp_err_t err      = nvs_get_str(h, key.c_str(), nullptr, &required);
        if (err != ESP_OK || required == 0 || required > (MAX_STRING_SIZE + 1)) {
            LOGD(TAG,
                "getStr size failed ns='%s' key='%s' err=%d req=%u",
                ns.c_str(),
                key.c_str(),
                err,
                required);
            nvs_close(h);
            return false;
        }
        char buf[MAX_STRING_SIZE + 1];
        err = nvs_get_str(h, key.c_str(), buf, &required);
        nvs_close(h);
        if (err == ESP_OK) {
            buf[required - 1] = '\0';
            value             = ValueString(buf);
            LOGD("gotStr ns='%s' key='%s' val='%s'", ns.c_str(), key.c_str(), value.c_str());
            return true;
        }
        LOGD("getStr ns='%s' key='%s' err=%d", ns.c_str(), key.c_str(), err);
        return false;
    }

    bool getString(const Namespace &ns, const Key &key, ValueString &value,
        const ValueString &defaultValue) override {
        if (getString(ns, key, value)) return true;
        LOGD(TAG,
            "key='%s' missing ns='%s', defStr='%s'",
            key.c_str(),
            ns.c_str(),
            defaultValue.c_str());
        if (!setString(ns, key, defaultValue)) {
            LOGD("default write failed key='%s' ns='%s'", key.c_str(), ns.c_str());
            return false;
        }
        value = defaultValue;
        return true;
    }
};
