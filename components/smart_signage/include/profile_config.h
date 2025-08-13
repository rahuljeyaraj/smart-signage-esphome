// profiles_config_flat.h
#pragma once

#include <ArduinoJson.h> // ArduinoJson v7
#include <etl/flat_map.h>
#include <algorithm>
#include <cstring>

#include "log.h"               // ASCII-only logging
#include "audio/audio_const.h" // audio::kSourceStrLen, audio::kMaxPlaylist
#include "audio/audio_event.h" // audio::AudioPlaySpec

namespace esphome::smart_signage::profiles_config {

// ────────────────────────────────────────────────────────────────────────────
// Event enumeration (plain enum class)
// ────────────────────────────────────────────────────────────────────────────
enum class EventId : uint8_t { SetupDone, SensorError, Start, Detected, Unknown };

// Map enum -> string (switch-case)
inline const char *eventToCStr(EventId e) {
    switch (e) {
    case EventId::SetupDone: return "setup_done";
    case EventId::SensorError: return "sensor_error";
    case EventId::Start: return "start";
    case EventId::Detected: return "detected";
    default: return "unknown";
    }
}

// Map string -> enum (strcmp chain)
inline EventId eventFromCStr(const char *s) {
    if (!s || s[0] == '\0') return EventId::Unknown;
    if (std::strcmp(s, "setup_done") == 0) return EventId::SetupDone;
    if (std::strcmp(s, "sensor_error") == 0) return EventId::SensorError;
    if (std::strcmp(s, "start") == 0) return EventId::Start;
    if (std::strcmp(s, "detected") == 0) return EventId::Detected;
    return EventId::Unknown;
}

// ────────────────────────────────────────────────────────────────────────────
/** Composite key: (profileIdx, event) for the flat_map */
struct Key {
    uint16_t profile{0};
    EventId  event{EventId::Unknown};

    // flat_map needs strict weak ordering
    bool operator<(const Key &other) const {
        if (profile < other.profile) return true;
        if (profile > other.profile) return false;
        return static_cast<uint8_t>(event) < static_cast<uint8_t>(other.event);
    }
};

// ────────────────────────────────────────────────────────────────────────────
// ProfilesConfig: construct-empty, init(json) later, heap-free lookups
// Choose capacities via template args.
//   MAX_PROFILES:       maximum number of profiles you expect
//   MAX_EVENTS_TOTAL:   total (profile,event) pairs with audio you expect
// ────────────────────────────────────────────────────────────────────────────
template <size_t MAX_PROFILES, size_t MAX_EVENTS_TOTAL>
class ProfilesConfig {
  public:
    ProfilesConfig() = default;

    /** Clear current data and load from JSON. Returns false on first failure. */
    bool init(const char *jsonUtf8) {
        clear();

        if (!jsonUtf8 || jsonUtf8[0] == '\0') {
            LOGE("%s: input json is null or empty", TAG);
            return false;
        }

        // Parse into a temporary doc (v7 heap used only here)
        JsonDocument         doc;
        DeserializationError err = deserializeJson(doc, jsonUtf8);
        if (err) {
            LOGE("%s: deserializeJson failed: %s", TAG, err.c_str());
            return false;
        }

        auto profiles = doc["profiles"].as<JsonArrayConst>();
        if (profiles.isNull()) {
            LOGE("%s: 'profiles' missing or not an array", TAG);
            return false;
        }

        const size_t psize = profiles.size();
        if (psize > MAX_PROFILES) {
            LOGE("%s: profiles size %zu exceeds MAX_PROFILES %zu", TAG, psize, MAX_PROFILES);
            // Not fatal; we ingest up to MAX_PROFILES.
        }

        size_t ingestedPairs = 0;

        for (size_t pi = 0; pi < std::min(psize, (size_t) MAX_PROFILES); ++pi) {
            auto profile = profiles[pi].as<JsonObjectConst>();
            if (profile.isNull()) {
                LOGE("%s: profile %zu is not an object", TAG, pi);
                continue;
            }

            auto events = profile["events"].as<JsonObjectConst>();
            if (events.isNull()) {
                LOGE("%s: profile %zu has no 'events' object", TAG, pi);
                continue;
            }

            // Iterate event entries in this profile
            for (auto kvp : events) {
                const char *evKey = kvp.key().c_str();
                auto        evObj = kvp.value().as<JsonObjectConst>();

                EventId ev = eventFromCStr(evKey);
                if (ev == EventId::Unknown) {
                    LOGE("%s: unknown event '%s' in profile %zu", TAG, evKey, pi);
                    continue;
                }

                auto audioObj = evObj["audio"].as<JsonObjectConst>();
                if (audioObj.isNull()) {
                    // No audio for this event; skip (not an error).
                    continue;
                }

                audio::AudioPlaySpec spec{}; // zero-initialized
                // playCnt -> loopCount (0 = infinite)
                uint32_t playCnt = audioObj["playCnt"] | 1u;
                spec.loopCount   = static_cast<uint16_t>(std::min<uint32_t>(playCnt, 0xFFFFu));

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
                            LOGE("%s: profile %zu '%s' playList[%zu] not an object",
                                TAG,
                                pi,
                                evKey,
                                i);
                            continue;
                        }
                        const char *src = item["src"] | "";
                        if (src[0] == '\0') {
                            LOGE("%s: profile %zu '%s' playList[%zu] missing src",
                                TAG,
                                pi,
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
                    LOGE("%s: (profile,event) pairs exceed MAX_EVENTS_TOTAL=%zu; ingestion stopped",
                        TAG,
                        MAX_EVENTS_TOTAL);
                    return false; // caller should increase capacity
                }

                if (table_.full()) {
                    LOGE("%s: flat_map full (MAX_EVENTS_TOTAL=%zu)", TAG, MAX_EVENTS_TOTAL);
                    return false;
                }

                const Key k{static_cast<uint16_t>(pi), ev};
                auto      it = table_.find(k);
                if (it == table_.end()) {
                    const bool inserted = table_.insert(std::make_pair(k, spec)).second;
                    if (!inserted) {
                        LOGE(
                            "%s: flat_map insert failed at profile %zu event '%s'", TAG, pi, evKey);
                        return false;
                    }
                    ++ingestedPairs;
                } else {
                    // Duplicate (profile,event) -> replace
                    it->second = spec;
                }
            }
        }

        return true;
    }

    /** Heap-free lookup at runtime */
    bool getAudioPlaySpec(uint32_t profileIdx, EventId ev, audio::AudioPlaySpec &out) const {
        if (profileIdx > 0xFFFFu) {
            LOGE("%s: profileIdx too large: %u", TAG, static_cast<unsigned int>(profileIdx));
            return false;
        }

        const Key k{static_cast<uint16_t>(profileIdx), ev};
        auto      it = table_.find(k);
        if (it == table_.end()) {
            LOGE("%s: not found (profile=%u, event=%s)",
                TAG,
                static_cast<unsigned int>(profileIdx),
                eventToCStr(ev));
            return false;
        }
        out = it->second; // POD copy
        return true;
    }

    void clear() { table_.clear(); }

  private:
    static constexpr const char *TAG = "ProfilesConfig";

    etl::flat_map<Key, audio::AudioPlaySpec, MAX_EVENTS_TOTAL> table_;
};

} // namespace esphome::smart_signage::profiles_config
