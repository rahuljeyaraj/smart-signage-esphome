// src/profile_settings.h
#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

#include <etl/array.h>
#include <etl/vector.h>
#include <etl/string.h>

#include "log.h"
#include "common.h"          // ProfileName
#include "profile_catalog.h" // ProfileCatalog (names + audio/led tables)
#include "radar/radar_const.h"
#include "audio/audio_const.h"
#include "led/led_const.h"
#include "ctrl/ctrl_const.h"

// Storage interface (adjust path if different in your tree)
#include "storage/istorage.h"

namespace esphome::smart_signage {

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

class ProfileSettings {
  public:
    static constexpr uint16_t kBlobVersion = 1;
    static constexpr char     TAG[]        = "ProfileSettings";

    // Inject storage + catalog in the ctor
    ProfileSettings(storage::IStorage &storage, const ProfileCatalog &catalog)
        : st_(storage), catalog_(catalog) {}

    // Load current profile+values. If current is absent:
    //  - choose defaultName = first non-empty in catalog_.profileNames (or "profile")
    //  - defaultVals = ProfileValues{}
    //  - persist them, and return those.
    bool loadCurrentSettings(ProfileName &outName, ProfileValues &outVals) {
        ProfileName curr{};
        if (!loadCurrentProfileName_(curr) || curr.empty()) {
            const ProfileName   defaultName = pickDefaultName_();
            const ProfileValues defaultVals{};

            if (!setCurrentProfile(defaultName)) return false;
            if (!storeSettings(defaultName, defaultVals)) return false;

            outName = defaultName;
            outVals = defaultVals;
            SS_LOGI("defaulted to \"%s\"", defaultName.c_str());
            return true;
        }

        ProfileValues vals{};
        if (!loadProfileValues_(curr, vals)) {
            // Stored name exists but no values; seed with defaults for that name
            const ProfileValues defaultVals{};
            if (!storeSettings(curr, defaultVals)) return false;
            outName = curr;
            outVals = defaultVals;
            SS_LOGW("values missing; wrote defaults for \"%s\"", curr.c_str());
            return true;
        }

        outName = curr;
        outVals = vals;
        return true;
    }

    // Back-compat helper: signature kept, simply delegates to loadCurrentSettings.
    bool loadCurrentOrDefault(
        const ProfileName & /*ignored*/, ProfileName &outName, ProfileValues &outVals) {
        return loadCurrentSettings(outName, outVals);
    }

    // Persist values for a profile
    bool storeSettings(const ProfileName &name, const ProfileValues &vals) {
        ProfileBlob blob{};
        blob.version = kBlobVersion;
        blob.pad     = 0;
        blob.values  = vals;
        blob.crc32   = calcCrc_(&blob, offsetof(ProfileBlob, crc32));

        const auto k  = makeProfileKey_(name);
        const bool ok = st_.storeBlob(k, &blob, sizeof(blob));
        if (!ok)
            SS_LOGE("storeBlob failed for key=\"%s\"", k.c_str());
        else
            SS_LOGI("stored key=\"%s\"", k.c_str());
        return ok;
    }

    // Persist current profile string
    bool setCurrentProfile(const ProfileName &name) {
        storage::Key k{"CurrProfile"};
        const bool   ok = st_.storeString(k, name.c_str());
        if (!ok)
            SS_LOGE("storeString CurrProfile failed");
        else
            SS_LOGI("current=\"%s\"", name.c_str());
        return ok;
    }

    // —— Spec accessors by (profile, event name) ——
    bool getAudioPlaySpec(
        const ProfileName &profile, const char *eventName, audio::AudioPlaySpec &out) const {
        return catalog_.getAudioPlaySpec(profile, eventName, out);
    }
    bool getLedPlaySpec(
        const ProfileName &profile, const char *eventName, led::LedPlaySpec &out) const {
        return catalog_.getLedPlaySpec(profile, eventName, out);
    }

    // Names for UI
    const ProfileList &profileNames() const { return catalog_.profileNames; }

    // ── Small compatibility helpers (keep existing call sites compiling) ──
    bool loadCurrentVal(ProfileValues &out) {
        ProfileName tmp;
        return loadCurrentSettings(tmp, out);
    }
    void getCurrentProfile(ProfileName &out) { (void) loadCurrentProfileName_(out); }
    template <size_t N>
    void getProfileList(etl::vector<ProfileName, N> &out) const {
        out.clear();
        for (const auto &n : catalog_.profileNames) {
            if (!n.empty()) {
                if (out.full()) break;
                out.push_back(n);
            }
        }
    }

  private:
    // Choose default profile name only when needed (no caching in members)
    ProfileName pickDefaultName_() const {
        for (const auto &n : catalog_.profileNames) {
            if (!n.empty()) return n;
        }
        return ProfileName{"profile"};
    }

    // Build a sanitized key like "p_<lowercase_alnum_underscore>"
    static storage::Key makeProfileKey_(const ProfileName &name) {
        storage::Key key;
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
        const uint8_t *p   = (const uint8_t *) data;
        while (len--) {
            crc ^= *p++;
            for (int i = 0; i < 8; ++i) {
                uint32_t m = -(crc & 1u);
                crc        = (crc >> 1) ^ (0xEDB88320u & m);
            }
        }
        return ~crc;
    }

    bool loadCurrentProfileName_(ProfileName &out) {
        storage::Key k{"CurrProfile"};
        char         buf[64] = {0};
        size_t       cap     = sizeof(buf);
        if (!st_.loadString(k, buf, cap)) {
            out.clear();
            return false;
        }
        out = buf;
        return true;
    }

    bool loadProfileValues_(const ProfileName &name, ProfileValues &out) {
        const auto  k = makeProfileKey_(name);
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
        const uint32_t expect = calcCrc_(&blob, offsetof(ProfileBlob, crc32));
        if (blob.version != kBlobVersion || expect != blob.crc32) {
            SS_LOGW("bad version/crc for key=\"%s\"", k.c_str());
            return false;
        }
        out = blob.values;
        return true;
    }

  private:
    storage::IStorage &st_;
    ProfileCatalog     catalog_;
};

} // namespace esphome::smart_signage
