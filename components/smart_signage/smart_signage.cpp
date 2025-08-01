#include "smart_signage.h"
#include "active_object.h"
#include "ctrl_fsm.h"
#include "queue.h"
#include "radar_fsm.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace esphome::smart_signage {

using RadarAO = ActiveObject<radar::FSM, radar::RxEvent>;
using CtrlAO = ActiveObject<ctrl::FSM, ctrl::RxEvent>;

radar::FSM radarFsm;
ctrl::FSM ctrlFsm;

Queue radarQ(8, sizeof(radar::RxEvent));
Queue ctrlQ(8, sizeof(ctrl::RxEvent));

RadarAO radarAO{radarFsm, radarQ, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1};
CtrlAO ctrlAO{ctrlFsm, ctrlQ, "ctrlTask", 8192, tskIDLE_PRIORITY + 2, 1};

void SmartSignage::setup() {}

void SmartSignage::loop() {
    ctrl::RxEvent evt;

    evt = ctrl::Setup{};
    ctrlQ.post(&evt);
    evt = ctrl::Start{};
    ctrlQ.post(&evt);
    evt = ctrl::Timeout{};
    ctrlQ.post(&evt);
    vTaskDelay(pdMS_TO_TICKS(100));
}

void SmartSignage::dump_config() { LOGI(TAG, "SmartSignage component loaded"); }

} // namespace esphome::smart_signage
