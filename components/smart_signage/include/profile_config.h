// profiles_config_flat.h  (aka profile_config.h)
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
#include "common.h"

namespace esphome::smart_signage {

// ────────────────────────────────────────────────────────────────────────────
// Events (camelCase)
// ────────────────────────────────────────────────────────────────────────────
enum class EventId : uint8_t {
    SetupDone,
    Error,
    Start,
    Stop,
    UiUpdate,
    Detected,
    Detected0Cm,
    Fell,
    Rose,
    SessionEnd,
    Unknown
};

inline const char *eventToCStr(EventId e) {
    switch (e) {
    case EventId::SetupDone: return "SetupDone";
    case EventId::Error: return "Error";
    case EventId::Start: return "Start";
    case EventId::Stop: return "Stop";
    case EventId::UiUpdate: return "UiUpdate";
    case EventId::Detected: return "Detected";
    case EventId::Detected0Cm: return "Detected0Cm";
    case EventId::Fell: return "Fell";
    case EventId::Rose: return "Rose";
    case EventId::SessionEnd: return "SessionEnd";
    default: return "Unknown";
    }
}

inline bool streq_(const char *a, const char *b) { return std::strcmp(a, b) == 0; }

inline EventId eventFromCStr(const char *s) {
    if (!s || s[0] == '\0') return EventId::Unknown;
    if (streq_(s, "SetupDone")) return EventId::SetupDone;
    if (streq_(s, "Error")) return EventId::Error;
    if (streq_(s, "Start")) return EventId::Start;
    if (streq_(s, "Stop")) return EventId::Stop;
    if (streq_(s, "UiUpdate")) return EventId::UiUpdate;
    if (streq_(s, "Detected")) return EventId::Detected;
    if (streq_(s, "Detected0Cm")) return EventId::Detected0Cm;
    if (streq_(s, "Fell")) return EventId::Fell;
    if (streq_(s, "Rose")) return EventId::Rose;
    if (streq_(s, "SessionEnd")) return EventId::SessionEnd;
    return EventId::Unknown;
}

// ────────────────────────────────────────────────────────────────────────────
// LED spec
// ────────────────────────────────────────────────────────────────────────────
enum class LedPattern : uint8_t { Unknown, Square, Triangle };

inline const char *ledPatternToCStr(LedPattern p) {
    switch (p) {
    case LedPattern::Square: return "square";
    case LedPattern::Triangle: return "triangle";
    default: return "unknown";
    }
}

inline LedPattern ledPatternFromCStr(const char *s) {
    if (!s || s[0] == '\0') return LedPattern::Unknown;
    if (std::strcmp(s, "square") == 0) return LedPattern::Square;
    if (std::strcmp(s, "triangle") == 0) return LedPattern::Triangle;
    return LedPattern::Unknown;
}

struct LedPlaySpec {
    LedPattern pattern{LedPattern::Unknown};
    uint16_t   periodMs{0}; // blink period
    uint16_t   cnt{0};      // 0 = infinite
};

// Composite key: (profileName, event)
struct Key {
    ProfileName name{};
    EventId     event{EventId::Unknown};

    bool operator<(const Key &other) const {
        if (name < other.name) return true;
        if (other.name < name) return false;
        return static_cast<uint8_t>(event) < static_cast<uint8_t>(other.event);
    }
};

template <size_t MAX_PROFILES, size_t MAX_EVENTS_TOTAL>
class ProfilesConfig {
  public:
    static constexpr const char *TAG = "ProfilesConfig";

    ProfilesConfig() = default;

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
        }

