// #pragma once
// #include <ArduinoJson.h> // v7
// #include <cstdint>
// #include <cstring>
// #include "log.h"
// #include "audio/audio_const.h"
// #include "audio/audio_event.h"

// namespace esphome::smart_signage::profiles {

// static constexpr char TAG[] = "ProfilesDB";

// enum class EventId : uint8_t {
//     SetupDone,
//     SetupError,
//     Start,
//     Detected,
//     NotDetected,
//     Fallen,
//     Rose,
//     RunEnd,
//     LowBattery,
//     COUNT
// };
// inline uint8_t idx(EventId e) { return static_cast<uint8_t>(e); }

// enum class LedKind : uint8_t { None, Off, On, Blink, Breathe, BreatheByDistance };

// struct LedBlink {
//     uint16_t on_ms{120}, off_ms{120};
//     uint16_t count{3};
// };
// struct LedBreatheTri {
//     uint16_t rise_ms{200}, fall_ms{200};
//     uint16_t count{1};
// };
// struct LedBreatheByDist {
//     bool  invert{true};
//     float fmin{1.0f}, fmax{5.0f};
// };

// struct LedBinding {
//     bool    has{false};
//     LedKind kind{LedKind::None};
//     union {
//         LedBlink         blink;
//         LedBreatheTri    tri;
//         LedBreatheByDist bydist;
//     } u{};
// };

// struct AudioBinding {
//     bool                 has{false};
//     audio::AudioPlaySpec spec{};
// };
// struct EventBinding {
//     AudioBinding audio;
//     LedBinding   led;
// };

// struct ProfileSummary {
//     char id[24]{};
//     char name[32]{};
// };

// struct ProfileMap {
//     char         id[24]{};
//     char         name[32]{};
//     EventBinding events[idx(EventId::COUNT)]{};
//     uint32_t     gen{0};
// };

// class ProfilesDB {
//   public:
//     explicit ProfilesDB(const char *json_text) : json_(json_text) {}
//     uint8_t profile_count() const {
//         if (!json_) return 0;
//         JsonDocument doc;
//         if (deserializeJson(doc, json_) != DeserializationError::Ok) return 0;
//         auto arr = doc["profiles"];
//         return arr.is<JsonArray>() ? static_cast<uint8_t>(arr.size()) : 0;
//     }
//     bool get_summary(uint8_t i, ProfileSummary &out) const {
//         JsonDocument doc;
//         if (deserializeJson(doc, json_) != DeserializationError::Ok) return false;
//         auto arr = doc["profiles"];
//         if (!arr.is<JsonArray>() || i >= arr.size()) return false;
//         auto p = arr[i];
//         cpy_(p["id"], out.id, sizeof(out.id));
//         cpy_(p["name"], out.name, sizeof(out.name));
//         return true;
//     }
//     bool build_profile_map(uint8_t i, ProfileMap &out) const {
//         JsonDocument doc;
//         if (deserializeJson(doc, json_) != DeserializationError::Ok) {
//             SS_LOGE("json parse");
//             return false;
//         }
//         auto arr = doc["profiles"];
//         if (!arr.is<JsonArray>() || i >= arr.size()) return false;
//         auto p = arr[i];
//         cpy_(p["id"], out.id, sizeof(out.id));
//         cpy_(p["name"], out.name, sizeof(out.name));
//         auto ev = p["events"];
//         if (!ev.is<JsonObject>()) return true;

//         map_one_(ev["setup_done"], EventId::SetupDone, out);
//         map_one_(ev["setup_error"], EventId::SetupError, out);
//         map_one_(ev["start"], EventId::Start, out);
//         map_one_(ev["detected"], EventId::Detected, out);
//         map_one_(ev["not_detected"], EventId::NotDetected, out);
//         map_one_(ev["fallen"], EventId::Fallen, out);
//         map_one_(ev["rose"], EventId::Rose, out);
//         map_one_(ev["run_end"], EventId::RunEnd, out);
//         map_one_(ev["low_battery"], EventId::LowBattery, out);
//         return true;
//     }

