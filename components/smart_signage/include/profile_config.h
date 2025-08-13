#pragma once

#include <ArduinoJson.h> // ArduinoJson v7
#include <etl/flat_map.h>
#include <etl/array.h>
#include <etl/vector.h>
#include <algorithm>
#include <cstring>

#include "log.h"               // SS_LOGx (ASCII-only)
#include "audio/audio_const.h" // audio::kSourceStrLen, audio::kMaxPlaylist
#include "audio/audio_event.h" // audio::AudioPlaySpec
#include "types.h"

namespace esphome::smart_signage {

// ────────────────────────────────────────────────────────────────────────────
// Event enumeration (plain enum class)
// ────────────────────────────────────────────────────────────────────────────
enum class EventId : uint8_t { SetupDone, SensorError, Start, Detected, Unknown };

inline const char *eventToCStr(EventId e) {
    switch (e) {
    case EventId::SetupDone: return "setup_done";
    case EventId::SensorError: return "sensor_error";
    case EventId::Start: return "start";
    case EventId::Detected: return "detected";
    default: return "unknown";
    }
}

inline EventId eventFromCStr(const char *s) {
    if (!s || s[0] == '\0') return EventId::Unknown;
    if (std::strcmp(s, "setup_done") == 0) return EventId::SetupDone;
    if (std::strcmp(s, "sensor_error") == 0) return EventId::SensorError;
    if (std::strcmp(s, "start") == 0) return EventId::Start;
    if (std::strcmp(s, "detected") == 0) return EventId::Detected;
    return EventId::Unknown;
}

// ────────────────────────────────────────────────────────────────────────────
/** Composite key: (profileName, event) for the flat_map */
struct Key {
    ProfileName name{};
    EventId     event{EventId::Unknown};

    // flat_map needs strict weak ordering
    bool operator<(const Key &other) const {
        if (name < other.name) return true;
        if (other.name < name) return false; // ensure strict ordering
        return static_cast<uint8_t>(event) < static_cast<uint8_t>(other.event);
    }
};

// ────────────────────────────────────────────────────────────────────────────
// ProfilesConfig
//   • Parses JSON (ArduinoJson v7) at init()
//   • Stores audio specs keyed by (ProfileName, EventId)
//   • getProfileList(...) returns the list of ProfileName (no indices)
// ────────────────────────────────────────────────────────────────────────────
template <size_t MAX_PROFILES, size_t MAX_EVENTS_TOTAL>
class ProfilesConfig {
  public:
    static constexpr const char *TAG = "ProfilesConfig";

    ProfilesConfig() = default;

