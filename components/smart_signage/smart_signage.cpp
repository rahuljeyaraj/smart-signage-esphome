// keep your existing includes…
#include "smart_signage.h"
#include "log.h"
#include "board.h"

// NEW: bare Arduino + IDF I2S std API (no AudioTools)
#include <Arduino.h>
#include "driver/i2s_std.h"
#include <math.h>

#include "smart_signage.h"
#include "log.h"
#include "board.h"
#include <LittleFS.h>
#include <AudioToolsConfig.h>
#include <AudioTools/CoreAudio/AudioI2S/I2SStream.h>
#include <AudioTools/CoreAudio/VolumeStream.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/AudioCodecs/AudioEncoded.h>
#include <AudioTools/CoreAudio/StreamCopy.h>

#include "esp_partition.h"
#include "esp_log.h"
#include "helper.h"

namespace esphome::smart_signage {
namespace {
    audio_tools::I2SStream          g_i2s;
    audio_tools::I2SConfig          g_cfg;
    audio_tools::MP3DecoderHelix    g_mp3;
    audio_tools::VolumeStream       g_vol{g_i2s};
    audio_tools::EncodedAudioStream g_dec{&g_vol, &g_mp3};
    audio_tools::StreamCopy         g_copy;
    File                            g_file;
    bool                            g_active = false;
} // namespace

SmartSignage::SmartSignage(const UiHandles &ui, const char *configJson)
    : nvsConfigManager_{kNVSNamespace}, userIntf_{nvsConfigManager_, ui, configJson}, //
                                                                                      //
      ctrlQ_{}, radarQ_{}, imuQ_{}, ledQ_{}, audioQ_{},                               //
      //

      radarSerial_{1}, radarHal_{radarSerial_, RADAR_RX_PIN, RADAR_TX_PIN}, radarTimer_{},
      radarAo_{radarQ_, ctrlQ_, radarHal_, radarTimer_, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1},
      //
      imu_{Wire}, imuTimer_{},
      imuHal_{imu_, Wire, MPU6500_DEFAULT_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN},
      imuAo_{imuQ_, ctrlQ_, imuHal_, imuTimer_, "imuTask", 8192, tskIDLE_PRIORITY + 2, 1}, //

      //
      ledHal_{LED0_PIN}, ledTimer_{},
      ledAo_{ledQ_, ctrlQ_, ledHal_, ledTimer_, "ledTask", 8192, tskIDLE_PRIORITY + 2, 1}, //

      //
      ctrlFsm_{radarQ_, imuQ_, ledQ_, audioQ_}, audioFsm_{ctrlQ_}, //
      ctrlFsmLogger_{"ctrlFsmLogger"}, audioFsmLogger_{"audioFsmLogger"},
      ctrlAo_{ctrlQ_, ctrlFsm_, ctrlFsmLogger_, "ctrlTask", 8192, tskIDLE_PRIORITY + 2, 1},

      audioAo_{audioQ_, audioFsm_, audioFsmLogger_, "audioTask", 8192, tskIDLE_PRIORITY + 2, 1} {}

void SmartSignage::setup() {
    SS_LOGI("SmartSignage setup");
    
    if (!LittleFS.begin(true)) {
        SS_LOGE("LittleFS mount failed");
        return;
    }

    print_partition_table();
    list_all_files();

    // 2) I2S out
    g_cfg          = g_i2s.defaultConfig(audio_tools::TX_MODE);
    g_cfg.pin_bck  = I2S_BCLK_PIN;
    g_cfg.pin_ws   = I2S_LRCK_PIN;
    g_cfg.pin_data = I2S_DATA_PIN;
    if (!g_i2s.begin(g_cfg)) {
        SS_LOGE("I2S begin failed");
        return;
    }

    // 3) decoder -> I2S (through volume)
    g_dec.addNotifyAudioChange(g_i2s);
    g_vol.begin();

    // 4) open and start
    g_file = LittleFS.open("/test/welcome_2.mp3", "r");
    if (!g_file) {
        SS_LOGE("open /test/welcome_2.mp3 failed");
        return;
    }
    g_dec.begin();
    g_copy.begin(g_dec, g_file);
    g_active = true;

    // // your existing init
    // userIntf_.setup();
    // ctrlQ_.post(ctrl::CmdSetup{});
}

void SmartSignage::loop() {
    if (g_active) {
        if (g_copy.copy() == 0 && !g_file.available()) {
            g_copy.end();
            g_dec.end();
            g_file.close();
            g_active = false;
            SS_LOGI("Playback done");
        }
    }
}
void SmartSignage::dump_config() { SS_LOGI("SmartSignage ready"); }
} // namespace esphome::smart_signage

