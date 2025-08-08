#pragma once
#include <HardwareSerial.h>
#include <MyLD2410.h>
#include "iradar_hal.h"

#include "log.h"

namespace esphome::smart_signage::radar::hal {

class LD2410RadarHal : public IRadarHal {
  public:
    LD2410RadarHal(HardwareSerial &serial, int8_t rxPin, int8_t txPin)
        : serial_(serial), rxPin_(rxPin), txPin_(txPin), radar_(serial) {}

    bool init() override {
        serial_.begin(256000, SERIAL_8N1, rxPin_, txPin_);
        LOGD("Serial started on RX=%d, TX=%d", rxPin_, txPin_);

        auto resp = radar_.begin();
        if (resp != MyLD2410::Response::ACK) {
            LOGE("radar.begin() returned %d", static_cast<int>(resp));
            return false;
        }
        LOGD("Radar init OK");
        return true;
    }

    bool hasNewData() override { return radar_.check() == MyLD2410::Response::DATA; }

    bool presenceDetected() override { return radar_.presenceDetected(); }

    uint16_t getDistance() override {
        uint16_t movingDis     = radar_.movingTargetDistance();
        uint16_t stationaryDis = radar_.stationaryTargetDistance();

        // For Serial Plotter: add platformio extention "serial-plotter by Mario Zechner", and
        // uncomment the next line
        // Serial.printf(">Moving_Target_Distance:%u,Stationary_Target_Distance:%u\r\n", movingDis,
        // stationaryDis);

        return movingDis;
    }

  private:
    HardwareSerial &serial_;
    int8_t          rxPin_, txPin_;
    MyLD2410        radar_;

    static constexpr char TAG[] = "RadarHal";
};
} // namespace esphome::smart_signage::radar::hal
