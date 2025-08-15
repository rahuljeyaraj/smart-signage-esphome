#pragma once

#include <ArduinoJson.h> // ArduinoJson v7
#include <etl/flat_map.h>
#include <etl/vector.h>
#include <algorithm>
#include <cstring>

#include "log.h"                   // SS_LOGx (ASCII-only)
#include "audio/audio_const.h"     // audio::kSourceStrLen, audio::kMaxPlaylist
#include "audio/audio_play_spec.h" // audio::AudioPlaySpec
#include "profile/profile_defs.h" // SS_MAX_PROFILES, SS_MAX_EVENTS_TOTAL, ProfileName, ProfileNames
#include "led/led_pattern_spec.h" // led::LedPatternSpec, ledPatternFromCStr, ledPatternToCStr

namespace esphome::smart_signage::profile {

// ────────────────────────────────────────────────────────────────────────────
// Events (camelCase)
// ────────────────────────────────────────────────────────────────────────────
enum class EventId : uint8_t {
    Ready,
    Error,
    Start,
    Stop,
    UiUpdate,
    Clear,
    Detected,
    DetectedDistanceMax,
    DetectedDistanceMin,
    Fell,
    Rose,
    SessionEnd,
    Unknown
};

inline const char *eventToCStr(EventId e) {
    switch (e) {
    case EventId::Ready: return "Ready";
    case EventId::Error: return "Error";
    case EventId::Start: return "Start";
    case EventId::Stop: return "Stop";
    case EventId::UiUpdate: return "UiUpdate";
    case EventId::Clear: return "Clear";
    case EventId::Detected: return "Detected";
    case EventId::DetectedDistanceMax: return "DetectedDistanceMax";
    case EventId::DetectedDistanceMin: return "DetectedDistanceMin";
    case EventId::Fell: return "Fell";
    case EventId::Rose: return "Rose";
    case EventId::SessionEnd: return "SessionEnd";
    default: return "Unknown";
    }
}

inline bool streq_(const char *a, const char *b) { return std::strcmp(a, b) == 0; }

inline EventId eventFromCStr(const char *s) {
    if (!s || s[0] == '\0') return EventId::Unknown;
    if (streq_(s, "Ready")) return EventId::Ready;
    if (streq_(s, "Error")) return EventId::Error;
    if (streq_(s, "Start")) return EventId::Start;
    if (streq_(s, "Stop")) return EventId::Stop;
    if (streq_(s, "UiUpdate")) return EventId::UiUpdate;
    if (streq_(s, "Clear")) return EventId::Clear;
    if (streq_(s, "Detected")) return EventId::Detected;
    if (streq_(s, "DetectedDistanceMax")) return EventId::DetectedDistanceMax;
    if (streq_(s, "DetectedDistanceMin")) return EventId::DetectedDistanceMin;
    if (streq_(s, "Fell")) return EventId::Fell;
    if (streq_(s, "Rose")) return EventId::Rose;
    if (streq_(s, "SessionEnd")) return EventId::SessionEnd;
    return EventId::Unknown;
}

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
class ProfileCatalogT {
  public:
    static constexpr char TAG[] = "ProfileCatalog";

    ProfileCatalogT() = default;

