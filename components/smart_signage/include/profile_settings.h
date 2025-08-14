#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

#include <etl/array.h>
#include <etl/string.h>

#include "log.h"        // SS_LOGx (ASCII-only)
#include "common.h"     // Key, ProfileName
#include "storage.h"    // IStorage
#include "app_config.h" // ProfileCatalog

#include "ctrl/ctrl_const.h"
#include "radar/radar_const.h"
#include "audio/audio_const.h"
#include "led/led_const.h"

namespace esphome::smart_signage {

// Per-profile persisted values (runtime knobs)
struct ProfileValues {
    uint32_t sessionMins{ctrl::kDefaultSessionMins};
    uint32_t radarRangeCm{radar::kDefaultRangeCm};
    uint8_t  audioVolPct{audio::kDefaultVolPct};
    uint8_t  ledBrightPct{led::kDefaultBrightPct};
};

// Versioned + CRC-protected blob layout for storage
struct ProfileBlob {
    uint16_t      version;
    uint16_t      pad;
    ProfileValues values;
    uint32_t      crc32;
};

class ProfileSettings {
  public:
    static constexpr uint16_t kBlobVersion = 1;
    static constexpr char     TAG[]        = "ProfileSettings";

    // Constructor: inject storage only. Catalog is provided later via setCatalog().
    explicit ProfileSettings(IStorage &storage) : st_(storage) {}

    // Set the catalog (names + event tables). Also selects default profile name
    // as the first name in the list, and default values = ProfileValues{}.
    void setCatalog(const ProfileCatalog &catalog) {
        catalog_ = catalog; // copy
        // choose default profile = first non-empty name
        for (const auto &n : catalog_.profileNames) {
            if (!n.empty()) {
                defName_ = n;
                break;
            }
        }
        defVals_ = ProfileValues{};
        if (defName_.empty()) {
            // Fallback name if list is empty; still valid for storage keys
            defName_ = ProfileName{"profile"};
            SS_LOGW("Catalog had no profile names; defaulting to \"profile\"");
        }
    }

    // Load current profile + values; if missing, store defaults and return them.
    bool loadCurrentSettings(ProfileName &outName, ProfileValues &outVals) {
        ProfileName curr{};
        if (!loadCurrentProfileName_(curr) || curr.empty()) {
            curr = defName_;
            if (!setCurrentProfile(curr)) {
                SS_LOGE("setCurrentProfile failed");
                return false;
            }
            if (!storeSettings(curr, defVals_)) {
                SS_LOGE("storeSettings(default) failed");
                return false;
            }
            outName = curr;
            outVals = defVals_;
            SS_LOGI("Defaulted to profile \"%s\"", curr.c_str());
            return true;
        }

        ProfileValues vals{};
        if (!loadProfileValues_(curr, vals)) {
            if (!storeSettings(curr, defVals_)) {
                SS_LOGE("storeSettings(init) failed");
                return false;
            }
            outName = curr;
            outVals = defVals_;
            SS_LOGW("Values missing; wrote defaults for \"%s\"", curr.c_str());
            return true;
        }

        outName = curr;
        outVals = vals;
        return true;
    }

    // Store values for a given profile name
    bool storeSettings(const ProfileName &name, const ProfileValues &vals) {
        ProfileBlob blob{};
        blob.version = kBlobVersion;
        blob.pad     = 0;
        blob.values  = vals;
        blob.crc32   = calcCrc_(&blob, offsetof(ProfileBlob, crc32));

        Key        k  = makeProfileKey_(name);
        const bool ok = st_.storeBlob(k, &blob, sizeof(blob));
        if (!ok)
            SS_LOGE("storeBlob failed for key=\"%s\"", k.c_str());
        else
            SS_LOGI("Stored key=\"%s\"", k.c_str());
        return ok;
    }

    // Persist the current profile string
    bool setCurrentProfile(const ProfileName &name) {
        Key k;
        k             = "CurrProfile";
        const bool ok = st_.storeString(k, name.c_str());
        if (!ok)
            SS_LOGE("storeString CurrProfile failed");
        else
            SS_LOGI("Current=\"%s\"", name.c_str());
        return ok;
    }

    // Accessors so all profile-related data stays centralized
    const etl::array<ProfileName, MAX_PROFILES> &profileNames() const {
        return catalog_.profileNames;
    }

    bool findAudioSpec(const Key &k, audio::AudioPlaySpec &out) const {
        return catalog_.findAudio(k, out);
    }

    bool findLedSpec(const Key &k, LedPlaySpec &out) const { return catalog_.findLed(k, out); }

  private:
    // ───────── storage helpers ─────────

    bool loadCurrentProfileName_(ProfileName &out) {
        Key k;
        k              = "CurrProfile";
        char   buf[64] = {0};
        size_t cap     = sizeof(buf);

        if (!st_.loadString(k, buf, cap)) {
            // if too small or missing, return empty
            out.clear();
            return false;
        }
        out = buf; // truncated to ProfileName capacity if longer
        return true;
    }

    bool loadProfileValues_(const ProfileName &name, ProfileValues &out) {
        Key         k = makeProfileKey_(name);
        ProfileBlob blob{};
        size_t      len = sizeof(blob);
        if (!st_.loadBlob(k, &blob, len)) {
            SS_LOGW("loadBlob missing for key=\"%s\"", k.c_str());
            return false;
        }
        if (len != sizeof(blob)) {
            SS_LOGW("bad blob size for key=\"%s\" (%u)", k.c_str(), (unsigned) len);
            return false;
        }
        if (blob.version != kBlobVersion || !checkCrc_(blob)) {
            SS_LOGW("bad version/crc for key=\"%s\"", k.c_str());
            return false;
        }
        out = blob.values;
        return true;
    }

    static Key makeProfileKey_(const ProfileName &name) {
        // sanitize -> lowercase [a-z0-9_] and prefix "p_"
        Key key;
        key = "p_";
        for (const char *p = name.c_str(); *p && key.size() < key.max_size(); ++p) {
            char c = *p;
            if (c >= 'A' && c <= 'Z') c = (char) (c - 'A' + 'a');
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_') {
                if (key.size() + 1 < key.max_size()) key += c;
            } else {
                if (key.size() + 1 < key.max_size()) key += '_';
            }
        }
        return key;
    }

    static uint32_t calcCrc_(const void *data, size_t len) {
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

    static bool checkCrc_(const ProfileBlob &b) {
        uint32_t expect = calcCrc_(&b, offsetof(ProfileBlob, crc32));
        return (expect == b.crc32) && (b.version == kBlobVersion);
    }

  private:
    IStorage      &st_;
    ProfileCatalog catalog_{}; // set via setCatalog()
    ProfileName    defName_{}; // chosen from catalog_.profileNames[0]
    ProfileValues  defVals_{}; // always ProfileValues{}
};

} // namespace esphome::smart_signage
