#pragma once
#include "iled_hal.h"
#include "driver/ledc.h"
#include "esp_err.h"

namespace esphome::smart_signage::led::hal {

/// Macro to turn a percent [0–100] into a raw duty [0–maxDuty].
#define CALC_DUTY(percent, maxDuty) (uint32_t) (((uint32_t) (percent) * (maxDuty)) / 100U)

/// Local error macro: If ESP-IDF call fails, return false from the current function.
#define RETURN_FALSE_IF_ERR(expr)                                                                  \
    do {                                                                                           \
        if ((expr) != ESP_OK) return false;                                                        \
    } while (0)

class EspLedHal : public ILedHal {
  public:
    EspLedHal(int pin, ledc_channel_t channel = LEDC_CHANNEL_0, ledc_timer_t timer = LEDC_TIMER_0,
        ledc_mode_t speedMode = LEDC_LOW_SPEED_MODE, uint32_t freqHz = 5000,
        ledc_timer_bit_t resolution = LEDC_TIMER_8_BIT)
        : pin_(pin), channel_(channel), timer_(timer), speedMode_(speedMode), freqHz_(freqHz),
          resolution_(resolution), maxDuty_((1u << resolution) - 1), fadeEndCb_(nullptr),
          fadeEndCbCtx_(nullptr) {}

    void setFadeEndCallback(FadeEndCb cb, void *cbCtx) override {
        fadeEndCb_    = cb;
        fadeEndCbCtx_ = cbCtx;
    }

    bool init() override {

        RETURN_FALSE_IF_ERR(ledc_fade_func_install(0));

        ledc_timer_config_t ledc_timer = {};
        ledc_timer.speed_mode          = speedMode_;
        ledc_timer.timer_num           = timer_;
        ledc_timer.duty_resolution     = resolution_;
        ledc_timer.freq_hz             = freqHz_;
        ledc_timer.clk_cfg             = LEDC_AUTO_CLK;
        RETURN_FALSE_IF_ERR(ledc_timer_config(&ledc_timer));

        ledc_channel_config_t ledc_channel = {};
        ledc_channel.channel               = channel_;
        ledc_channel.duty                  = 0;
        ledc_channel.gpio_num              = pin_;
        ledc_channel.speed_mode            = speedMode_;
        ledc_channel.hpoint                = 0;
        ledc_channel.timer_sel             = timer_;
        ledc_channel.flags.output_invert   = 0;
        ledc_channel.intr_type             = fadeEndCb_ ? LEDC_INTR_FADE_END : LEDC_INTR_DISABLE;
        RETURN_FALSE_IF_ERR(ledc_channel_config(&ledc_channel));

        if (fadeEndCb_) {
            ledc_cbs_t cbs = {.fade_cb = &EspLedHal::trampolineISR};
            RETURN_FALSE_IF_ERR(ledc_cb_register(speedMode_, channel_, &cbs, this));
        }

        return true;
    }

    bool setBrightness(uint8_t percent) override {
        uint32_t duty = calcDuty(percent, maxDuty_);
        RETURN_FALSE_IF_ERR(ledc_set_duty(speedMode_, channel_, duty));
        RETURN_FALSE_IF_ERR(ledc_update_duty(speedMode_, channel_));
        return true;
    }

    bool turnOff() override { return ledc_stop(speedMode_, channel_, 0) == ESP_OK; }

    bool fadeTo(uint8_t targetPercent, uint32_t durationMs) override {
        uint32_t duty = calcDuty(targetPercent, maxDuty_);
        RETURN_FALSE_IF_ERR(ledc_set_fade_with_time(speedMode_, channel_, duty, durationMs));
        RETURN_FALSE_IF_ERR(ledc_fade_start(speedMode_, channel_, LEDC_FADE_NO_WAIT));
        return true;
    }

  private:
    static bool IRAM_ATTR trampolineISR(const ledc_cb_param_t *, void *ctx) {
        auto *self = static_cast<EspLedHal *>(ctx);
        if (self->fadeEndCb_) self->fadeEndCb_(self->fadeEndCbCtx_);
        return false;
    }

    inline uint32_t calcDuty(uint8_t percent, uint32_t maxDuty) {
        if (percent > 100) percent = 100;
        return ((uint32_t) percent * maxDuty) / 100U;
    }

    int              pin_;
    ledc_channel_t   channel_;
    ledc_timer_t     timer_;
    ledc_mode_t      speedMode_;
    uint32_t         freqHz_;
    ledc_timer_bit_t resolution_;
    const uint32_t   maxDuty_;

    FadeEndCb fadeEndCb_;
    void     *fadeEndCbCtx_;

    static constexpr const char *TAG = "EspLedHal";
};

#undef CALC_DUTY
#undef RETURN_FALSE_IF_ERR

} // namespace esphome::smart_signage::led::hal