    // Parse and fill from JSON string; returns true on success.
    bool init(const char *jsonUtf8) {
        clear();

        if (!jsonUtf8 || jsonUtf8[0] == '\0') {
            SS_LOGE("input json is null or empty");
            return false;
        }

        ArduinoJson::JsonDocument doc;
        auto                      err = deserializeJson(doc, jsonUtf8);
        if (err) {
            SS_LOGE("deserializeJson failed: %s", err.c_str());
            return false;
        }

        auto profiles = doc["profiles"].as<ArduinoJson::JsonArrayConst>();
        if (profiles.isNull()) {
            SS_LOGE("'profiles' missing or not an array");
            return false;
        }

        const size_t psize = profiles.size();
        if (psize > MAX_PROFILES) {
            SS_LOGE("profiles size %zu exceeds MAX_PROFILES %zu", psize, (size_t) MAX_PROFILES);
        }

        // Build profile name list using push_back (no raw indexing).
        for (size_t pi = 0; pi < std::min(psize, (size_t) MAX_PROFILES); ++pi) {
            auto profile = profiles[pi].as<ArduinoJson::JsonObjectConst>();
            if (profile.isNull()) {
                SS_LOGE("profile %zu is not an object", pi);
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

            if (!profileNames_.full()) {
                profileNames_.push_back(pname);
            } else {
                SS_LOGW("profile name list full; dropping '%s'", pname.c_str());
            }

            auto events = profile["events"].as<ArduinoJson::JsonObjectConst>();
            if (events.isNull()) {
                SS_LOGE("profile '%s' has no 'events' object", pname.c_str());
                continue;
            }

            for (auto kvp : events) {
                const char *evKey = kvp.key().c_str();
                auto        evObj = kvp.value().as<ArduinoJson::JsonObjectConst>();

                EventId ev = eventFromCStr(evKey);
                if (ev == EventId::Unknown) {
                    SS_LOGE(
                        "profile '%s' has unknown event '%s'", pname.c_str(), evKey ? evKey : "");
                    continue;
                }

                // AUDIO
                if (auto audioObj = evObj["audio"].as<ArduinoJson::JsonObjectConst>();
                    !audioObj.isNull()) {
                    audio::AudioPlaySpec spec{};
                    uint32_t             playCnt = audioObj["playCnt"] | 1u;
                    spec.playCnt = static_cast<uint16_t>(std::min<uint32_t>(playCnt, 0xFFFFu));

                    auto list = audioObj["playList"].as<ArduinoJson::JsonArrayConst>();
                    if (!list.isNull()) {
                        const size_t  listSz = list.size();
                        const uint8_t maxN =
                            static_cast<uint8_t>(std::min<size_t>(audio::kMaxPlaylist, listSz));
                        uint8_t n = 0;
                        for (size_t i = 0; i < maxN; ++i) {
                            auto item = list[i].as<ArduinoJson::JsonObjectConst>();
                            if (item.isNull()) {
                                SS_LOGE("profile '%s' '%s' playList[%zu] not an object",
                                    pname.c_str(),
                                    evKey,
                                    i);
                                continue;
                            }
                            const char *src = item["src"] | "";
                            if (src[0] == '\0') {
                                SS_LOGE("profile '%s' '%s' playList[%zu] missing src",
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
                        SS_LOGE("audio flat_map full (MAX_EVENTS_TOTAL=%zu)",
                            (size_t) MAX_EVENTS_TOTAL);
                        return false;
                    }
                    const Key k{pname, ev};
                    auto      it = audioTable_.find(k);
                    if (it == audioTable_.end()) {
                        if (!audioTable_.insert(std::make_pair(k, spec)).second) {
                            SS_LOGE("audio insert failed at profile '%s' event '%s'",
                                pname.c_str(),
                                evKey);
                            return false;
                        }
                    } else {
                        it->second = spec;
                    }
                }

                // LED
                if (auto ledObj = evObj["led"].as<ArduinoJson::JsonObjectConst>();
                    !ledObj.isNull()) {
                    led::LedPatternSpec lspec{};
                    lspec.pattern =
                        led::ledPatternFromCStr((const char *) (ledObj["pattern"] | ""));
                    uint32_t pms   = ledObj["periodMs"] | 0u;
                    uint32_t cnt   = ledObj["cnt"] | (ledObj["nt"] | 0u); // accept legacy "nt"
                    lspec.periodMs = static_cast<uint16_t>(std::min<uint32_t>(pms, 0xFFFFu));
                    lspec.cnt      = static_cast<uint16_t>(std::min<uint32_t>(cnt, 0xFFFFu));

                    if (ledTable_.full()) {
                        SS_LOGE(
                            "led flat_map full (MAX_EVENTS_TOTAL=%zu)", (size_t) MAX_EVENTS_TOTAL);
                        return false;
                    }
                    const Key k{pname, ev};
                    auto      it = ledTable_.find(k);
                    if (it == ledTable_.end()) {
                        if (!ledTable_.insert(std::make_pair(k, lspec)).second) {
                            SS_LOGE("led insert failed at profile '%s' event '%s'",
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

        if (profileNames_.empty()) {
            SS_LOGW("No profiles found");
        } else {
            SS_LOGI("Available Profiles(cnt: %u):", (unsigned) profileNames_.size());
            for (size_t i = 0; i < profileNames_.size(); ++i) {
                SS_LOGI("\t%u: %s", (unsigned) i, profileNames_[i].c_str());
            }
        }

        // dumpAll_();
        return true;
    }

    bool getAudioPlaySpec(
        const ProfileName &profileName, EventId ev, audio::AudioPlaySpec &out) const {
        const Key k{profileName, ev};
        auto      it = audioTable_.find(k);
        if (it == audioTable_.end()) {
            SS_LOGE(
                "audio not found (profile='%s', event=%s)", profileName.c_str(), eventToCStr(ev));
            return false;
        }
        out = it->second;
        return true;
    }

    bool getLedPatternSpec(
        const ProfileName &profileName, EventId ev, led::LedPatternSpec &out) const {
        const Key k{profileName, ev};
        auto      it = ledTable_.find(k);
        if (it == ledTable_.end()) {
            SS_LOGE("led not found (profile='%s', event=%s)", profileName.c_str(), eventToCStr(ev));
            return false;
        }
        out = it->second;
        return true;
    }

    void clear() {
        audioTable_.clear();
        ledTable_.clear();
        profileNames_.clear(); // safe reset; avoids touching internals of contained strings
    }

    void getProfileNames(ProfileNames &out) const {
        out = profileNames_; // ETL vector supports copy assignment
    }

  private:
    void dumpAll_() const {
        SS_LOGD("dump begin (audio=%zu, led=%zu)",
            (size_t) audioTable_.size(),
            (size_t) ledTable_.size());

        // Print rows for all audio keys, merging LED if present
        for (auto it = audioTable_.begin(); it != audioTable_.end(); ++it) {
            const Key  &k  = it->first;
            const auto &as = it->second;
            const auto  lt = ledTable_.find(k);
            if (lt != ledTable_.end()) {
                const auto &ls = lt->second;
                SS_LOGD("profile='%s' event=%s | audio{cnt=%hu,n=%hhu} "
                        "led{pattern=%s,periodMs=%hu,cnt=%hu}",
                    k.name.c_str(),
                    eventToCStr(k.event),
                    as.playCnt,
                    as.n,
                    ledPatternToCStr(ls.pattern),
                    ls.periodMs,
                    ls.cnt);
            } else {
                SS_LOGD("profile='%s' event=%s | audio{cnt=%hu,n=%hhu} led{-}",
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
            SS_LOGD("profile='%s' event=%s | audio{-} led{pattern=%s,periodMs=%hu,cnt=%hu}",
                k.name.c_str(),
                eventToCStr(k.event),
                ledPatternToCStr(ls.pattern),
                ls.periodMs,
                ls.cnt);
        }

        SS_LOGD("dump end");
    }

    etl::flat_map<Key, audio::AudioPlaySpec, MAX_EVENTS_TOTAL> audioTable_;
    etl::flat_map<Key, led::LedPatternSpec, MAX_EVENTS_TOTAL>  ledTable_;
    ProfileNames                                               profileNames_;
};

// Bind to your project-wide capacities from common.h
using ProfileCatalog = ProfileCatalogT<SS_MAX_PROFILES, SS_MAX_EVENTS_TOTAL>;

} // namespace esphome::smart_signage::profile
