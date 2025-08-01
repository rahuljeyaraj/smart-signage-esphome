#pragma once
#include "active_object.h"
#include "log.h"
#include "sml.hpp"
#include <etl/variant.h>

namespace esphome::smart_signage {

// Events
struct Setup {};
struct Start {};
struct Stop {};
struct Teardown {};
struct TimerPoll {};
struct SetDistCm {
    uint16_t cm;
};
struct SetSampleInt {
    uint32_t ms;
};
struct SetCtrlAO {
    void *CtrlAO;
};

using RadarEvent =
    etl::variant<Setup, Start, Stop, Teardown, TimerPoll, SetDistCm, SetSampleInt, SetCtrlAO>;

// FSM
class RadarFSM {
    using Self = RadarFSM;

  public:
    RadarFSM() = default;

    auto operator()() noexcept {
        using namespace boost::sml;
        auto onSetupGuard = wrap(&Self::onSetup);
        return make_transition_table(
            // clang-format off
            *state<Idle>    + event<Setup>      [ !onSetupGuard ]   = state<Error>
            ,state<Idle>    + event<Setup>      [  onSetupGuard ]   = state<Ready>
            ,state<Ready>   + event<Start>      / &Self::onStart    = state<Active>
            ,state<Active>  + event<Stop>       / &Self::onStop     = state<Ready>
            ,state<Ready>   + event<Teardown>   / &Self::onTeardown = state<Idle>
            
            ,state<Active>  + event<TimerPoll>      / &Self::onPoll
            ,state<_>       + event<SetDistCm>      / &Self::onSetDist
            ,state<_>       + event<SetSampleInt>   / &Self::onSampleInt
            ,state<_>       + event<SetCtrlAO>      / &Self::onSetCtrlAO
            // clang-format on
        );
    }

    // Actions
    bool onSetup(const Setup &) {
        LOGI(TAG, "onSetup: initializing hardware...");
        if (!stubHardwareInit()) {
            LOGE(TAG, "onSetup: hardware init failed, throwing");
            return false;
        }
        LOGI(TAG, "onSetup: success");
        return true;
    }

    void onStart(const Start &) { LOGI(TAG, "onStart"); }

    void onStop(const Stop &) {
        LOGI(TAG, "onStop: stopping radar");
        // stubHardwareStop();
    }

    void onTeardown(const Teardown &) {
        LOGI(TAG, "onTeardown: tearing down");
        // stubHardwareTeardown();
    }

    void onPoll(const TimerPoll &) {
        LOGI(TAG, "onPoll: reading data (maxDist=%u cm, interval=%u ms)", detDistCm_, sampleIntMs_);
        // stubHardwarePoll();
    }

    void onSetDist(const SetDistCm &c) {
        detDistCm_ = c.cm;
        LOGI(TAG, "onSetDist: set max distance to %u cm", detDistCm_);
    }

    void onSampleInt(const SetSampleInt &c) {
        sampleIntMs_ = c.ms;
        LOGI(TAG, "onSampleInt: set sample interval to %u ms", sampleIntMs_);
    }

    void onSetCtrlAO(const SetCtrlAO &e) {
        ctrlAO_ = e.CtrlAO;
        LOGI(TAG, "CtrlAOconfigured: %p", ctrlAO_);
    }

  private:
    static constexpr char TAG[] = "ss";
    bool stubHardwareInit() { return true; }

    uint16_t detDistCm_{0};
    uint32_t sampleIntMs_{0};
    void *ctrlAO_{nullptr};

    // state tags (no data)
    struct Idle {};
    struct Ready {};
    struct Active {};
    struct Error {};
};

using RadarAO = ActiveObject<RadarFSM, RadarEvent>;

} // namespace esphome::smart_signage