    /** Clear current data and load from JSON. Returns false on first failure. */
    bool init(const char *jsonUtf8) {
        clear();

        if (!jsonUtf8 || jsonUtf8[0] == '\0') {
            SS_LOGE("%s: input json is null or empty", TAG);
            return false;
        }

        JsonDocument         doc;
        DeserializationError err = deserializeJson(doc, jsonUtf8);
        if (err) {
            SS_LOGE("%s: deserializeJson failed: %s", TAG, err.c_str());
            return false;
        }

        auto profiles = doc["profiles"].as<JsonArrayConst>();
        if (profiles.isNull()) {
            SS_LOGE("%s: 'profiles' missing or not an array", TAG);
            return false;
        }

        const size_t psize = profiles.size();
        if (psize > MAX_PROFILES) {
            SS_LOGE("%s: profiles size %zu exceeds MAX_PROFILES %zu", TAG, psize, MAX_PROFILES);
            // Not fatal; we ingest up to MAX_PROFILES.
        }

        size_t ingestedPairs = 0;

        for (size_t pi = 0; pi < std::min(psize, (size_t) MAX_PROFILES); ++pi) {
            auto profile = profiles[pi].as<JsonObjectConst>();
            if (profile.isNull()) {
                SS_LOGE("%s: profile %zu is not an object", TAG, pi);
                continue;
            }

            // ── Profile name (truncate to 15 chars if longer) ──
            const char *name = profile["name"] | "";
            ProfileName pname;
            if (name[0] != '\0') {
                pname = name; // ETL string truncates to capacity
            } else {
                // Fallback: "Profile <pi>"
                char buf[20];
                std::snprintf(buf, sizeof(buf), "Profile %u", (unsigned) pi);
                pname = buf;
            }

            // Store the name for list API
            profileNames_[pi] = pname;

            auto events = profile["events"].as<JsonObjectConst>();
            if (events.isNull()) {
                SS_LOGE("%s: profile '%s' has no 'events' object", TAG, pname.c_str());
                continue;
            }

            // Iterate event entries in this profile
            for (auto kvp : events) {
                const char *evKey = kvp.key().c_str();
                auto        evObj = kvp.value().as<JsonObjectConst>();

                EventId ev = eventFromCStr(evKey);
                if (ev == EventId::Unknown) {
                    SS_LOGE("%s: profile '%s' has unknown event '%s'", TAG, pname.c_str(), evKey);
                    continue;
                }

                auto audioObj = evObj["audio"].as<JsonObjectConst>();
                if (audioObj.isNull()) {
                    // No audio for this event; skip (not an error).
                    continue;
                }

                audio::AudioPlaySpec spec{}; // zero-initialized
                // playCnt -> playCnt (0 = infinite)
                uint32_t playCnt = audioObj["playCnt"] | 1u;
                spec.playCnt     = static_cast<uint16_t>(std::min<uint32_t>(playCnt, 0xFFFFu));

                // playList
                auto list = audioObj["playList"].as<JsonArrayConst>();
                if (!list.isNull()) {
                    const size_t  listSz = list.size();
                    const uint8_t maxN =
                        static_cast<uint8_t>(std::min<size_t>(audio::kMaxPlaylist, listSz));

                    uint8_t n = 0;
                    for (size_t i = 0; i < maxN; ++i) {
                        auto item = list[i].as<JsonObjectConst>();
                        if (item.isNull()) {
                            SS_LOGE("%s: profile '%s' '%s' playList[%zu] not an object",
                                TAG,
                                pname.c_str(),
                                evKey,
                                i);
                            continue;
                        }
                        const char *src = item["src"] | "";
                        if (src[0] == '\0') {
                            SS_LOGE("%s: profile '%s' '%s' playList[%zu] missing src",
                                TAG,
                                pname.c_str(),
                                evKey,
                                i);
                            continue;
                        }
                        uint32_t dms = item["delayMs"] | 0u;

                        std::strncpy(spec.items[n].source, src, audio::kSourceStrLen - 1);
                        spec.items[n].source[audio::kSourceStrLen - 1] = '\0';
                        spec.items[n].gapMs =
                            static_cast<uint16_t>(std::min<uint32_t>(dms, 0xFFFFu));
                        ++n;
                    }
                    spec.n = n;
                }

                if (ingestedPairs >= MAX_EVENTS_TOTAL) {
                    SS_LOGE("%s: (profile,event) pairs exceed MAX_EVENTS_TOTAL=%zu; stop",
                        TAG,
                        MAX_EVENTS_TOTAL);
                    return false; // caller should increase capacity
                }

                if (table_.full()) {
                    SS_LOGE("%s: flat_map full (MAX_EVENTS_TOTAL=%zu)", TAG, MAX_EVENTS_TOTAL);
                    return false;
                }

                const Key k{pname, ev};
                auto      it = table_.find(k);
                if (it == table_.end()) {
                    const bool inserted = table_.insert(std::make_pair(k, spec)).second;
                    if (!inserted) {
                        SS_LOGE("%s: flat_map insert failed at profile '%s' event '%s'",
                            TAG,
                            pname.c_str(),
                            evKey);
                        return false;
                    }
                    ++ingestedPairs;
                } else {
                    // Duplicate (profile,event) -> replace
                    it->second = spec;
                }
            }
        }

        dumpAll_();
        return true;
    }

    /** Heap-free lookup at runtime by profile NAME (no index). */
    bool getAudioPlaySpec(
        const ProfileName &profileName, EventId ev, audio::AudioPlaySpec &out) const {
        const Key k{profileName, ev};
        auto      it = table_.find(k);
        if (it == table_.end()) {
            SS_LOGE("%s: not found (profile='%s', event=%s)",
                TAG,
                profileName.c_str(),
                eventToCStr(ev));
            return false;
        }
        out = it->second; // POD copy
        return true;
    }

    void clear() {
        table_.clear();
        for (size_t i = 0; i < MAX_PROFILES; ++i) profileNames_[i].clear();
    }

    // Return list of profile NAMES (labels). No indices.
    void getProfileList(etl::vector<ProfileName, MAX_PROFILES> &out) const {
        out.clear();
        for (uint32_t i = 0; i < MAX_PROFILES; ++i) {
            if (!profileNames_[i].empty()) {
                out.push_back(profileNames_[i]);
                if (out.full()) break;
            }
        }
    }

  private:
    void dumpAll_() const {
        SS_LOGD("%s: dump begin (entries=%zu)", TAG, table_.size());
        for (auto it = table_.begin(); it != table_.end(); ++it) {
            const Key  &k    = it->first;
            const auto &spec = it->second;
            SS_LOGD("%s: profile='%s', event=%s, playCnt=%hu, n=%hhu",
                TAG,
                k.name.c_str(),
                eventToCStr(k.event),
                spec.playCnt,
                spec.n);
            for (uint8_t i = 0; i < spec.n; ++i) {
                SS_LOGD("%s:   [%hhu] src=\"%s\", gapMs=%hu",
                    TAG,
                    i,
                    spec.items[i].source,
                    spec.items[i].gapMs);
            }
        }
        SS_LOGD("%s: dump end", TAG);
    }

    etl::flat_map<Key, audio::AudioPlaySpec, MAX_EVENTS_TOTAL> table_;
    etl::array<ProfileName, MAX_PROFILES>                      profileNames_{};
};

} // namespace esphome::smart_signage
