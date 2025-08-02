#pragma once
#include "ctrl/ctrl_q.h"
#include "log.h"
#include "radar/radar_event.h"
#include "sml.hpp"
#include <etl/variant.h>

namespace esphome::smart_signage::radar {

class FSM {
    using Self = FSM;

  public:
    FSM(ctrl::Q &q) : ctrlQ_(q) {}

    auto operator()() noexcept {
        using namespace boost::sml;
        return make_transition_table(
            // clang-format off
            *state<Idle>    + event<CmdSetup>      [ !wrap(&Self::SetupGuard) ]     = state<Error>
            ,state<Idle>    + event<CmdSetup>      [  wrap(&Self::SetupGuard) ]     = state<Ready>
            ,state<Ready>   + event<CmdStart>      / &Self::onCmdStart              = state<Active>
            ,state<Active>  + event<CmdStop>       / &Self::onCmdStop               = state<Ready>
            ,state<Ready>   + event<CmdTeardown>   / &Self::onCmdTeardown           = state<Idle>

            ,state<Active>  + event<EvtTimerPoll>   / &Self::onEvtTimerPoll
            ,state<_>       + event<SetDistCm>      / &Self::onSetDist
            ,state<_>       + event<SetSampleInt>   / &Self::onSetSampleInt
            // clang-format on
        );
    }

    // Guard
    bool SetupGuard(const CmdSetup &e) {
        LOGI(TAG, "onSetup: initializing hardware...");
        if (!stubHardwareInit()) {
            LOGE(TAG, "onSetup: hardware init failed, throwing");
            ctrlQ_.post(ctrl::EvtRadarError{});
            return false;
        }
        LOGI(TAG, "onSetup: success");
        ctrlQ_.post(ctrl::EvtRadarReady{});
        return true;
    }

    // Actions
    void onCmdStart(const CmdStart &) { LOGI(TAG, "onStart"); }

    void onCmdStop(const CmdStop &) {
        LOGI(TAG, "onStop: stopping radar");
        // stubHardwareStop();
    }

    void onCmdTeardown(const CmdTeardown &) {
        LOGI(TAG, "onTeardown: tearing down");
        // stubHardwareTeardown();
    }

    void onEvtTimerPoll(const EvtTimerPoll &) {
        LOGI(TAG, "onPoll: reading data (maxDist=%u cm, interval=%u ms)", detDistCm_, sampleIntMs_);
        ctrlQ_.post(ctrl::EvtRadarData{true, 100});
        // stubHardwarePoll();
    }

    void onSetDist(const SetDistCm &c) {
        detDistCm_ = c.cm;
        LOGI(TAG, "onSetDist: set max distance to %u cm", detDistCm_);
    }

    void onSetSampleInt(const SetSampleInt &c) {
        sampleIntMs_ = c.ms;
        LOGI(TAG, "onSetSampleInt: set sample interval to %u ms", sampleIntMs_);
    }

  private:
    static constexpr char TAG[] = "radar";

    ctrl::Q &ctrlQ_;

    bool stubHardwareInit() { return true; }

    uint16_t detDistCm_{0};
    uint32_t sampleIntMs_{0};
    // void *ctrlAo_{nullptr};

    // state tags (no data)
    struct Idle {};
    struct Ready {};
    struct Active {};
    struct Error {};
};

} // namespace esphome::smart_signage::radar
