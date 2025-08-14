#pragma once
#include "sml.hpp"
#include "log.h"
#include "ctrl/ctrl_q.h"
#include "led/led_event.h"
#include "led/hal/iled_hal.h"
#include "timer/itimer.h"

namespace sml = boost::sml;

namespace esphome::smart_signage::led {

class FSM {
    using Self = FSM;

  public:
    FSM(ctrl::Q &ctrlQ, hal::ILedHal &hal, timer::ITimer &timer);

  private:
    struct Idle {};
    struct Error {};
    struct Off {};
    struct On {};
    struct BreatheUp {};
    struct HoldHigh {};
    struct BreatheDown {};
    struct HoldLow {};

    struct BreathCfg {
        uint16_t toHighMs{0};
        uint16_t holdHighMs{0};
        uint16_t toLowMs{0};
        uint16_t holdLowMs{0};
        uint16_t cntLeft{0}; // decremented at end of HoldLow
        bool     finite{false};
    };

    bool tryInitHal(const CmdSetup &);
    bool breathUpGuard(const EvtTimerEnd &) const;

    void onSetupOk(const CmdSetup &);
    void onSetupFail(const CmdSetup &);
    void onTeardown(const CmdTeardown &);
    void onError();

    void onCmdOn(const CmdOn &);
    void onEnterOff();

    void setBreatheParams(const CmdBreathe &);

    void onBreatheUpEntry();
    void onHoldHighEntry();
    void onBreatheDownEntry();
    void onHoldLowEntry();

    void decCycleAfterHoldLow(const EvtTimerEnd &);
    void finishBreathe(const EvtTimerEnd &);
    void onSetBrightness(const SetBrightness &);

    ctrl::Q       &ctrlQ_;
    hal::ILedHal  &hal_;
    timer::ITimer &timer_;
    BreathCfg      breath_;
    uint8_t        brightnessPct_{kDefaultBrightPct};

    /**
     * Breath Waveform:
     *     highLevel   ____________
     *                /            \
     *               /              \                  (repeat)
     *              /                \               /
     *   lowLevel _/                  \_____________/
     *            ^   ^            ^   ^           ^
     *            |   |            |   |           |
     *            |   | holdHighMs |   | holdLowMs |
     *          toHighMs          toLowMs
     */
    struct Ready {
        auto operator()() const noexcept {
            using namespace sml;
            return make_transition_table(*state<Off> + sml::on_entry<_> / &Self::onEnterOff,
                // clang-format off
                *state<Off>         + event<CmdBreathe>     / &Self::setBreatheParams   = state<BreatheUp>
                ,state<On>          + event<CmdBreathe>     / &Self::setBreatheParams   = state<BreatheUp>
                ,state<_>           + event<CmdBreathe>     / &Self::setBreatheParams
                ,state<BreatheUp>   + event<EvtFadeEnd>                                 = state<HoldHigh>
                ,state<HoldHigh>    + event<EvtTimerEnd>                                = state<BreatheDown>
                ,state<BreatheDown> + event<EvtFadeEnd>                                 = state<HoldLow>
                ,state<HoldLow>     + event<EvtTimerEnd>    [ &Self::breathUpGuard ]    = state<BreatheUp>
                ,state<HoldLow>     + event<EvtTimerEnd>    / &Self::finishBreathe      = state<Off>

                ,state<_>           + event<CmdOff>                                     = state<Off>
                ,state<_>           + event<CmdOn>          / &Self::onCmdOn            = state<On>

                ,state<BreatheUp>   + sml::on_entry<_>      / &Self::onBreatheUpEntry
                ,state<HoldHigh>    + sml::on_entry<_>      / &Self::onHoldHighEntry
                ,state<BreatheDown> + sml::on_entry<_>      / &Self::onBreatheDownEntry
                ,state<HoldLow>     + sml::on_entry<_>      / &Self::onHoldLowEntry

                // clang-format on
            );
        }
    };

  public:
    auto operator()() noexcept {
        using namespace sml;
        return make_transition_table(
            // clang-format off
            *state<Idle>    + event<CmdSetup> [ &Self::tryInitHal ] / &Self::onSetupOk          = state<Ready>
            ,state<Idle>    + event<CmdSetup>                       / &Self::onSetupFail        = state<Error>
            ,state<Ready>   + event<CmdTeardown>                    / &Self::onTeardown         = state<Idle>
            ,state<_>       + event<SetBrightness>                  / &Self::onSetBrightness    = state<On>
            ,state<Error>   + sml::on_entry<_>                      / &Self::onError
            // clang-format on
        );
    }

    static constexpr char TAG[] = "LedFsm";
};

} // namespace esphome::smart_signage::led