        for (size_t pi = 0; pi < std::min(psize, (size_t) MAX_PROFILES); ++pi) {
            auto profile = profiles[pi].as<JsonObjectConst>();
            if (profile.isNull()) {
                SS_LOGE("%s: profile %zu is not an object", TAG, pi);
                continue;
            }

            const char *name = profile["name"] | "";
            ProfileName pname;
            if (name[0] != '\0') {
                pname = name; // ETL truncates to 15
            } else {
                char buf[20];
                std::snprintf(buf, sizeof(buf), "Profile %u", (unsigned) pi);
                pname = buf;
            }
            profileNames_[pi] = pname;

            auto events = profile["events"].as<JsonObjectConst>();
            if (events.isNull()) {
                SS_LOGE("%s: profile '%s' has no 'events' object", TAG, pname.c_str());
                continue;
            }

            for (auto kvp : events) {
                const char *evKey = kvp.key().c_str();
                auto        evObj = kvp.value().as<JsonObjectConst>();

                EventId ev = eventFromCStr(evKey);
                if (ev == EventId::Unknown) {
                    SS_LOGE("%s: profile '%s' has unknown event '%s'", TAG, pname.c_str(), evKey);
                    continue;
                }

                // AUDIO
                if (auto audioObj = evObj["audio"].as<JsonObjectConst>(); !audioObj.isNull()) {
                    audio::AudioPlaySpec spec{};
                    uint32_t             playCnt = audioObj["playCnt"] | 1u;
                    spec.playCnt = static_cast<uint16_t>(std::min<uint32_t>(playCnt, 0xFFFFu));

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

                    if (audioTable_.full()) {
                        SS_LOGE("%s: audio flat_map full (MAX_EVENTS_TOTAL=%zu)",
                            TAG,
                            MAX_EVENTS_TOTAL);
                        return false;
                    }
                    const Key k{pname, ev};
                    auto      it = audioTable_.find(k);
                    if (it == audioTable_.end()) {
                        if (!audioTable_.insert(std::make_pair(k, spec)).second) {
                            SS_LOGE("%s: audio insert failed at profile '%s' event '%s'",
                                TAG,
                                pname.c_str(),
                                evKey);
                            return false;
                        }
                    } else {
                        it->second = spec;
                    }
                }

                // LED
                if (auto ledObj = evObj["led"].as<JsonObjectConst>(); !ledObj.isNull()) {
                    LedPlaySpec lspec{};
                    lspec.pattern  = ledPatternFromCStr((const char *) (ledObj["pattern"] | ""));
                    uint32_t pms   = ledObj["periodMs"] | 0u;
                    uint32_t cnt   = ledObj["cnt"] | (ledObj["nt"] | 0u); // accept "nt"
                    lspec.periodMs = static_cast<uint16_t>(std::min<uint32_t>(pms, 0xFFFFu));
                    lspec.cnt      = static_cast<uint16_t>(std::min<uint32_t>(cnt, 0xFFFFu));

                    if (ledTable_.full()) {
                        SS_LOGE(
                            "%s: led flat_map full (MAX_EVENTS_TOTAL=%zu)", TAG, MAX_EVENTS_TOTAL);
                        return false;
                    }
                    const Key k{pname, ev};
                    auto      it = ledTable_.find(k);
                    if (it == ledTable_.end()) {
                        if (!ledTable_.insert(std::make_pair(k, lspec)).second) {
                            SS_LOGE("%s: led insert failed at profile '%s' event '%s'",
                                TAG,
                                pname.c_str(),
                                evKey);
                            return false;
                        }
                    } else {
                        it->second = lspec;
                    }
                }
            }
        }

        dumpAll_();
        return true;
    }

    bool getAudioPlaySpec(
        const ProfileName &profileName, EventId ev, audio::AudioPlaySpec &out) const {
        const Key k{profileName, ev};
        auto      it = audioTable_.find(k);
        if (it == audioTable_.end()) {
            SS_LOGE("%s: audio not found (profile='%s', event=%s)",
                TAG,
                profileName.c_str(),
                eventToCStr(ev));
            return false;
        }
        out = it->second;
        return true;
    }

    bool getLedPlaySpec(const ProfileName &profileName, EventId ev, LedPlaySpec &out) const {
        const Key k{profileName, ev};
        auto      it = ledTable_.find(k);
        if (it == ledTable_.end()) {
            SS_LOGE("%s: led not found (profile='%s', event=%s)",
                TAG,
                profileName.c_str(),
                eventToCStr(ev));
            return false;
        }
        out = it->second;
        return true;
    }

    void clear() {
        audioTable_.clear();
        ledTable_.clear();
        for (size_t i = 0; i < MAX_PROFILES; ++i) profileNames_[i].clear();
    }

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
        SS_LOGD("%s: dump begin (audio=%zu, led=%zu)", TAG, audioTable_.size(), ledTable_.size());

        // Print rows for all audio keys, merging LED if present
        for (auto it = audioTable_.begin(); it != audioTable_.end(); ++it) {
            const Key  &k  = it->first;
            const auto &as = it->second;
            const auto  lt = ledTable_.find(k);
            if (lt != ledTable_.end()) {
                const auto &ls = lt->second;
                SS_LOGD("%s: profile='%s' event=%s | audio{cnt=%hu,n=%hhu} "
                        "led{pattern=%s,periodMs=%hu,cnt=%hu}",
                    TAG,
                    k.name.c_str(),
                    eventToCStr(k.event),
                    as.playCnt,
                    as.n,
                    ledPatternToCStr(ls.pattern),
                    ls.periodMs,
                    ls.cnt);
            } else {
                SS_LOGD("%s: profile='%s' event=%s | audio{cnt=%hu,n=%hhu} led{-}",
                    TAG,
                    k.name.c_str(),
                    eventToCStr(k.event),
                    as.playCnt,
                    as.n);
            }
        }

        // Print rows for LED-only keys
        for (auto it = ledTable_.begin(); it != ledTable_.end(); ++it) {
            const Key &k = it->first;
            if (audioTable_.find(k) != audioTable_.end()) continue; // already printed above
            const auto &ls = it->second;
            SS_LOGD("%s: profile='%s' event=%s | audio{-} led{pattern=%s,periodMs=%hu,cnt=%hu}",
                TAG,
                k.name.c_str(),
                eventToCStr(k.event),
                ledPatternToCStr(ls.pattern),
                ls.periodMs,
                ls.cnt);
        }

        SS_LOGD("%s: dump end", TAG);
    }

    etl::flat_map<Key, audio::AudioPlaySpec, MAX_EVENTS_TOTAL> audioTable_;
    etl::flat_map<Key, LedPlaySpec, MAX_EVENTS_TOTAL>          ledTable_;
    etl::array<ProfileName, MAX_PROFILES>                      profileNames_;
};

using ProfilesConfigT = ProfilesConfig<SS_MAX_PROFILES, SS_MAX_EVENTS_TOTAL>;

} // namespace esphome::smart_signage
