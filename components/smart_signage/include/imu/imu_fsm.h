#pragma once
#include <etl/variant.h>
#include <etl/debounce.h>
#include "sml.hpp"
#include "log.h"
#include "ctrl/ctrl_q.h"
#include "imu/imu_event.h"
#include "imu/hal/iimu_hal.h"
#include "timer/itimer.h"

namespace sml = boost::sml;

namespace esphome::smart_signage::imu {

using Vector = Eigen::Vector3f;

class FSM {
    using Self = FSM;

  public:
    explicit FSM(ctrl::Q &q, hal::IImuHal &hal, timer::ITimer &timer);

    struct Active {
        auto operator()() const noexcept {
            using namespace sml;
            return make_transition_table(
                // clang-format off
                *state<Standing> + event<EvtTimerPoll> [  wrap(&Self::isFallenGuard) ]  = state<Fallen>
                ,state<Fallen>   + event<EvtTimerPoll> [ !wrap(&Self::isFallenGuard) ]  = state<Standing>
                ,state<Fallen>   + sml::on_entry<_>    / &Self::onFallenEntry
                ,state<Fallen>   + sml::on_exit<_>     / &Self::onFallenExit
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
            ,state<_>        + event<SetFallAngle>  / &Self::onSetFallAngle
            ,state<_>        + event<SetConfirmCnt> / &Self::onSetConfirmCnt
            ,state<_>        + event<SetSampleInt>  / &Self::onSetSampleInt
            ,state<Error>    + sml::on_entry<_>     / &Self::onError
            // clang-format on
        );
    }

  private:
    // Guards
    bool isReadyGuard(const CmdSetup &);
    bool isFallenGuard(const EvtTimerPoll &);

    // Actions
    void onCmdStart(const CmdStart &);
    void onCmdStop(const CmdStop &);
    void onCmdTeardown(const CmdTeardown &);
    void onSetFallAngle(const SetFallAngle &);
    void onSetConfirmCnt(const SetConfirmCnt &);
    void onSetSampleInt(const SetSampleInt &);

    void onFallenEntry();
    void onFallenExit();
    void onError();

    // Helper
    uint16_t computeTiltAngle(const Vector &curAccel, const Vector &refAccel) const;

    ctrl::Q       &ctrlQ_;
    hal::IImuHal  &hal_;
    timer::ITimer &timer_;

    Vector                 refAcc_{Vector::Zero()};
    etl::debounce<0, 0, 0> isFallenDebounce_;

    bool stubHardwareInit();

    uint16_t fallAngleDeg_{kDefaultFallAngleDeg};
    uint32_t confirmCnt_{kDefaultConfirmCount};
    uint32_t sampleIntMs_{kDefaultSampleIntMs};

    // State tags (no data)
    struct Idle {};
    struct Ready {};
    struct Error {};
    // struct Active{}; is a sub SM
    struct Standing {};
    struct Fallen {};

    static constexpr char TAG[] = "ImuFsm";
};

} // namespace esphome::smart_signage::imu
