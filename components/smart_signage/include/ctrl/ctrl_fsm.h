#pragma once
#include "ctrl/ctrl_event.h"
#include "log.h"
#include "radar/radar_q.h"
#include "sml.hpp"
#include <etl/variant.h>

namespace radar = esphome::smart_signage::radar;

namespace esphome::smart_signage::ctrl {

class FSM {
    using Self = FSM;

  public:
    FSM(radar::Q &radarQ) : radarQ_(radarQ) {}

    auto operator()() noexcept {
        using namespace boost::sml;

        return make_transition_table(
            // clang-format off
            *state<Idle> + event<CmdSetup> / &Self::onCmdSetup           = state<Setup>
            ,state<Setup> + event<EvtIntfReady>[&Self::ReadyGuard]       = state<Ready>
            ,state<Setup> + event<EvtTimeout> / &Self::onSetupTimeout    = state<Error>
            ,state<Ready> + event<CmdStart> / &Self::onCmdStart          = state<Active>
            ,state<Active> + event<CmdStop> / &Self::onCmdStop           = state<Ready>
            ,state<Active> + event<EvtTimeout> / &Self::onActiveTimeout  = state<Idle>
            ,state<Active> + event<EvtRadarData> / &Self::onEvtRadarData = state<Active>
            ,state<Active> + event<EvtFell> / &Self::onEvtFell           = state<Fallen>
            ,state<Fallen> + event<EvtRose> / &Self::onEvtRose           = state<Active>
            ,state<_> + event<EvtIntfError> / &Self::onEvtIntfError      = state<Error>
            // clang-format on
        );
    }

    // Guards
    bool ReadyGuard(const EvtIntfReady &e) {
        // readySeen_.insert(e);
        // etl::visit([this](auto const &ev) { readySeen_.insert(ev); }, e);
        // return readySeen_.size() == kIntfCnt;
        return true;
    }

    // Actions
    void onCmdSetup(const CmdSetup &) {
        radarQ_.post(radar::CmdSetup{});
        LOGI(TAG, "onSetup");
    }

    void onCmdStart(const CmdStart &e) {
        radarQ_.post(radar::CmdStart{});

        runTimeMins_ = e.runTimeMins;
        LOGI(TAG, "onStart: runTimeMins=%u", runTimeMins_);
        // schedule Timeout via your automation API...
    }

    void onCmdStop(const CmdStop &e) {
        radarQ_.post(radar::CmdStop{});
        LOGI(TAG, "onStop");
    }

    void onEvtRadarData(const EvtRadarData &e) {
        LOGI(TAG,
            "onRadarData: detected=%s dist=%u cm ts=%u",
            e.detected ? "Y" : "N",
            static_cast<unsigned>(e.distanceCm),
            static_cast<unsigned>(e.timestampTicks));
    }

    void onEvtFell(const EvtFell &) { LOGI(TAG, "onFell: fall detected"); }
    void onEvtRose(const EvtRose &) { LOGI(TAG, "onRose: rise detected"); }

    void onSetupTimeout(const EvtTimeout &) { LOGI(TAG, "onSetupTimeout: error"); }

    void onActiveTimeout(const EvtTimeout &) {
        radarQ_.post(radar::CmdStop{});
        radarQ_.post(radar::CmdTeardown{});

        LOGI(TAG, "onActiveTimeout: tearing down");
    }

    void onEvtIntfError(const EvtIntfError &) { LOGE(TAG, "InitError!"); }

  private:
    static constexpr char TAG[] = "ctrl";

    radar::Q &radarQ_;
    uint32_t  runTimeMins_{0};

    // ––– State tags –––––––––––––––––––––––––––––––––––––
    struct Idle {};
    struct Setup {};
    struct Ready {};
    struct Active {};
    struct Fallen {};
    struct Error {};
};

} // namespace esphome::smart_signage::ctrl
