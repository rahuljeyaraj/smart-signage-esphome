#include "smart_signage.h"
#include "ctrl/ctrl_ao.h"
#include "radar/radar_ao.h"
#include "imu/imu_ao.h"
#include "fsm_logger.h"

namespace esphome::smart_signage {

ctrl::Q  ctrlQ;
radar::Q radarQ;
imu::Q   imuQ;

ctrl::FSM  ctrlFsm(radarQ, imuQ);
radar::FSM radarFsm(ctrlQ);
imu::FSM   imuFsm(ctrlQ);

FsmLogger ctrlFsmLogger("ctrlFsmLogger");
FsmLogger radarFsmLogger("radarFsmLogger");
FsmLogger imuFsmLogger("imuFsmLogger");

ctrl::AO  ctrlAo(ctrlQ, ctrlFsm, ctrlFsmLogger, "ctrlTask", 8192, tskIDLE_PRIORITY + 2, 1);
radar::AO radarAo(radarQ, radarFsm, radarFsmLogger, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1);
imu::AO   imurAo(imuQ, imuFsm, imuFsmLogger, "imuTask", 8192, tskIDLE_PRIORITY + 2, 1);

void SmartSignage::setup() { LOGI("SmartSignage setup"); }

void SmartSignage::loop() {

    LOGI("SmartSignage loop");
    ctrlQ.post(ctrl::CmdSetup{});
    vTaskDelay(pdMS_TO_TICKS(100));
    ctrlQ.post(ctrl::EvtLedReady{});
    ctrlQ.post(ctrl::EvtAudioReady{});
    vTaskDelay(pdMS_TO_TICKS(100));
    ctrlQ.post(ctrl::CmdStart{});
    vTaskDelay(pdMS_TO_TICKS(100));
    ctrlQ.post(ctrl::CmdStop{});
    vTaskDelay(pdMS_TO_TICKS(100));
    ctrlQ.post(ctrl::CmdTeardown{});
    vTaskDelay(pdMS_TO_TICKS(500));
}

void SmartSignage::dump_config() { LOGI("SmartSignage component loaded"); }

} // namespace esphome::smart_signage
