#include "smart_signage.h"
#include "log.h"
#include "board.h"
#include <Arduino.h>
#include <Wire.h>

namespace esphome::smart_signage {

// Controller controller(logger);
// SmartSignageApp app(controller, logger);

// // Radar setup
// HardwareSerial radarSerial(1);
// LD2410RadarHal radarHal(radarSerial, RADAR_RX_PIN, RADAR_TX_PIN, logger);
// RadarDriver frontRadar(Id::FrontRadar, controller, radarHal, logger);
// IMU setup
// ESP32-S3: supports two I2C peripherals (0/1 = Wire/Wire1).
// ESP32-C3: only one I2C peripheral (0 = Wire).
// MPU6500   imu(Wire);
// I2cImuHal imuHal(imu, Wire, MPU6500_DEFAULT_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN);
// ImuDriver imuDrver(Id::FallDetector, controller, imuHal);
// // LED setup
// EspLedHal ledHal(LED0_PIN);
// LedDriver frontLed(Id::FrontLed, controller, ledHal, logger);

// // Speaker setup
// audio_tools::AudioSourceLittleFS source("/", ".mp3");
// audio_tools::MP3DecoderHelix decoder;
// I2SAudioHal audioHal(source, decoder, I2S_BCLK_PIN, I2S_LRCK_PIN, I2S_DATA_PIN);
// AudioDriver audioDriver(Id::FrontSpeaker, controller, audioHal, logger);

SmartSignage::SmartSignage(const UiHandles &ui, const char *configJson)
    // clang-format off
    /*── Helpers ──*/
    : nvsConfigManager_{kNVSNamespace}
    , userIntf_{nvsConfigManager_, ui, configJson}

    /*──────  Radar dependencies ────*/
    , radarSerial_{1}
    , radarHal_{radarSerial_, RADAR_RX_PIN, RADAR_TX_PIN}
    , radarTimer_()

    /*──────  Imu dependencies ────*/
    , imu_(Wire) //ESP32-S3: (I2C 0/1 = Wire/Wire1), ESP32-C3: (I2C 0 = Wire).
    , imuHal_(imu_, Wire, MPU6500_DEFAULT_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN)
    , imuTimer_()

    , ledHal_(LED0_PIN)
    , ledTimer_()

    /*── Queues ──*/
    , ctrlQ_{}
    , radarQ_{}
    , imuQ_{}
    , ledQ_{}
    , audioQ_{}
    
    /*── FSMs ──*/
    , ctrlFsm_{radarQ_, imuQ_, ledQ_, audioQ_}
    , ledFsm_{ctrlQ_}
    , audioFsm_{ctrlQ_}

    /*── Loggers ──*/
    , ctrlFsmLogger_{"ctrlFsmLogger"}
    , audioFsmLogger_{"audioFsmLogger"}

    /*── Active objects (tasks) ──*/
    , radarAo_{radarQ_, ctrlQ_, radarHal_, radarTimer_, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1}
    , imuAo_    {imuQ_,    ctrlQ_, imuHal_, imuTimer_   "imuTask",      8192, tskIDLE_PRIORITY + 2, 1}
    , ledAo_    {ledQ_,  ctrlQ_, ledHal_,  ledTimer_   "ledTask",      8192, tskIDLE_PRIORITY + 2, 1}
    , ctrlAo_   {ctrlQ_,    ctrlFsm_,   ctrlFsmLogger_,     "ctrlTask",     8192, tskIDLE_PRIORITY + 2, 1}
   

    , audioAo_  {audioQ_,   audioFsm_,  audioFsmLogger_,    "audioTask",    8192, tskIDLE_PRIORITY + 2, 1}
// clang-format on
{}

void SmartSignage::setup() {
    LOGI("SmartSignage setup");
    userIntf_.setup();
    ctrlQ_.post(ctrl::CmdSetup{});
    vTaskDelay(pdMS_TO_TICKS(1000));
    ctrlQ_.post(ctrl::CmdStart{});
}
void SmartSignage::loop() {}
void SmartSignage::dump_config() { LOGI("SmartSignage ready"); }

} // namespace esphome::smart_signage
