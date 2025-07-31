#include "smart_signage.h"
#include "radar_fsm.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace esphome::smart_signage {
RadarFSM radarFsm;
QueueHandle_t radarQueue = xQueueCreate(8, sizeof(RadarEvent));
RadarDriver radarDriver{radarFsm, radarQueue, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1};

void SmartSignage::setup() {}

void SmartSignage::loop() {
    // drive through states
    radarDriver.post(Configure{200, 1000});
    radarDriver.post(Activate{});
    radarDriver.post(TimerPoll{});
    radarDriver.post(Deactivate{});
    vTaskDelay(pdMS_TO_TICKS(100));
}

void SmartSignage::dump_config() { LOGI(TAG, "SmartSignage component loaded"); }

} // namespace esphome::smart_signage
