#include "smart_signage.h"
#include "log.h"
#include "board.h"
#include "helper.h"

#include <LittleFS.h>
#include <cstdio>

static constexpr char TAG[] = "SmartSignage";

namespace esphome::smart_signage {

SmartSignage::SmartSignage(const UiHandles &ui, const char *configJson)
    : ctrlQ_{}, radarQ_{}, imuQ_{}, ledQ_{}, audioQ_{}, configJson_{configJson}, profilesCfg_{},
      ui_{ui, ctrlQ_} {}
// :  profileDb_{configJson}
//   ,nvsConfigManager_{kNVSNamespace}, userIntf_{nvsConfigManager_, ui, configJson}

//   ,

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

    etl::vector<ProfileName, SS_MAX_PROFILES> names;
    profilesCfg_.getProfileList(names);

    // Push options to UI (stateless)
    ui_.set_profile_options(names);

    // Jump to a specific label(stateless)
    ui_.set_current_profile(names[1]);

    // 0) parse profiles JSON if you want UI options later
    if (!profilesCfg_.init(configJson_)) {
        SS_LOGW("profilesCfg init failed (continuing NVS smoke test)");
    }

    // 1) init NVS once
    if (!NvsSmartSignage::initNvs()) {
        SS_LOGE("NVS init failed");
        return;
    }

    // 2) ensure current profile + load values (defaults if missing)
    ProfileName   currName;
    ProfileValues currVals;
    if (!NvsSmartSignage::loadCurrentOrDefault(names[0], currName, currVals)) {
        SS_LOGE("loadCurrentOrDefault failed");
        return;
    }
    SS_LOGI("current=\"%s\" mins=%u range=%u vol=%u bright=%u",
        currName.c_str(),
        (unsigned) currVals.sessionMins,
        (unsigned) currVals.radarRangeCm,
        (unsigned) currVals.audioVolPct,
        (unsigned) currVals.ledBrightPct);

    // 3) publish to UI (stateless bridge)
    ui_.set_current_profile(currName);
    ui_.set_session_mins(currVals.sessionMins);
    ui_.set_radar_range_cm(currVals.radarRangeCm);
    ui_.set_audio_vol_pct(currVals.audioVolPct);
    ui_.set_led_bright_pct(currVals.ledBrightPct);

    // 4) mutate values, save, and read back to verify
    currVals.sessionMins += 1;
    currVals.radarRangeCm += 5;
    if (currVals.audioVolPct < 100) currVals.audioVolPct += 1;
    if (currVals.ledBrightPct < 100) currVals.ledBrightPct += 2;

    if (!NvsSmartSignage::saveProfile(currName, currVals)) {
        SS_LOGE("saveProfile failed for \"%s\"", currName.c_str());
    } else {
        ProfileValues verify{};
        if (NvsSmartSignage::loadProfile(currName, verify)) {
            SS_LOGI("verify \"%s\" mins=%u range=%u vol=%u bright=%u",
                currName.c_str(),
                (unsigned) verify.sessionMins,
                (unsigned) verify.radarRangeCm,
                (unsigned) verify.audioVolPct,
                (unsigned) verify.ledBrightPct);
        }
    }

    // 5) create/switch to another profile and verify
    ProfileName   otherName = "pqrs";
    ProfileValues otherVals{};
    otherVals.sessionMins  = 15;
    otherVals.radarRangeCm = 250;
    otherVals.audioVolPct  = 60;
    otherVals.ledBrightPct = 40;

    NvsSmartSignage::saveProfile(otherName, otherVals);
    NvsSmartSignage::setCurrentProfile(otherName);

    ProfileName   readCurr;
    ProfileValues readVals;
    NvsSmartSignage::loadCurrentOrDefault(readCurr, readVals);

    SS_LOGI("switched current=\"%s\" mins=%u range=%u vol=%u bright=%u",

        readCurr.c_str(),
        (unsigned) readVals.sessionMins,
        (unsigned) readVals.radarRangeCm,
        (unsigned) readVals.audioVolPct,
        (unsigned) readVals.ledBrightPct);

    // push the new current selection to UI (optional)
    ui_.set_current_profile(readCurr);
    ui_.set_session_mins(readVals.sessionMins);
    ui_.set_radar_range_cm(readVals.radarRangeCm);
    ui_.set_audio_vol_pct(readVals.audioVolPct);
    ui_.set_led_bright_pct(readVals.ledBrightPct);

    // 6) (optional) populate UI select with names from ProfilesConfig
    {
        etl::vector<ProfileName, SS_MAX_PROFILES> names;
        profilesCfg_.getProfileList(names); // returns ProfileName list (15-char)
        if (!names.empty()) {
            ui_.set_profile_options(names);
            // ensure UI shows whichever is current in NVS
            ui_.set_current_profile(readCurr);
        }
    }

    SS_LOGI("setup end (NVS smoke test complete)");

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
