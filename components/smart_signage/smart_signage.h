#pragma once
#include "esphome.h"
#include <esphome/components/number/number.h>
#include <esphome/components/select/select.h>

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
#include "storage/nvs_storage.h"
#include "profile_settings.h"
#include "config/app_config.h"
#include "config/app_config_parser.h"
#include "user_intf.h"
#include "common.h"

namespace esphome::smart_signage {

class SmartSignage : public Component {
  public:
    SmartSignage(const UiHandles &ui, const char *configJson);

    void setup() override;
    void loop() override;
    void dump_config() override;

  private:
    const char *catalogJson_;

    /*────── Message queues ────────*/
    ctrl::Q  ctrlQ_;
    radar::Q radarQ_;
    imu::Q   imuQ_;
    led::Q   ledQ_;
    audio::Q audioQ_;

    /*────── User UI ────────*/
    UserIntfT ui_;

    /*──────  Radar  ────*/
    HardwareSerial             radarSerial_;
    radar::hal::LD2410RadarHal radarHal_;
    timer::EspTimer            radarTimer_;
    radar::RadarAO             radarAo_;

    /*──────  Imu ────*/
    MPU6500             imu_;
    imu::hal::I2cImuHal imuHal_;
    timer::EspTimer     imuTimer_;
    imu::ImuAO          imuAo_;

    /*──────  Led  ────*/
    led::hal::EspLedHal ledHal_;
    timer::EspTimer     ledTimer_;
    led::LedAO          ledAo_;

    /*──────  Audio  ────*/
    audio::hal::I2SAudioHal audioHal_;
    timer::EspTimer         audioTimer_;
    audio::AO               audioAo_;

    /*──────  Ctrl / Settings ───────*/
    storage::NvsStorage storage_;
    ProfileCatalog      profileCatalog_;
    ProfileSettings     profileSettings_;
    timer::EspTimer     ctrlTimer_;
    ctrl::CtrlAO        ctrlAo_;

    static constexpr char kNVSNamespace[] = "SmartSignage";
    static constexpr char TAG[]           = "SmartSignage";
};

} // namespace esphome::smart_signage