// // #include "smart_signage.h"
// // #include "log.h"
// // #include "board.h"
// // #include <Arduino.h>
// // #include <Wire.h>
// // #

// // namespace esphome::smart_signage {

// // // Controller controller(logger);
// // // SmartSignageApp app(controller, logger);

// // // // Radar setup
// // // HardwareSerial radarSerial(1);
// // // LD2410RadarHal radarHal(radarSerial, RADAR_RX_PIN, RADAR_TX_PIN, logger);
// // // RadarDriver frontRadar(Id::FrontRadar, controller, radarHal, logger);
// // // IMU setup
// // // ESP32-S3: supports two I2C peripherals (0/1 = Wire/Wire1).
// // // ESP32-C3: only one I2C peripheral (0 = Wire).
// // // MPU6500   imu(Wire);
// // // I2cImuHal imuHal(imu, Wire, MPU6500_DEFAULT_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN);
// // // ImuDriver imuDrver(Id::FallDetector, controller, imuHal);
// // // // LED setup
// // // EspLedHal ledHal(LED0_PIN);
// // // LedDriver frontLed(Id::FrontLed, controller, ledHal, logger);

// // // // Speaker setup
// // // audio_tools::AudioSourceLittleFS source("/", ".mp3");
// // // audio_tools::MP3DecoderHelix decoder;
// // // I2SAudioHal audioHal(source, decoder, I2S_BCLK_PIN, I2S_LRCK_PIN, I2S_DATA_PIN);
// // // AudioDriver audioDriver(Id::FrontSpeaker, controller, audioHal, logger);

// // SmartSignage::SmartSignage(const UiHandles &ui, const char *configJson)
// //     : nvsConfigManager_{kNVSNamespace}, userIntf_{nvsConfigManager_, ui, configJson}, //
// //                                                                                       //
// //       ctrlQ_{}, radarQ_{}, imuQ_{}, ledQ_{}, audioQ_{},                               //
// //       //

// //       radarSerial_{1}, radarHal_{radarSerial_, RADAR_RX_PIN, RADAR_TX_PIN}, radarTimer_{},
// //       radarAo_{radarQ_, ctrlQ_, radarHal_, radarTimer_, "radarTask", 8192, tskIDLE_PRIORITY +
// 2,
// //       1},
// //       //
// //       imu_{Wire}, imuTimer_{},
// //       imuHal_{imu_, Wire, MPU6500_DEFAULT_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN},
// //       imuAo_{imuQ_, ctrlQ_, imuHal_, imuTimer_, "imuTask", 8192, tskIDLE_PRIORITY + 2, 1}, //

// //       //
// //       ledHal_{LED0_PIN}, ledTimer_{},
// //       ledAo_{ledQ_, ctrlQ_, ledHal_, ledTimer_, "ledTask", 8192, tskIDLE_PRIORITY + 2, 1}, //

// //       //
// //       ctrlFsm_{radarQ_, imuQ_, ledQ_, audioQ_}, audioFsm_{ctrlQ_}, //
// //       ctrlFsmLogger_{"ctrlFsmLogger"}, audioFsmLogger_{"audioFsmLogger"},
// //       ctrlAo_{ctrlQ_, ctrlFsm_, ctrlFsmLogger_, "ctrlTask", 8192, tskIDLE_PRIORITY + 2, 1},

// //       audioAo_{audioQ_, audioFsm_, audioFsmLogger_, "audioTask", 8192, tskIDLE_PRIORITY + 2,
// 1}
// //       {}

// // void SmartSignage::setup() {
// //     SS_LOGI("SmartSignage setup");
// //     LittleFS.begin();
// //     userIntf_.setup();
// //     ctrlQ_.post(ctrl::CmdSetup{});
// // }
// // void SmartSignage::loop() {}
// // void SmartSignage::dump_config() { SS_LOGI("SmartSignage ready"); }

// // } // namespace esphome::smart_signage
