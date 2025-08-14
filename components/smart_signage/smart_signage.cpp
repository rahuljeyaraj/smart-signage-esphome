#include "smart_signage.h"
#include "log.h"
#include "board.h"
#include "helper.h"

#include <LittleFS.h>
#include <cstdio>

static constexpr char TAG[] = "SmartSignage";

namespace esphome::smart_signage {

SmartSignage::SmartSignage(const UiHandles &uiHandles, const char *configJson)
    // clang-format off
    : ctrlQ_{}, radarQ_{}, imuQ_{}, ledQ_{}, audioQ_{}
    
    , ui_{uiHandles, ctrlQ_}, configJson_{configJson}, profilesCfg_{}, ctrlTimer_{}
    , storage_{kNVSNamespace}
    , profileCatalog_{}, profileSettings_{storage_}
    , ctrlAo_{ctrlQ_, radarQ_, imuQ_, ledQ_, audioQ_, ctrlTimer_,profilesCfg_, ui_, "ctrlTask", 8192, tskIDLE_PRIORITY + 2, 1}

    , radarSerial_{1}
    , radarHal_{radarSerial_, RADAR_RX_PIN, RADAR_TX_PIN}
    , radarTimer_{}
    , radarAo_{radarQ_, ctrlQ_, radarHal_, radarTimer_, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1}

    , imu_{Wire}
    , imuHal_{imu_, Wire, MPU6500_DEFAULT_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN}
    , imuTimer_{}
    , imuAo_{imuQ_, ctrlQ_, imuHal_, imuTimer_, "imuTask", 8192, tskIDLE_PRIORITY + 2, 1}

    , ledHal_{LED0_PIN}
    , ledTimer_{}
    , ledAo_{ledQ_, ctrlQ_, ledHal_, ledTimer_, "ledTask", 8192, tskIDLE_PRIORITY + 2, 1}

    , audioHal_{I2S_BCLK_PIN, I2S_LRCK_PIN, I2S_DATA_PIN}
    , audioTimer_{} 
    , audioAo_{audioQ_, ctrlQ_, audioHal_, audioTimer_, "audioTask", 8192, tskIDLE_PRIORITY + 2, 1} {}
// clang-format on

void SmartSignage::setup() {
    SS_LOGE("---------------3");
    // if (!profilesCfg_.init(configJson_)) {
    //     SS_LOGE("Config Json Parsing Failed.");
    //     return;
    // }

    if (!profileCatalog_.init(configJson_)) {
        SS_LOGE("Config Json Parsing Failed.");
        return;
    }

    if (!storage_.init()) {
        SS_LOGE("NVS init failed");
        return;
    }

    const ProfileName name = "abcd";
    profileSettings_.writeCurrentProfile(name);
    ProfileName readname;
    bool        ok = true;
    ok             = profileSettings_.readCurrentProfile(readname);
    SS_LOGI("readname: %d %s", ok, readname.c_str());

    profile::ProfileValues val{};
    val.audioVolPct  = 50;
    val.ledBrightPct = 55;
    val.sessionMins  = 60;
    val.radarRangeCm = 300;
    profileSettings_.writeProfileValues(name, val);
    profile::ProfileValues newVal{};

    ok = profileSettings_.readProfileValues(name, newVal);
    SS_LOGI("readname: %d audioVolPct:%d, ledBrightPct: %d, sessionMins: %d, radarRangeCm:%d",
        ok,
        val.audioVolPct  = 50,
        val.ledBrightPct = 55,
        val.sessionMins  = 60,
        val.radarRangeCm = 300);

    ProfileNames names;
    profileCatalog_.getProfileNames(names);
    SS_LOGI("Found %u profile option(s)", (unsigned) names.size());
    if (names.empty()) {
        SS_LOGW("No profiles found");
    } else {
        for (size_t i = 0; i < names.size(); ++i) {
            SS_LOGI("Option %u: %s", (unsigned) i, names[i].c_str());
        }
    }

    // if (!NvsSmartSignage::initNvs()) {
    //     SS_LOGE("NVS init failed");
    //     return;
    // }

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

    print_partition_table();
    list_all_files();
    ctrlQ_.post(ctrl::CmdSetup{});
}

void SmartSignage::loop() {
    // No polling required for audio (HAL runs its copy task)
}

void SmartSignage::dump_config() { SS_LOGI("SmartSignage ready"); }

} // namespace esphome::smart_signage
