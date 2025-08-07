#pragma once

#include "config_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "log.h"

class NVSConfigManager : public ConfigManager {
  public:
    explicit NVSConfigManager(const Namespace &ns) : ns_(ns) {}

    bool setValue(const Key &key, uint32_t value) override {
        nvs_handle_t h;
        if (nvs_open(ns_.c_str(), NVS_READWRITE, &h) != ESP_OK) {
            LOGD("open ns='%s' key='%s' failed", ns_.c_str(), key.c_str());
            return false;
        }
        auto err = nvs_set_u32(h, key.c_str(), value);
        if (err == ESP_OK) {
            nvs_commit(h);
            LOGD("set ns='%s' key='%s' val=%u", ns_.c_str(), key.c_str(), value);
        } else {
            LOGD("set ns='%s' key='%s' err=%d", ns_.c_str(), key.c_str(), err);
        }
        nvs_close(h);
        return err == ESP_OK;
    }

    bool getValue(const Key &key, uint32_t &value) override {
        nvs_handle_t h;
        if (nvs_open(ns_.c_str(), NVS_READONLY, &h) != ESP_OK) {
            LOGD("open ns='%s' key='%s' failed", ns_.c_str(), key.c_str());
            return false;
        }
        auto err = nvs_get_u32(h, key.c_str(), &value);
        nvs_close(h);
        if (err == ESP_OK) {
            LOGD("got ns='%s' key='%s' val=%u", ns_.c_str(), key.c_str(), value);
            return true;
        }
        LOGD("get ns='%s' key='%s' err=%d", ns_.c_str(), key.c_str(), err);
        return false;
    }

    bool getValue(const Key &key, uint32_t &value, uint32_t defaultValue) override {
        if (getValue(key, value)) return true;
        LOGD("key='%s' missing ns='%s', default=%u", key.c_str(), ns_.c_str(), defaultValue);
        if (!setValue(key, defaultValue)) {
            LOGD("default write failed key='%s' ns='%s'", key.c_str(), ns_.c_str());
            return false;
        }
        value = defaultValue;
        return true;
    }

    bool eraseAll() override {
        nvs_handle_t h;
        if (nvs_open(ns_.c_str(), NVS_READWRITE, &h) != ESP_OK) {
            LOGD("eraseAll: open ns='%s' failed", ns_.c_str());
            return false;
        }
        auto err = nvs_erase_all(h);
        if (err == ESP_OK) {
            nvs_commit(h);
            LOGD("eraseAll: ns='%s' succeeded", ns_.c_str());
        } else {
            LOGD("eraseAll: ns='%s' err=%d", ns_.c_str(), err);
        }
        nvs_close(h);
        return err == ESP_OK;
    }

  private:
    Namespace ns_;

    static inline constexpr char TAG[] = "NVSConfigMgr";
};
