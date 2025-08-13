#include "smart_signage.h"
#include "log.h"
#include "board.h"
#include "helper.h"

#include <LittleFS.h>

static constexpr char TAG[] = "SmartSignage";

namespace esphome::smart_signage {

SmartSignage::SmartSignage(const UiHandles &ui, const char *configJson)
    : configJson_{configJson}, profilesCfg_{} {}
// :  profileDb_{configJson}
//   ,nvsConfigManager_{kNVSNamespace}, userIntf_{nvsConfigManager_, ui, configJson}

//   ,
//   ctrlQ_{}, radarQ_{}, imuQ_{}, ledQ_{}, audioQ_{}

//   ,
//   ctrlFsm_{radarQ_, imuQ_, ledQ_, audioQ_},
//   ctrlLogger_{"CtrlFSM"},
//   ctrlAo_{ctrlQ_, ctrlFsm_, ctrlLogger_, "ctrlTask", 8192, tskIDLE_PRIORITY + 2, 1}

//   ,
//   radarSerial_{1}, radarHal_{radarSerial_, RADAR_RX_PIN, RADAR_TX_PIN}, radarTimer_{},
//   radarAo_{radarQ_, ctrlQ_, radarHal_, radarTimer_, "radarTask", 8192, tskIDLE_PRIORITY + 2,
//   1}

//   ,
//   imu_{Wire}, imuHal_{imu_, Wire, MPU6500_DEFAULT_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN},
//   imuTimer_{},
//   imuAo_{imuQ_, ctrlQ_, imuHal_, imuTimer_, "imuTask", 8192, tskIDLE_PRIORITY + 2, 1}

//   ,
//   ledHal_{LED0_PIN}, ledTimer_{},
//   ledAo_{ledQ_, ctrlQ_, ledHal_, ledTimer_, "ledTask", 8192, tskIDLE_PRIORITY + 2, 1}

//   ,
//   audioHal_{I2S_BCLK_PIN, I2S_LRCK_PIN, I2S_DATA_PIN}, audioTimer_{}, audioAo_{audioQ_,
//                                                                           ctrlQ_,
//                                                                           audioHal_,
//                                                                           audioTimer_,
//                                                                           "audioTask",
//                                                                           8192,
//                                                                           tskIDLE_PRIORITY +
//                                                                           2, 1}

void SmartSignage::setup() {
    if (!profilesCfg_.init(configJson_)) {
        SS_LOGE("Config Json Parsing Failed.");
        return;
    }

    // for (uint8_t i = 0; i < count; ++i) {
    //     ProfileSummary summary{};
    //     if (!profileDb_.get_summary(i, summary)) {
    //         SS_LOGW("summary[%hhu] fetch failed", i);
    //         continue;
    //     }
    //     SS_LOGI("summary[%hhu]: id='%s', name='%s'", i, summary.id, summary.name);
    // }

    // SS_LOGI("SmartSignage setup");

    // if (!LittleFS.begin(true)) {
    //     SS_LOGE("LittleFS mount failed");
    //     return;
    // }

    // print_partition_table();
    // list_all_files();
    // ctrlQ_.post(ctrl::CmdSetup{});
}

void SmartSignage::loop() {
    // No polling required for audio (HAL runs its copy task)
}

void SmartSignage::dump_config() { SS_LOGI("SmartSignage ready"); }

} // namespace esphome::smart_signage
