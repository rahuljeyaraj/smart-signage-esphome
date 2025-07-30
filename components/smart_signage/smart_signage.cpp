#include "logger.h"
#include "smart_signage.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
namespace esphome
{
    namespace smart_signage
    {

        static const char *TAG = "ss";

        void SmartSignage::setup()
        {

            LOGI(TAG, "SmartSignage: setup");
        }

        void SmartSignage::loop()
        {
            LOGI(TAG, "SmartSignage: loop");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        void SmartSignage::dump_config()
        {
            LOGC(TAG, "SmartSignage");
        }

    } // namespace smart_signage
} // namespace esphome