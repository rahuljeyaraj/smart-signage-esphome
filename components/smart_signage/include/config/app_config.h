// #pragma once

// #include <etl/array.h>
// #include <etl/flat_map.h>
// #include <etl/string.h>

// #include "log.h"               // SS_LOGx (ASCII-only)
// #include "common.h"            // ProfileName (etl::string<N>)
// #include "audio/audio_event.h" // audio::AudioPlaySpec
// #include "led/led_play_spec.h"

// namespace esphome::smart_signage {

// // Event id mapping (same names you used in JSON)
// enum class EventId : uint8_t {
//     SetupDone,
//     Error,
//     Start,
//     Stop,
//     UiUpdate,
//     Detected,
//     Detected0Cm,
//     Fell,
//     Rose,
//     SessionEnd,
//     Unknown
// };

// inline const char *eventToCStr(EventId e) {
//     switch (e) {
//     case EventId::SetupDone: return "SetupDone";
//     case EventId::Error: return "Error";
//     case EventId::Start: return "Start";
//     case EventId::Stop: return "Stop";
//     case EventId::UiUpdate: return "UiUpdate";
//     case EventId::Detected: return "Detected";
//     case EventId::Detected0Cm: return "Detected0Cm";
//     case EventId::Fell: return "Fell";
//     case EventId::Rose: return "Rose";
//     case EventId::SessionEnd: return "SessionEnd";
//     default: return "Unknown";
//     }
// }
// inline bool    streq_(const char *a, const char *b) { return std::strcmp(a, b) == 0; }
// inline EventId eventFromCStr(const char *s) {
//     if (!s || s[0] == '\0') return EventId::Unknown;
//     if (streq_(s, "SetupDone")) return EventId::SetupDone;
//     if (streq_(s, "Error")) return EventId::Error;
//     if (streq_(s, "Start")) return EventId::Start;
//     if (streq_(s, "Stop")) return EventId::Stop;
//     if (streq_(s, "UiUpdate")) return EventId::UiUpdate;
//     if (streq_(s, "Detected")) return EventId::Detected;
//     if (streq_(s, "Detected0Cm")) return EventId::Detected0Cm;
//     if (streq_(s, "Fell")) return EventId::Fell;
//     if (streq_(s, "Rose")) return EventId::Rose;
//     if (streq_(s, "SessionEnd")) return EventId::SessionEnd;
//     return EventId::Unknown;
// }

// // Catalog of profile-related static config
// struct ProfileCatalog {
//     // composite key used internally and by the parser
//     struct EntryKey {
//         ProfileName name{};
//         EventId     event{EventId::Unknown};
//         bool        operator<(const EntryKey &o) const {
//             if (name < o.name) return true;
//             if (o.name < name) return false;
//             return (uint8_t) event < (uint8_t) o.event;
//         }
//     };

//     etl::array<ProfileName, SS_MAX_PROFILES>                           profileNames{};
//     etl::flat_map<EntryKey, audio::AudioPlaySpec, SS_MAX_EVENTS_TOTAL> audioTable{};
//     etl::flat_map<EntryKey, led::LedPlaySpec, SS_MAX_EVENTS_TOTAL>     ledTable{};

//     // Simple lookups by (profileName, event name)
//     bool getAudioSpec(
//         const ProfileName &profile, const char *eventName, audio::AudioPlaySpec &out) const {
//         EventId ev = eventFromCStr(eventName);
//         if (ev == EventId::Unknown) return false;
//         EntryKey k{profile, ev};
//         auto     it = audioTable.find(k);
//         if (it == audioTable.end()) return false;
//         out = it->second;
//         return true;
//     }
//     bool getLedSpec(
//         const ProfileName &profile, const char *eventName, led::LedPlaySpec &out) const {
//         EventId ev = eventFromCStr(eventName);
//         if (ev == EventId::Unknown) return false;
//         EntryKey k{profile, ev};
//         auto     it = ledTable.find(k);
//         if (it == ledTable.end()) return false;
//         out = it->second;
//         return true;
//     }
// };

// struct AppConfig {
//     ProfileCatalog catalog;
// };

// } // namespace esphome::smart_signage
