// src/profile_settings.h
#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

#include <etl/array.h>
#include <etl/vector.h>
#include <etl/string.h>

#include "log.h"
#include "profile/profile_defs.h"
#include "storage/istorage.h"

namespace esphome::smart_signage::profile {

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

    ProfileSettings(storage::IStorage &storage) : storage_(storage) {}

    // Persist current profile string
    bool writeCurrentProfile(const ProfileName &name) {
        const bool ok = storage_.writeString(kCurrentProfileKey, name.c_str());
        if (!ok)
            SS_LOGE("writeString CurrProfile failed");
        else
            SS_LOGI("current=\"%s\"", name.c_str());
        return ok;
    }

    bool readCurrentProfile(ProfileName &out) {
        char   buf[16] = {0};
        size_t cap     = sizeof(buf);
        if (!storage_.readString(kCurrentProfileKey, buf, cap)) { return false; }
        out = buf;
        return true;
    }

    bool readProfileValues(const ProfileName &name, ProfileValues &out) {
        const auto  k = makeProfileKey_(name);
        ProfileBlob blob{};
        size_t      len = sizeof(blob);
        if (!storage_.readBlob(k, &blob, len)) {
            SS_LOGW("readBlob missing for key=\"%s\"", k.c_str());
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

    bool writeProfileValues(const ProfileName &name, ProfileValues &vals) {
        ProfileBlob blob{};
        blob.version = kBlobVersion;
        blob.pad     = 0;
        blob.values  = vals;
        blob.crc32   = calcCrc_(&blob, offsetof(ProfileBlob, crc32));

        const auto k  = makeProfileKey_(name);
        const bool ok = storage_.writeBlob(k, &blob, sizeof(blob));
        if (!ok)
            SS_LOGE("writeBlob failed for key=\"%s\"", k.c_str());
        else
            SS_LOGI("wrote values of key=\"%s\"", k.c_str());
        return ok;
    }

  private:
    // Build a sanitized key like "p_<lowercase_alnum_underscore>"
    static storage::Key makeProfileKey_(const ProfileName &name) {
        storage::Key key = name;
        etl::to_lower_case(key);
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

  private:
    storage::IStorage    &storage_;
    static constexpr char kCurrentProfileKey[] = "CurrProfile";
};

} // namespace esphome::smart_signage::profile
