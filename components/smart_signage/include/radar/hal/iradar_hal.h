#pragma once

#include <cstdint>

class IRadarHal {
  public:
    virtual ~IRadarHal()                = default;
    virtual bool     init()             = 0;
    virtual bool     hasNewData()       = 0;
    virtual bool     presenceDetected() = 0;
    virtual uint16_t getDistance()      = 0;


};