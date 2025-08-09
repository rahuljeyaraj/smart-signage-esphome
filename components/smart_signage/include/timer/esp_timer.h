#pragma once
#include "timer/itimer.h"
#include <esp_timer.h>
#include "log.h"

namespace esphome::smart_signage::timer {

class EspTimer : public ITimer {
  public:
    EspTimer() = default;
    ~EspTimer() override {
        if (handle_) {
            esp_timer_delete(handle_);
            SS_LOGI("Deleted timer");
        }
    }

    bool create(const char *name, Callback cb, void *arg) override {
        esp_timer_create_args_t args{};
        args.callback        = cb;
        args.arg             = arg;
        args.dispatch_method = ESP_TIMER_TASK;
        args.name            = name;

        esp_err_t err = esp_timer_create(&args, &handle_);
        if (err != ESP_OK) {
            SS_LOGE("esp_timer_create(%s) failed: %d", name, err);
            handle_ = nullptr;
            return false;
        }
        SS_LOGI("Created timer %s", name);
        return true;
    }

    void startOnce(uint64_t timeout_us) override {
        if (!handle_) return;
        esp_err_t err = esp_timer_start_once(handle_, timeout_us);
        if (err != ESP_OK) { SS_LOGE("startOnce failed: %d", err); }
    }

    void startPeriodic(uint64_t period_us) override {
        if (!handle_) return;
        esp_err_t err = esp_timer_start_periodic(handle_, period_us);
        if (err != ESP_OK) { SS_LOGE("startPeriodic failed: %d", err); }
    }

    void stop() override {
        if (!handle_) return;
        esp_err_t err = esp_timer_stop(handle_);
        if (err != ESP_OK) { SS_LOGE("stop failed: %d", err); }
    }

  private:
    esp_timer_handle_t handle_{nullptr};

    static constexpr char TAG[] = "EspTimer";
};

} // namespace esphome::smart_signage::timer
