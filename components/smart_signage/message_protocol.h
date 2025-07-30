#pragma once

#include <cstdint>
#include <etl/variant.h>

namespace esphome
{
    namespace smart_signage
    {

        // ─── Event Protocol (sensor → controller) ─────────────────
        enum class EventId : uint8_t
        {
            PresenceDetected,
            FallDetected,
        };

        using EventData = etl::variant<etl::monostate, bool, uint16_t>;

        struct Event
        {
            EventId id;
            uint32_t timestamp; // xTaskGetTickCount()
            EventData data;     // unused in this stub
        };

        // ─── Command Protocol (controller → actuator) ───────────────
        enum class CmdId : uint8_t
        {
            LedOn,
            PlayAudio,
        };

        using CmdParam = etl::variant<etl::monostate, uint8_t, uint16_t>;

        struct Command
        {
            CmdId id;
            CmdParam param;
        };

    } // namespace smart_signage
} // namespace esphome
