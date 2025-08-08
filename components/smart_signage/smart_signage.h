#pragma once
#include "esphome.h"
#include <esphome/components/number/number.h>
#include <esphome/components/select/select.h>

#include "timer/esp_timer.h"
#include "radar/hal/ld2410_radar_hal.h"
#include "imu/hal/i2c_imu_hal.h"

#include "ctrl/ctrl_ao.h"
#include "radar/radar_ao.h"
#include "imu/imu_ao.h"
#include "led/led_ao.h"
#include "audio/audio_ao.h"
#include "fsm_logger.h"

#include "config/nvs_config_manager.h"
#include "user_intf.h"

namespace esphome::smart_signage {

class SmartSignage : public Component {
  public:
    SmartSignage(const UiHandles &ui, const char *configJson);

    void setup() override;
    void loop() override;
    void dump_config() override;

  private:
    /*────── High-level helpers ────*/
    NVSConfigManager nvsConfigManager_;
    UserIntf         userIntf_;

    /*────── Message queues ────────*/
    ctrl::Q  ctrlQ_;
    radar::Q radarQ_;
    imu::Q   imuQ_;
    led::Q   ledQ_;
    audio::Q audioQ_;

    /*──────  Radar dependencies ────*/
    SimpleKalmanFilter         filter_;
    HardwareSerial             radarSerial_;
    radar::hal::LD2410RadarHal radarHal_;

    /*──────  Imu dependencies ────*/
    MPU6500             imu_;
    imu::hal::I2cImuHal imuHal_;
    timer::EspTimer     imuPollTimer_;

    /*────── Finite-state machines ─*/
    ctrl::FSM  ctrlFsm_;
    radar::FSM radarFsm_;
    imu::FSM   imuFsm_;
    led::FSM   ledFsm_;
    audio::FSM audioFsm_;

    /*────── Loggers ───────────────*/
    FsmLogger ctrlFsmLogger_;
    FsmLogger radarFsmLogger_;
    FsmLogger imuFsmLogger_;
    FsmLogger ledFsmLogger_;
    FsmLogger audioFsmLogger_;

    /*────── Active objects/tasks ──*/
    ctrl::AO  ctrlAo_;
    radar::AO radarAo_;
    imu::AO   imuAo_;
    led::AO   ledAo_;
    audio::AO audioAo_;

    static constexpr char kNVSNamespace[] = "SmartSignage";
    static constexpr char TAG[]           = "SmartSignage";

  private:
    static void imuPollCb(void *arg);
};

} // namespace esphome::smart_signage
