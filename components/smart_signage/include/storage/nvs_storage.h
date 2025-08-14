#pragma once

#include "storage/istorage.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "log.h"

namespace esphome::smart_signage::storage {
class NvsStorage : public IStorage {
  public:
    explicit NvsStorage(const Namespace &ns) : ns_(ns) {}

    // ───────── Scalars ─────────
    bool storeU32(const Key &key, uint32_t value) override {
        nvs_handle_t h;
        if (nvs_open(ns_.c_str(), NVS_READWRITE, &h) != ESP_OK) return false;
        esp_err_t err = nvs_set_u32(h, key.c_str(), value);
        if (err == ESP_OK) err = nvs_commit(h);
        nvs_close(h);
        return err == ESP_OK;
    }

    bool loadU32(const Key &key, uint32_t &value) override {
        nvs_handle_t h;
        if (nvs_open(ns_.c_str(), NVS_READONLY, &h) != ESP_OK) return false;
        esp_err_t err = nvs_get_u32(h, key.c_str(), &value);
        nvs_close(h);
        return err == ESP_OK;
    }

    bool loadU32OrDefault(const Key &key, uint32_t &value, uint32_t defValue) override {
        if (loadU32(key, value)) return true;
        value = defValue;
        // Try to persist the default so future loads succeed
        (void) storeU32(key, defValue);
        return true;
    }

    // ───────── Blobs ─────────
    bool storeBlob(const Key &key, const void *data, size_t len) override {
        nvs_handle_t h;
        if (nvs_open(ns_.c_str(), NVS_READWRITE, &h) != ESP_OK) return false;
        esp_err_t err = nvs_set_blob(h, key.c_str(), data, len);
        if (err == ESP_OK) err = nvs_commit(h);
        nvs_close(h);
        return err == ESP_OK;
    }

    bool loadBlob(const Key &key, void *data, size_t &inout_len) override {
        nvs_handle_t h;
        if (nvs_open(ns_.c_str(), NVS_READONLY, &h) != ESP_OK) return false;

        size_t    needed = 0;
        esp_err_t err    = nvs_get_blob(h, key.c_str(), nullptr, &needed);
        if (err != ESP_OK) {
            nvs_close(h);
            return false;
        }

        if (inout_len < needed || data == nullptr) {
            // Tell caller how much is needed, no copy
            inout_len = needed;
            nvs_close(h);
            return false;
        }

        err = nvs_get_blob(h, key.c_str(), data, &needed);
        nvs_close(h);
        if (err == ESP_OK) {
            inout_len = needed;
            return true;
        }
        return false;
    }

    // ───────── Strings ─────────
    bool storeString(const Key &key, const char *str) override {
        nvs_handle_t h;
        if (nvs_open(ns_.c_str(), NVS_READWRITE, &h) != ESP_OK) return false;
        esp_err_t err = nvs_set_str(h, key.c_str(), str ? str : "");
        if (err == ESP_OK) err = nvs_commit(h);
        nvs_close(h);
        return err == ESP_OK;
    }

    bool loadString(const Key &key, char *buf, size_t &inout_len) override {
        nvs_handle_t h;
        if (nvs_open(ns_.c_str(), NVS_READONLY, &h) != ESP_OK) return false;

        size_t    needed = 0;
        esp_err_t err    = nvs_get_str(h, key.c_str(), nullptr, &needed);
        if (err != ESP_OK) {
            nvs_close(h);
            return false;
        }

        if (inout_len < needed || buf == nullptr) {
            // Not enough space: report required size (includes null terminator)
            inout_len = needed;
            nvs_close(h);
            return false;
        }

        err = nvs_get_str(h, key.c_str(), buf, &needed);
        nvs_close(h);
        if (err == ESP_OK) {
            inout_len = needed; // includes '\0'
            return true;
        }
        return false;
    }

    // ───────── Housekeeping ─────────
    bool eraseKey(const Key &key) override {
        nvs_handle_t h;
        if (nvs_open(ns_.c_str(), NVS_READWRITE, &h) != ESP_OK) return false;
        esp_err_t err = nvs_erase_key(h, key.c_str());
        if (err == ESP_OK) err = nvs_commit(h);
        nvs_close(h);
        // Treat "not found" as success for idempotence
        return err == ESP_OK || err == ESP_ERR_NVS_NOT_FOUND;
    }

    bool eraseAll() override {
        nvs_handle_t h;
        if (nvs_open(ns_.c_str(), NVS_READWRITE, &h) != ESP_OK) return false;
        esp_err_t err = nvs_erase_all(h);
        if (err == ESP_OK) err = nvs_commit(h);
        nvs_close(h);
        return err == ESP_OK;
    }

  private:
    Namespace                    ns_;
    static inline constexpr char TAG[] = "NvsStorage";
};
} // namespace esphome::smart_signage::storage
