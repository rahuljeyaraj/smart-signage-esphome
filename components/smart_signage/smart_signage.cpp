#include "smart_signage.h"

namespace esphome::smart_signage {

RadarFSM radarFsm;
QueueHandle_t radarQueue = xQueueCreate(8, sizeof(RadarEvent));
RadarAO radarAO{radarFsm, radarQueue, "radarTask", 8192, tskIDLE_PRIORITY + 2, 1};

void SmartSignage::setup() {}

void SmartSignage::loop() {
    // drive through states

    radarAO.post(Setup{});
    radarAO.post(Start{});
    radarAO.post(Stop{});
    radarAO.post(Teardown{});
    vTaskDelay(pdMS_TO_TICKS(100));
}

void SmartSignage::dump_config() { LOGI(TAG, "SmartSignage component loaded"); }

} // namespace esphome::smart_signage
