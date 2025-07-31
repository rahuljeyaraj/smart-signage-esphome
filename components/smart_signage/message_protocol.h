#pragma once
#include <etl/message.h>
#include <etl/string.h>

namespace esphome::smart_signage
{

    // ─── Message IDs ────────────────────────────────────────────────────────────
    enum : uint16_t
    {
        MSG_PRESENCE = 1,
        MSG_FALL,
        MSG_LED_ON,
        MSG_PLAY_AUDIO
    };

    // ─── Sensor-side events (Radar / IMU) ───────────────────────────────────────
    struct PresenceMsg : etl::message<MSG_PRESENCE>
    {
    };
    struct FallMsg : etl::message<MSG_FALL>
    {
    };

    // ─── Actuator commands (Controller → LED / Speaker) ─────────────────────────
    struct LedOnMsg : etl::message<MSG_LED_ON>
    {
        uint8_t brightness{};
        LedOnMsg(uint8_t bri = 80) : brightness(bri) {}
    };

    struct PlayAudioMsg : etl::message<MSG_PLAY_AUDIO>
    {
        etl::string<64> path;
        PlayAudioMsg() = default;
        explicit PlayAudioMsg(const char *p) { path.assign(p); }
    };

} // namespace esphome::smart_signage
