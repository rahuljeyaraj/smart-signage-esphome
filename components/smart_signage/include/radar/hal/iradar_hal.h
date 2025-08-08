#pragma once

#include <cstdint>

namespace esphome::smart_signage::radar::hal {
class IRadarHal {
  public:
    virtual ~IRadarHal()                = default;
    virtual bool     init()             = 0;
    virtual bool     hasNewData()       = 0;
    virtual bool     presenceDetected() = 0;
    virtual uint16_t getDistance()      = 0;
};
} // namespace esphome::smart_signage::radar::hal