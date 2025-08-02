#include "smart_signage.h"
// #include "ctrl_fsm.h"
// #include "events.h"
#include "radar/ao.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace esphome::smart_signage {

// namespace ctrl {
// using Q = Queue<RxEvent, 16>; // TODO, get the size from const
// }

radar::Q radarQ;
radar::FSM radarFsm;
radar::AO radarAo(radarFsm, radarQ, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1);
// ctrl::FSM ctrlFsm(ctrlQ, radarQ);


// using CtrlAO = ActiveObject<ctrl::FSM, ctrl::Q>;

// RadarAO radarAo(radarFsm, radarQ, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1);
// CtrlAO ctrlAo(ctrlFsm, ctrlQ, "ctrlTask", 8192, tskIDLE_PRIORITY + 2, 1);

void SmartSignage::setup() {}

void SmartSignage::loop() {

    radarQ.post(radar::Setup{});
    // evt = ctrl::Start{};
    // ctrlQ.post(&evt);
    // evt = ctrl::Timeout{};
    // ctrlQ.post(&evt);
    vTaskDelay(pdMS_TO_TICKS(100));
}

void SmartSignage::dump_config() { LOGI(TAG, "SmartSignage component loaded"); }

} // namespace esphome::smart_signage
