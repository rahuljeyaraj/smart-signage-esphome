#include "esphome/components/smart_signage/smart_signage.h"
#include "log.h"
#include "board.h"
#include "helper.h"

#include <LittleFS.h>
#include <cstdio>

static constexpr char TAG[] = "SmartSignage";

namespace esphome::smart_signage {

SmartSignage::SmartSignage(const UiHandles &uiHandles, const char *catalogJson)
    // clang-format off
    : ctrlQ_{}, radarQ_{}, imuQ_{}, ledQ_{}, audioQ_{}
    , ui_{uiHandles, ctrlQ_}
    , catalogJson_{catalogJson}
    // Ctrl / Settings: storage first, then settings (inject storage), then timer, then AO
    , storage_{kNVSNamespace}
    , profileCatalog_{}
    , profileSettings_{storage_, profileCatalog_}
    , ctrlTimer_{}
    , ctrlAo_{ctrlQ_, radarQ_, imuQ_, ledQ_, audioQ_, ctrlTimer_, profileSettings_, ui_, "ctrlTask", 8192, tskIDLE_PRIORITY + 2, 1}
    // Radar
    , radarSerial_{1}
    , radarHal_{radarSerial_, RADAR_RX_PIN, RADAR_TX_PIN}
    , radarTimer_{}
    , radarAo_{radarQ_, ctrlQ_, radarHal_, radarTimer_, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1}
    // IMU
    , imu_{Wire}
    , imuHal_{imu_, Wire, MPU6500_DEFAULT_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN}
    , imuTimer_{}
    , imuAo_{imuQ_, ctrlQ_, imuHal_, imuTimer_, "imuTask", 8192, tskIDLE_PRIORITY + 2, 1}
    // LED
    , ledHal_{LED0_PIN}
    , ledTimer_{}
    , ledAo_{ledQ_, ctrlQ_, ledHal_, ledTimer_, "ledTask", 8192, tskIDLE_PRIORITY + 2, 1}
    // Audio
    , audioHal_{I2S_BCLK_PIN, I2S_LRCK_PIN, I2S_DATA_PIN}
    , audioTimer_{}
    , audioAo_{audioQ_, ctrlQ_, audioHal_, audioTimer_, "audioTask", 8192, tskIDLE_PRIORITY + 2, 1}
{} // clang-format on

void SmartSignage::setup() {
    // Initialize NVS (explicit)
    if (!storage_.init()) {
        SS_LOGE("NVS init failed");
        return;
    }

    if (!profileCatalog_.init(catalogJson_)) {
        SS_LOGE("Config Json Parsing Failed.");
        return;
    }

    // Filesystem mount (optional based on your project)
    if (!LittleFS.begin(true)) {
        SS_LOGE("LittleFS mount failed");
        return;
    }

    print_partition_table();
    list_all_files();

    SS_LOGI("Kicking Ctrl to start");
    ctrlQ_.post(ctrl::CmdSetup{});
}

void SmartSignage::loop() {
    // No polling required for audio (HAL runs its copy task)
}

void SmartSignage::dump_config() { SS_LOGI("SmartSignage ready"); }

} // namespace esphome::smart_signage
