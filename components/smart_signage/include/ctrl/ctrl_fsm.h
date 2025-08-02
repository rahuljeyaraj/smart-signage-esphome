#pragma once

#include <etl/variant.h>
#include "sml.hpp"
#include "log.h"

#include "ctrl/ctrl_event.h"
#include "radar/radar_q.h"

namespace radar = esphome::smart_signage::radar;

namespace esphome::smart_signage::ctrl {

class FSM {
    using Self = FSM;

  public:
    explicit FSM(radar::Q &radarQ);

    auto operator()() noexcept {
        using namespace boost::sml;
        return make_transition_table(
            // clang-format off
            *state<Idle>     + event<CmdSetup>      / &Self::onCmdSetup           = state<Setup>
            ,state<Setup>    + event<EvtRadarReady> [ &Self::ReadyGuard ]         = state<Ready>
            // ,state<Setup>    + event<EvtImuReady>   [ &Self::ReadyGuard ]         = state<Ready>
            // ,state<Setup>    + event<EvtLedReady>   [ &Self::ReadyGuard ]         = state<Ready>
            // ,state<Setup>    + event<EvtAudioReady> [ &Self::ReadyGuard ]         = state<Ready>
            ,state<Setup>    + event<EvtTimeout>    / &Self::onSetupTimeout       = state<Error>
            ,state<Ready>    + event<CmdStart>      / &Self::onCmdStart           = state<Active>
            ,state<Ready>    + event<CmdTeardown>   / &Self::onCmdTeardown        = state<Idle>
            ,state<Active>   + event<CmdStop>       / &Self::onCmdStop            = state<Ready>
            ,state<Active>   + event<EvtTimeout>    / &Self::onActiveTimeout      = state<Idle>
            ,state<Active>   + event<EvtRadarData>  / &Self::onEvtRadarData       = state<Active>
            ,state<Active>   + event<EvtFell>       / &Self::onEvtFell            = state<Fallen>
            ,state<Fallen>   + event<EvtRose>       / &Self::onEvtRose            = state<Active>
            ,state<_>        + event<EvtRadarError>                               = state<Error>
            ,state<_>        + event<EvtImuError>                                 = state<Error>
            ,state<_>        + event<EvtLedError>                                 = state<Error>
            ,state<_>        + event<EvtAudioError>                               = state<Error>
            ,state<Error>    + on_entry<_>          / &Self::onError
            // clang-format on
        );
    }

  private:
    // Guards
    bool ReadyGuard(const EvtRadarReady &e);

    // Actions
    void onCmdSetup(const CmdSetup &);
    void onCmdStart(const CmdStart &);
    void onCmdStop(const CmdStop &);
    void onCmdTeardown(const CmdTeardown &);
    void onEvtRadarData(const EvtRadarData &);
    void onEvtFell(const EvtFell &);
    void onEvtRose(const EvtRose &);
    void onSetupTimeout(const EvtTimeout &);
    void onActiveTimeout(const EvtTimeout &);
    void onError();

    static constexpr char TAG[] = "ctrl";

    radar::Q &radarQ_;
    uint32_t  runTimeMins_{0};

    // State tags (no data)
    struct Idle {};
    struct Setup {};
    struct Ready {};
    struct Active {};
    struct Fallen {};
    struct Error {};
};

} // namespace esphome::smart_signage::ctrl
