#pragma once
#include <etl/variant.h>
#include <SimpleKalmanFilter.h>
#include "sml.hpp"
#include "ctrl/ctrl_q.h"
#include "radar/hal/iradar_hal.h"
#include "radar/radar_event.h"
#include "timer/itimer.h"

namespace esphome::smart_signage::radar {

class FSM {
    using Self = FSM;

  public:
    // Now take IRadarHal& so we can call into the real driver
    explicit FSM(ctrl::Q &q, hal::IRadarHal &hal, timer::ITimer &t, SimpleKalmanFilter &f);

    auto operator()() noexcept {
        using namespace boost::sml;
        return make_transition_table(
            // clang-format off
            *state<Idle>     + event<CmdSetup>      [ &Self::isReadyGuard ]     = state<Ready>
            ,state<Idle>     + event<CmdSetup>                                  = state<Error>
            ,state<Ready>    + event<CmdStart>      / &Self::onCmdStart         = state<Active>
            ,state<Active>   + event<CmdStop>       / &Self::onCmdStop          = state<Ready>
            ,state<Ready>    + event<CmdTeardown>   / &Self::onCmdTeardown      = state<Idle>
            ,state<Active>   + event<EvtTimerPoll>  / &Self::onEvtTimerPoll
            ,state<_>        + event<SetRangeCm>    / &Self::onSetDist
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
    void onSetDist(const SetRangeCm &);
    void onSetSampleInt(const SetSampleInt &);
    void onError();

    ctrl::Q            &ctrlQ_;
    hal::IRadarHal     &hal_;
    timer::ITimer      &timer_;
    SimpleKalmanFilter &filter_;

    uint16_t detDistCm_{kDefaultRangeCm};
    uint32_t sampleIntMs_{kDefaultSampleIntMs};

    // State tags
    struct Idle {};
    struct Ready {};
    struct Active {};
    struct Error {};

    static constexpr char TAG[] = "RadarFSM";
};

} // namespace esphome::smart_signage::radar
