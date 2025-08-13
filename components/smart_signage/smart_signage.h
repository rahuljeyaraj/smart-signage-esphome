#pragma once
#include "esphome.h"
#include <esphome/components/number/number.h>
#include <esphome/components/select/select.h>

#include <etl/vector.h>

#include "timer/esp_timer.h"
#include "radar/hal/ld2410_radar_hal.h"
#include "imu/hal/i2c_imu_hal.h"
#include "led/hal/esp_led_hal.h"
#include "audio/hal/i2s_audio_hal.h"

#include "ctrl/ctrl_ao.h"
#include "radar/radar_ao.h"
#include "imu/imu_ao.h"
#include "led/led_ao.h"
#include "audio/audio_ao.h"

#include "fsm_logger.h"
#include "config/nvs_config_manager.h"

// NEW: Profiles + UI bridge live in esphome::smart_signage
#include "profile_config.h" // ProfilesConfig<...>, ProfileName
#include "user_intf.h"      // UserIntf<MAX_PROFILES>, UiHandles

#ifndef SS_MAX_PROFILES
#define SS_MAX_PROFILES 8
#endif

#ifndef SS_MAX_EVENTS_TOTAL
#define SS_MAX_EVENTS_TOTAL 32
#endif

namespace esphome::smart_signage {


// Handy aliases matching your new classes
using ProfilesConfigT = ProfilesConfig<SS_MAX_PROFILES, SS_MAX_EVENTS_TOTAL>;
using UserIntfT       = UserIntf<SS_MAX_PROFILES>;

class SmartSignage : public Component {
  public:
    SmartSignage(const UiHandles &ui, const char *configJson);

    void setup() override;
    void loop() override;
    void dump_config() override;

  private:
    // ── Inputs / config
    const char *configJson_{nullptr};

    // ── Message queues
    ctrl::Q  ctrlQ_{};
    radar::Q radarQ_{};
    imu::Q   imuQ_{};
    led::Q   ledQ_{};
    audio::Q audioQ_{};

    // ── Profiles + UI bridge
    ProfilesConfigT profilesCfg_{};
    UserIntfT       ui_;


    static constexpr char kNVSNamespace[] = "SmartSignage";
    static constexpr char TAG[]           = "SmartSignage";
};

} // namespace esphome::smart_signage

// #pragma once
// #include "esphome.h"
// #include <esphome/components/number/number.h>
// #include <esphome/components/select/select.h>

// #include "timer/esp_timer.h"
// #include "radar/hal/ld2410_radar_hal.h"
// #include "imu/hal/i2c_imu_hal.h"
// #include "led/hal/esp_led_hal.h"
// #include "audio/hal/i2s_audio_hal.h"

// #include "ctrl/ctrl_ao.h"
// #include "radar/radar_ao.h"
// #include "imu/imu_ao.h"
// #include "led/led_ao.h"
// #include "audio/audio_ao.h"

// #include "fsm_logger.h"
// #include "config/nvs_config_manager.h"
// #include "profile_config.h"
// #include "user_intf.h"

// #ifndef SS_MAX_PROFILES
// #define SS_MAX_PROFILES 8
// #endif

// #ifndef SS_MAX_EVENTS_TOTAL
// #define SS_MAX_EVENTS_TOTAL 32
// #endif

// namespace esphome::smart_signage {

// class SmartSignage : public Component {
//   public:
//     SmartSignage(const UiHandles &ui, const char *configJson);

//     void setup() override;
//     void loop() override;
//     void dump_config() override;

//   private:
//     const char *configJson_;

//     /*────── Message queues ────────*/
//     ctrl::Q  ctrlQ_;
//     radar::Q radarQ_;
//     imu::Q   imuQ_;
//     led::Q   ledQ_;
//     audio::Q audioQ_;

//     ProfilesConfig<SS_MAX_PROFILES, SS_MAX_EVENTS_TOTAL> profilesCfg_;
//     UserIntf<SS_MAX_PROFILES>                            ui_;

//     /*────── High-level helpers ────*/
//     // profiles::ProfilesDB profileDb_;
//     // NVSConfigManager     nvsConfigManager_;
//     // UserIntf             userIntf_;

//     // /*──────  Radar  ────*/
//     // HardwareSerial             radarSerial_;
//     // radar::hal::LD2410RadarHal radarHal_;
//     // timer::EspTimer            radarTimer_;
//     // radar::RadarAO             radarAo_;

//     // /*──────  Imu ────*/
//     // MPU6500             imu_;
//     // imu::hal::I2cImuHal imuHal_;
//     // timer::EspTimer     imuTimer_;
//     // imu::ImuAO          imuAo_;

//     // /*──────  Led  ────*/
//     // led::hal::EspLedHal ledHal_;
//     // timer::EspTimer     ledTimer_;
//     // led::LedAO          ledAo_;

//     // /*──────  Audio  ────*/
//     // audio::hal::I2SAudioHal audioHal_;
//     // timer::EspTimer         audioTimer_;
//     // audio::AO               audioAo_;

//     // /*──────  Ctrl ───────*/
//     // ctrl::AO ctrlAo_;

//     static constexpr char kNVSNamespace[] = "SmartSignage";
//     static constexpr char TAG[]           = "SmartSignage";
// };

// } // namespace esphome::smart_signage
