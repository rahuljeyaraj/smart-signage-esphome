#pragma once
#include <etl/variant.h>
#include "sml.hpp"
#include "log.h"
#include "ctrl/ctrl_q.h"
#include "led/led_event.h"

namespace sml = boost::sml;

namespace esphome::smart_signage::led {

class FSM {
    using Self = FSM;

  public:
    explicit FSM(ctrl::Q &q);

    struct Breathing {
        auto operator()() const noexcept {
            using namespace sml;
            return make_transition_table(
                // clang-format off
                *state<Low> + event<CmdFadeIn> / &Self::onCmdFadeIn
                ,state<Low> + event<EvtFadeInEnd> / &Self::onFadeInEnd = state<High>
                ,state<High>+ event<CmdFadeOut>/ &Self::onCmdFadeOut
                ,state<High> + event<EvtFadeOutEnd> / &Self::onFadeOutEnd = state<Low>
                // clang-format on
            );
        }
    };

    struct Ready {
        auto operator()() const noexcept {
            using namespace sml;
            return make_transition_table(
                // clang-format off
                *state<Off> + event<CmdOn> / &Self::onCmdOn = state<On>
                ,state<Off> + event<CmdBreathe> / &Self::onCmdBreathe = state<Breathing>
                ,state<_> + event<CmdOff> / &Self::onCmdOn = state<Off>
                // clang-format on
            );
        }
    };

    auto operator()() noexcept {
        using namespace sml;
        return make_transition_table(
            // clang-format off
            *state<Idle>     + event<CmdSetup>      [ &Self::isReadyGuard ]          = state<Ready>
            ,state<Idle>     + event<CmdSetup>                                       = state<Error>
            ,state<Ready>    + event<CmdTeardown>   / &Self::onCmdTeardown           = state<Idle>



            ,state<Ready>    + event<on>      / &Self::onCmdOn              = state<on>
            ,state<Active>   + event<CmdStop>       / &Self::onCmdOff               = state<Ready>
            
            ,state<_>        + event<SetFallAngle>  / &Self::onSetFallAngle
            ,state<_>        + event<SetConfirmCnt> / &Self::onSetConfirmCnt
            ,state<_>        + event<SetSampleInt>  / &Self::onSetSampleInt
            ,state<Error>    + sml::on_entry<_>          / &Self::onError
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

    static constexpr char TAG[] = "led";

    ctrl::Q &ctrlQ_;

    bool stubHardwareInit();

    uint16_t fallAngleDeg_{kDefaultFallAngleDeg};
    uint32_t confirmCnt_{kDefaultConfirmCount};
    uint32_t sampleIntMs_{kDefaultSampleIntMs};

    // State tags (no data)
    struct Idle {};
    struct Error {};
    // struct Ready{}; is a sub SM
    struct Off {};
    struct On {};
    // struct Breathing {}; is a sub SM
    struct Low {};
    struct High {};
};

} // namespace esphome::smart_signage::led
