#pragma once
#include <etl/variant.h>
#include <SimpleKalmanFilter.h>
#include "sml.hpp"
#include "ctrl/ctrl_q.h"
#include "radar/hal/iradar_hal.h"
#include "radar/radar_event.h"
#include "timer/itimer.h"
namespace sml = boost::sml;
namespace esphome::smart_signage::radar {

class FSM {
    using Self = FSM;

  public:
    // Now take IRadarHal& so we can call into the real driver
    explicit FSM(ctrl::Q &q, hal::IRadarHal &hal, timer::ITimer &t);

    struct Active {
        auto operator()() const noexcept {
            using namespace sml;
            return make_transition_table(
                // clang-format off
                *state<Clear>     + event<EvtTimerPoll> [ wrap(&Self::hasDataGuard) && wrap(&Self::detectGuard) ]  = state<Detected>
                ,state<Detected>  + event<EvtTimerPoll> [ wrap(&Self::hasDataGuard) && !wrap(&Self::detectGuard) ]  = state<Clear>
                ,state<Detected>  + sml::on_entry<_>    / &Self::onDetectedEntry
                ,state<Detected>  + sml::on_exit<_>     / &Self::onDetectedExit
                // clang-format on
            );
        }
    };

    auto operator()() noexcept {
        using namespace sml;
        return make_transition_table(
            // clang-format off
            *state<Idle>     + event<CmdSetup>      [ &Self::isReadyGuard ]     = state<Ready>
            ,state<Idle>     + event<CmdSetup>                                  = state<Error>
            ,state<Ready>    + event<CmdStart>      / &Self::onCmdStart         = state<Active>
            ,state<Active>   + event<CmdStop>       / &Self::onCmdStop          = state<Ready>
            ,state<Ready>    + event<CmdTeardown>   / &Self::onCmdTeardown      = state<Idle>
            ,state<_>        + event<SetRangeCm>    / &Self::onSetDist
            ,state<_>        + event<SetSampleInt>  / &Self::onSetSampleInt
            ,state<Error>    + sml::on_entry<_>          / &Self::onError
            // clang-format on
        );
    }

  private:
    // Guards
    bool isReadyGuard(const CmdSetup &e);
    bool hasDataGuard();
    bool detectGuard();

    // Actions
    void onCmdStart(const CmdStart &);
    void onCmdStop(const CmdStop &);
    void onCmdTeardown(const CmdTeardown &);
    void onSetDist(const SetRangeCm &);
    void onSetSampleInt(const SetSampleInt &);

    void onDetectedEntry();
    void onDetectedExit();
    void onError();

    ctrl::Q           &ctrlQ_;
    hal::IRadarHal    &hal_;
    timer::ITimer     &timer_;
    SimpleKalmanFilter filter_{kMeasurementNoise, kInitialError, kProcessNoise};

    uint16_t detDistCm_{kDefaultRangeCm};
    uint32_t sampleIntMs_{kDefaultSampleIntMs};

    bool     hasRawData_    = false;
    bool     rawDetected_   = false;
    uint16_t rawDistanceCm_ = 0;

    // State tags
    struct Idle {};
    struct Ready {};
    struct Error {};
    // struct Active{}; is a sub SM
    struct Detected {};
    struct Clear {};

    static constexpr char TAG[] = "RadarFSM";
};

} // namespace esphome::smart_signage::radar