//   private:
//     static void cpy_(JsonVariantConst v, char *dst, size_t cap) {
//         const char *s = v.is<const char *>() ? v.as<const char *>() : "";
//         size_t      n = (cap > 0) ? strnlen(s, cap - 1) : 0;
//         if (cap) {
//             memcpy(dst, s, n);
//             dst[n] = '\0';
//         }
//     }
//     static uint16_t to_ms_(JsonVariantConst gap_ms, JsonVariantConst repeat_s) {
//         if (gap_ms.is<uint32_t>()) return static_cast<uint16_t>(gap_ms.as<uint32_t>());
//         if (repeat_s.is<uint32_t>()) return static_cast<uint16_t>(repeat_s.as<uint32_t>() *
//         1000u); if (repeat_s.is<float>()) return static_cast<uint16_t>(repeat_s.as<float>() *
//         1000.0f); return 0;
//     }
//     void map_audio_(JsonVariantConst node, AudioBinding &a) const {
//         if (!node) return;
//         if (node.is<const char *>()) {
//             a.has    = true;
//             a.spec.n = 1;
//             cpy_(node, a.spec.items[0].source, audio::kSourceStrLen);
//             a.spec.items[0].gapMs = 0;
//             a.spec.playCnt      = 1;
//             return;
//         }
//         if (node.is<JsonObject>()) {
//             auto s = node["audio"];
//             if (!s.is<const char *>()) return;
//             a.has    = true;
//             a.spec.n = 1;
//             cpy_(s, a.spec.items[0].source, audio::kSourceStrLen);
//             a.spec.items[0].gapMs = to_ms_(node["gap_ms"], node["repeat_s"]);
//             a.spec.playCnt      = node["loops"].is<uint32_t>()
//                                         ? static_cast<uint16_t>(node["loops"].as<uint32_t>())
//                                         : 1;
//             return;
//         }
//         if (node.is<JsonArray>()) {
//             a.has     = true;
//             uint8_t i = 0;
//             for (JsonObjectConst el : node.as<JsonArrayConst>()) {
//                 if (i >= audio::kMaxPlaylist) break;
//                 cpy_(el["audio"], a.spec.items[i].source, audio::kSourceStrLen);
//                 a.spec.items[i].gapMs = to_ms_(el["gap_ms"], el["repeat_s"]);
//                 ++i;
//             }
//             a.spec.n         = i;
//             a.spec.playCnt = 1;
//         }
//     }
//     void map_led_(JsonVariantConst node, LedBinding &lb) const {
//         if (!node || !node.is<JsonObject>()) return;
//         const char *eff =
//             node["effect"].is<const char *>() ? node["effect"].as<const char *>() : "on";
//         if (strcmp(eff, "off") == 0) {
//             lb.has  = true;
//             lb.kind = LedKind::Off;
//             return;
//         } else if (strcmp(eff, "on") == 0) {
//             lb.has  = true;
//             lb.kind = LedKind::On;
//             return;
//         } else if (strcmp(eff, "blink") == 0) {
//             lb.has            = true;
//             lb.kind           = LedKind::Blink;
//             lb.u.blink.on_ms  = node["on_ms"].is<uint32_t>() ? node["on_ms"].as<uint32_t>() :
//             120; lb.u.blink.off_ms = node["off_ms"].is<uint32_t>() ?
//             node["off_ms"].as<uint32_t>() : 120; lb.u.blink.count  = node["count"].is<uint32_t>()
//             ? node["count"].as<uint32_t>() : 3; return;
//         } else if (strcmp(eff, "breathe") == 0) {
//             lb.has  = true;
//             lb.kind = LedKind::Breathe;
//             lb.u.tri.rise_ms =
//                 node["rise_ms"].is<uint32_t>() ? node["rise_ms"].as<uint32_t>() : 200;
//             lb.u.tri.fall_ms =
//                 node["fall_ms"].is<uint32_t>() ? node["fall_ms"].as<uint32_t>() : 200;
//             lb.u.tri.count = node["count"].is<uint32_t>() ? node["count"].as<uint32_t>() : 1;
//             return;
//         } else if (strcmp(eff, "breathe_by_distance") == 0) {
//             lb.has             = true;
//             lb.kind            = LedKind::BreatheByDistance;
//             lb.u.bydist.invert = node["invert"].is<bool>() ? node["invert"].as<bool>() : true;
//             lb.u.bydist.fmin =
//                 node["freq_hz"]["min"].is<float>() ? node["freq_hz"]["min"].as<float>() : 1.0f;
//             lb.u.bydist.fmax =
//                 node["freq_hz"]["max"].is<float>() ? node["freq_hz"]["max"].as<float>() : 5.0f;
//             return;
//         }
//         lb.has  = true;
//         lb.kind = LedKind::On;
//     }
//     void map_one_(JsonVariantConst node, EventId id, ProfileMap &out) const {
//         if (!node) return;
//         auto &b = out.events[idx(id)];
//         if (node.is<JsonObject>()) {
//             map_audio_(node["audio"], b.audio);
//             map_led_(node["led"], b.led);
//             return;
//         }
//         if (node.is<JsonArray>()) { map_audio_(node, b.audio); }
//     }

//     const char *json_{nullptr};
// };

// } // namespace esphome::smart_signage::profiles
