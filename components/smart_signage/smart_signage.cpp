#include "smart_signage.h"
#include "log.h"
#include "board.h"
#include "helper.h"

#include <LittleFS.h>
#include "audio/audio_event.h"

static constexpr char TAG[] = "SmartSignage";

namespace esphome::smart_signage {

SmartSignage::SmartSignage(const UiHandles &ui, const char *configJson)
    : nvsConfigManager_{kNVSNamespace}, userIntf_{nvsConfigManager_, ui, configJson}

      ,
      ctrlQ_{}, radarQ_{}, imuQ_{}, ledQ_{}, audioQ_{}

      ,
      radarSerial_{1}, radarHal_{radarSerial_, RADAR_RX_PIN, RADAR_TX_PIN}, radarTimer_{},
      radarAo_{radarQ_, ctrlQ_, radarHal_, radarTimer_, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1}

      ,
      imu_{Wire}, imuHal_{imu_, Wire, MPU6500_DEFAULT_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN},
      imuTimer_{},
      imuAo_{imuQ_, ctrlQ_, imuHal_, imuTimer_, "imuTask", 8192, tskIDLE_PRIORITY + 2, 1}

      ,
      ledHal_{LED0_PIN}, ledTimer_{},
      ledAo_{ledQ_, ctrlQ_, ledHal_, ledTimer_, "ledTask", 8192, tskIDLE_PRIORITY + 2, 1}

      ,
      audioHal_{I2S_BCLK_PIN, I2S_LRCK_PIN, I2S_DATA_PIN}, audioTimer_{}, audioAo_{audioQ_,
                                                                              ctrlQ_,
                                                                              audioHal_,
                                                                              audioTimer_,
                                                                              "audioTask",
                                                                              8192,
                                                                              tskIDLE_PRIORITY + 2,
                                                                              1} {}

void SmartSignage::setup() {
    SS_LOGI("SmartSignage setup");

    if (!LittleFS.begin(true)) {
        SS_LOGE("LittleFS mount failed");
        return;
    }

    print_partition_table();
    list_all_files();

    // Bring up audio pipeline
    audioQ_.post(audio::CmdSetup{});
    audioQ_.post(audio::CmdSetVolume{audio::kDefaultVolPct});

    // Quick test: play one file once
    audio::CmdPlay play{};
    play.spec.n = 3;

    auto set = [&](uint8_t i, const char *path, uint16_t gap_ms) {
        snprintf(play.spec.items[i].source, audio::kSourceStrLen, "%s", path);
        play.spec.items[i].gapMs = gap_ms; // gap AFTER this item
    };

    set(0, "/test/welcome_1.mp3", 500);   // 0.5s gap
    set(1, "/test/welcome_2.mp3", 500);   // 0.75s gap
    set(2, "/test/welcome_3.mp3", 2000); // 1.0s gap

    play.spec.loopCount = 2; // play the list twice (0 = infinite)
    audioQ_.post(play);

    // Later:
    // userIntf_.setup();
    // ctrlQ_.post(ctrl::CmdSetup{});
}

void SmartSignage::loop() {
    // No polling required for audio (HAL runs its copy task)
}

void SmartSignage::dump_config() { SS_LOGI("SmartSignage ready"); }

} // namespace esphome::smart_signage
