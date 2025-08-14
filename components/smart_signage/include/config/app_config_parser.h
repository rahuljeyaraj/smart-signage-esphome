// #pragma once

// #include <ArduinoJson.h> // ArduinoJson v7
// #include "log.h"
// #include "config/app_config.h"
// #include "audio/audio_event.h" // audio::AudioPlaySpec
// #include "led/led_play_spec.h" // led::LedPlaySpec

// namespace esphome::smart_signage {

// static constexpr char TAG[] = "AppConfigParser";

// // Expectation: your audio layer provides this helper already.
// namespace audio {
//     bool parseAudioSpec(ArduinoJson::JsonVariantConst, AudioPlaySpec &);
// }

// // JSON string -> AppConfig
// inline bool parseAppConfig(const char *jsonUtf8, AppConfig &out) {
//     out = AppConfig{}; // clear

//     if (!jsonUtf8 || jsonUtf8[0] == '\0') {
//         SS_LOGE("input json is null or empty");
//         return false;
//     }

//     ArduinoJson::JsonDocument doc;
//     auto                      err = deserializeJson(doc, jsonUtf8);
//     if (err) {
//         SS_LOGE("JSON parse error %d", (int) err.code());
//         return false;
//     }

//     auto profiles = doc["profiles"].as<ArduinoJson::JsonArrayConst>();
//     if (profiles.isNull()) {
//         SS_LOGE("'profiles' missing or not an array");
//         return false;
//     }

//     const size_t psize = profiles.size();
//     if (psize > SS_MAX_PROFILES) {
//         SS_LOGE("profiles size %zu exceeds SS_MAX_PROFILES %zu", psize, (size_t) SS_MAX_PROFILES);
//     }

//     size_t wroteProfiles = 0;
//     for (size_t pi = 0; pi < std::min(psize, (size_t) SS_MAX_PROFILES); ++pi) {
//         auto profile = profiles[pi].as<ArduinoJson::JsonObjectConst>();
//         if (profile.isNull()) {
//             SS_LOGE("profile %zu is not an object", pi);
//             continue;
//         }

//         const char *name = profile["name"] | "";
//         ProfileName pname;
//         if (name[0] != '\0') {
//             pname = name; // ETL truncates as needed
//         } else {
//             char buf[20];
//             std::snprintf(buf, sizeof(buf), "Profile %u", (unsigned) pi);
//             pname = buf;
//         }
//         out.catalog.profileNames[wroteProfiles++] = pname;

//         auto events = profile["events"].as<ArduinoJson::JsonObjectConst>();
//         if (events.isNull()) {
//             SS_LOGE("profile '%s' has no 'events' object", pname.c_str());
//             continue;
//         }

//         // Iterate events under this profile
//         for (auto kvp : events) {
//             const char *evKey = kvp.key().c_str();
//             auto        evObj = kvp.value().as<ArduinoJson::JsonObjectConst>();

//             EventId ev = eventFromCStr(evKey);
//             if (ev == EventId::Unknown) {
//                 SS_LOGE("profile '%s' has unknown event '%s'", pname.c_str(), evKey);
//                 continue;
//             }

//             // AUDIO: delegate to your audio parser (no struct-field assumptions)
//             if (auto audioObj = evObj["audio"].as<ArduinoJson::JsonObjectConst>();
//                 !audioObj.isNull()) {
//                 audio::AudioPlaySpec spec{};
//                 if (!audio::parseAudioSpec(audioObj, spec)) {
//                     SS_LOGW("audio parse failed for '%s'/'%s'", pname.c_str(), evKey);
//                 } else {
//                     if (out.catalog.audioTable.full()) {
//                         SS_LOGE("audio flat_map full (SS_MAX_EVENTS_TOTAL=%zu)",
//                             (size_t) SS_MAX_EVENTS_TOTAL);
//                         return false;
//                     }
//                     ProfileCatalog::EntryKey k{pname, ev};
//                     auto                     it = out.catalog.audioTable.find(k);
//                     if (it == out.catalog.audioTable.end()) {
//                         if (!out.catalog.audioTable.insert({k, spec}).second) {
//                             SS_LOGE("audio insert failed at profile '%s' event '%s'",
//                                 pname.c_str(),
//                                 evKey);
//                             return false;
//                         }
//                     } else {
//                         it->second = spec;
//                     }
//                 }
//             }

//             // LED: { pattern, periodMs, cnt } (accept legacy "nt" for cnt)
//             if (auto ledObj = evObj["led"].as<ArduinoJson::JsonObjectConst>(); !ledObj.isNull()) {
//                 led::LedPlaySpec lspec{};
//                 lspec.pattern  = led::ledPatternFromCStr((const char *) (ledObj["pattern"] | ""));
//                 uint32_t pms   = ledObj["periodMs"] | 0u;
//                 uint32_t cnt   = ledObj["cnt"] | (ledObj["nt"] | 0u);
//                 lspec.periodMs = (uint16_t) (pms <= 0xFFFFu ? pms : 0xFFFFu);
//                 lspec.cnt      = (uint16_t) (cnt <= 0xFFFFu ? cnt : 0xFFFFu);

//                 if (out.catalog.ledTable.full()) {
//                     SS_LOGE("led flat_map full (SS_MAX_EVENTS_TOTAL=%zu)",
//                         (size_t) SS_MAX_EVENTS_TOTAL);
//                     return false;
//                 }
//                 ProfileCatalog::EntryKey k{pname, ev};
//                 auto                     it = out.catalog.ledTable.find(k);
//                 if (it == out.catalog.ledTable.end()) {
//                     if (!out.catalog.ledTable.insert({k, lspec}).second) {
//                         SS_LOGE(
//                             "led insert failed at profile '%s' event '%s'", pname.c_str(), evKey);
//                         return false;
//                     }
//                 } else {
//                     it->second = lspec;
//                 }
//             }
//         }
//     }

//     if (wroteProfiles == 0) {
//         out.catalog.profileNames[0] = "profile";
//         SS_LOGW("no profile names provided; defaulted to ['profile']");
//     }

//     return true;
// }

// } // namespace esphome::smart_signage
