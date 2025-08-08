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
    , filter_{radar::kMeasurementNoise, radar::kInitialError, radar::kProcessNoise}

    /*──────  Imu dependencies ────*/
    , imu_(Wire) //ESP32-S3: (I2C 0/1 = Wire/Wire1), ESP32-C3: (I2C 0 = Wire).
    , imuHal_(imu_, Wire, MPU6500_DEFAULT_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN)
    , imuPollTimer_()

    /*── Queues ──*/
    , ctrlQ_{}
    , radarQ_{}
    , imuQ_{}
    , ledQ_{}
    , audioQ_{}
    
    /*── FSMs ──*/
    , ctrlFsm_{radarQ_, imuQ_, ledQ_, audioQ_}
    , radarFsm_{ctrlQ_, radarHal_, filter_}
    , imuFsm_{ctrlQ_, imuHal_, imuPollTimer_}
    , ledFsm_{ctrlQ_}
    , audioFsm_{ctrlQ_}

    /*── Loggers ──*/
    , ctrlFsmLogger_{"ctrlFsmLogger"}
    , radarFsmLogger_{"radarFsmLogger"}
    , imuFsmLogger_{"imuFsmLogger"}
    , ledFsmLogger_{"ledFsmLogger"}
    , audioFsmLogger_{"audioFsmLogger"}

    /*── Active objects (tasks) ──*/
    , ctrlAo_{  ctrlQ_,     ctrlFsm_,   ctrlFsmLogger_,     "ctrlTask",     8192, tskIDLE_PRIORITY + 2, 1}
    , radarAo_{ radarQ_,    radarFsm_,  radarFsmLogger_,    "radarTask",    8192, tskIDLE_PRIORITY + 2, 1}
    , imuAo_{   imuQ_,      imuFsm_,    imuFsmLogger_,      "imuTask",      8192, tskIDLE_PRIORITY + 2, 1}
    , ledAo_{   ledQ_,      ledFsm_,    ledFsmLogger_,      "ledTask",      8192, tskIDLE_PRIORITY + 2, 1}
    , audioAo_{ audioQ_,    audioFsm_,  audioFsmLogger_,    "audioTask",    8192, tskIDLE_PRIORITY + 2, 1}
// clang-format on
{
    imuPollTimer_.create("imuPoll", &SmartSignage::imuPollCb, this);
}

void SmartSignage::setup() {
    LOGI("SmartSignage setup");
    userIntf_.setup();
    ctrlQ_.post(ctrl::CmdSetup{});
}
void SmartSignage::loop() {}
void SmartSignage::dump_config() { LOGI("SmartSignage ready"); }

void SmartSignage::imuPollCb(void *arg) {
    auto *self = static_cast<SmartSignage *>(arg);
    self->imuQ_.post(imu::EvtTimerPoll{});
}

} // namespace esphome::smart_signage

// // void print_littlefs_tree(const char *dirpath, int depth = 0) {
// //     static constexpr char TAG[] = "ss";
// //     DIR                  *dir   = opendir(dirpath);
// //     if (!dir) {
// //         LOGE("Could not open directory: %s", dirpath);
// //         return;
// //     }

// //     struct dirent *entry;
// //     while ((entry = readdir(dir)) != nullptr) {
// //         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

// //         // Construct full path
// //         char fullpath[256];
// //         snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);

// //         // Check if entry is a directory or file
// //         struct stat st;
// //         if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) {
// //             LOGI("[DIR]  %s", fullpath);
// //             // Recursive call for subdirectory
// //             print_littlefs_tree(fullpath, depth + 1);
// //         } else {
// //             LOGI("\t[FILE] %s", fullpath);
// //         }
// //     }
// //     closedir(dir);
// // }

// Config config;

// void SmartSignage::setup() {
//     LOGI("SmartSignage setup");

//     // bool ok = Config::set_radarDist("ss0", 6);
//     // LOGI("set_radarDist ok=%d", ok);

//     // LOGI("SmartSignage setup: mounting LittleFS...");

//     // esp_vfs_littlefs_conf_t conf = {
//     //     .base_path              = "/littlefs", // mount point
//     //     .partition_label        = "littlefs",  // must match your partition table
//     //     .format_if_mount_failed = true,
//     //     .dont_mount             = false,
//     // };
//     // esp_err_t err = esp_vfs_littlefs_register(&conf);
//     // if (err != ESP_OK) {
//     //     LOGE("Failed to mount LittleFS (%s)", esp_err_to_name(err));
//     // } else {
//     //     LOGI("Mounted LittleFS successfully at /littlefs");
//     // }
// }

// void SmartSignage::loop() {
//     // uint16_t dist = 0;
//     // bool     ok   = Config::get_radarDist("ss0", dist);
//     // LOGI("get_radarDist ok=%d, dist=%d", ok, dist);
//     // vTaskDelay(pdMS_TO_TICKS(2000));

//     // LOGI("SmartSignage loop: Listing LittleFS tree...");
//     // print_littlefs_tree("/littlefs");
//     // vTaskDelay(pdMS_TO_TICKS(2000));

//     // LOGI("SmartSignage loop");
//     // ctrlQ.post(ctrl::CmdSetup{});
//     // vTaskDelay(pdMS_TO_TICKS(100));
//     // ctrlQ.post(ctrl::CmdStart{});
//     // vTaskDelay(pdMS_TO_TICKS(100));
//     // ctrlQ.post(ctrl::CmdStop{});
//     // vTaskDelay(pdMS_TO_TICKS(100));
//     // ctrlQ.post(ctrl::CmdTeardown{});
//     // vTaskDelay(pdMS_TO_TICKS(500));
// }

// void SmartSignage::dump_config() { LOGI("SmartSignage component loaded"); }

// } // namespace esphome::smart_signage
