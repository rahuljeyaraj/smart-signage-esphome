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
    explicit FSM(ctrl::Q &q);

    auto operator()() noexcept {
        using namespace boost::sml;
        return make_transition_table(
            // clang-format off
            *state<Idle>     + event<CmdSetup>      [ &Self::isReadyGuard ]            = state<Ready>
            ,state<Idle>     + event<CmdSetup>                                       = state<Error>
            ,state<Ready>    + event<CmdStart>      / &Self::onCmdStart              = state<Active>
            ,state<Active>   + event<CmdStop>       / &Self::onCmdStop               = state<Ready>
            ,state<Ready>    + event<CmdTeardown>   / &Self::onCmdTeardown           = state<Idle>
            ,state<Active>   + event<EvtTimerPoll>  / &Self::onEvtTimerPoll
            ,state<_>        + event<SetDistCm>     / &Self::onSetDist
            ,state<_>        + event<SetSampleInt>  / &Self::onSetSampleInt
            ,state<Error>    + on_entry<_>          / &Self::onError
            // clang-format on
        );
    }

  private:
    // Guards
    bool isReadyGuard(const CmdSetup &e);

    // Actions
    void onCmdStart(const CmdStart &);
    void onCmdStop(const CmdStop &);
    void onCmdTeardown(const CmdTeardown &);
    void onEvtTimerPoll(const EvtTimerPoll &);
    void onSetDist(const SetDistCm &);
    void onSetSampleInt(const SetSampleInt &);
    void onError();

    static constexpr char TAG[] = "radar";

    ctrl::Q &ctrlQ_;

    bool stubHardwareInit();

    uint16_t detDistCm_{0};
    uint32_t sampleIntMs_{0};

    // State tags (no data)
    struct Idle {};
    struct Ready {};
    struct Active {};
    struct Error {};
};

} // namespace esphome::smart_signage::radar
