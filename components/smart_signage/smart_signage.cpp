#include "smart_signage.h"
#include "ctrl/ctrl_ao.h"
#include "radar/radar_ao.h"

namespace esphome::smart_signage {

ctrl::Q  ctrlQ;
radar::Q radarQ;

ctrl::FSM  ctrlFsm(radarQ);
radar::FSM radarFsm(ctrlQ);

ctrl::AO  ctrlAo(ctrlFsm, ctrlQ, "ctrlTask", 8192, tskIDLE_PRIORITY + 2, 1);
radar::AO radarAo(radarFsm, radarQ, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1);

void SmartSignage::setup() { LOGI(TAG, "SmartSignage setup"); }

void SmartSignage::loop() {

    LOGI(TAG, "SmartSignage loop");
    ctrlQ.post(ctrl::CmdSetup{});
    vTaskDelay(pdMS_TO_TICKS(100));
    ctrlQ.post(ctrl::CmdStart{});
    vTaskDelay(pdMS_TO_TICKS(100));
    ctrlQ.post(ctrl::CmdStop{});
    vTaskDelay(pdMS_TO_TICKS(100));
    ctrlQ.post(ctrl::CmdTeardown{});
    vTaskDelay(pdMS_TO_TICKS(500));
}

void SmartSignage::dump_config() { LOGI(TAG, "SmartSignage component loaded"); }

} // namespace esphome::smart_signage
