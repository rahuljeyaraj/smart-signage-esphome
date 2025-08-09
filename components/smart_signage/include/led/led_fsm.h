#pragma once
#include "sml.hpp"
#include "log.h"
#include "ctrl/ctrl_q.h"
#include "led/led_event.h" // CmdSetup, CmdTeardown, CmdOn, CmdOff, CmdBreathe, EvtFadeEnd
#include "led/hal/iled_hal.h"
#include "timer/itimer.h"

namespace sml = boost::sml;

namespace esphome::smart_signage::led {

class FSM {
    using Self = FSM;

  public:
    FSM(ctrl::Q &ctrlQ, hal::ILedHal &hal, timer::ITimer &timer);

    /*──────── Ready: Off | On | Breath(sub-SM) ─────*/

    struct Breathe {
        auto operator()() const noexcept {
            using namespace sml;
            return make_transition_table(
                // clang-format on
                *state<BreatheUp> + sml::on_entry<_> / &Self::onBreatheUpEntry,
                state<BreatheDown> + sml::on_entry<_> / &Self::onBreatheDownEntry,
                state<BreatheUp> + event<EvtFadeEnd>                          = state<BreatheDown>,
                state<BreatheDown> + event<EvtFadeEnd>[&Self::breatheUpGuard] = state<BreatheUp>,
                state<BreatheDown> + event<EvtFadeEnd>                        = state<Off>);
            // clang-format on
        }
    };

    struct Ready {
        auto operator()() const noexcept {
            using namespace sml;
            return make_transition_table(
                // clang-format on
                *state<Off> + sml::on_entry<_> / &Self::onEnterOff,
                state<_> + event<CmdOff>                                = state<Off>,
                state<_> + event<CmdOn> / &Self::onCmdOn                = state<On>,
                state<Off> + event<CmdBreathe> / &Self::setBreathParams = state<Breathe>,
                state<On> + event<CmdBreathe> / &Self::setBreathParams  = state<Breathe>,
                state<Breathe> + event<CmdBreathe> / &Self::setBreathParams,
                state<Breathe> + sml::on_exit<_> / &Self::onBreatheExit
                // clang-format on
            );
        }
    };

    auto operator()() noexcept {
        using namespace sml;
        return make_transition_table(
            *state<Idle> + event<CmdSetup>[&Self::hwInitGuard]    = state<Ready>,
            state<Idle> + event<CmdSetup>                         = state<Error>,
            state<Ready> + event<CmdTeardown> / &Self::onTeardown = state<Idle>,
            state<Error> + sml::on_entry<_> / &Self::onError);
    }

  private:
    /*── Tags ─*/
    struct Idle {};
    struct Error {};
    struct Off {};
    struct On {};
    struct Breath {};
    struct BreatheUp {};
    struct BreatheDown {};
    // (Future: struct StayUp {}; struct StayDown {};)

    /*── Breath runtime config ─*/
    struct BreathCfg {
        uint8_t  brightPct{100};
        uint16_t fadeInMs{0};
        uint16_t fadeOutMs{0};
        uint16_t cntLeft{0};
        bool     finite{false};
    };

    /*── Guards (decl only) ─*/
    bool hwInitGuard(const CmdSetup &);
    bool breatheUpGuard(const EvtFadeEnd &);

    /*── Actions (decl only) ─*/
    void onTeardown(const CmdTeardown &);
    void onError();

    void onCmdOn(const CmdOn &cmd);

    void onEnterOff();

    void setBreathParams(const CmdBreathe &cmd);

    // void finishBreathe(const EvtFadeEnd &); // Last Down finished → Off + notify

    // Entry hooks start fades
    void onBreatheUpEntry();
    void onBreatheDownEntry();
    void onBreatheExit();

    /*── Deps & data ─*/
    ctrl::Q       &ctrlQ_;
    hal::ILedHal  &hal_;
    timer::ITimer &timer_;
    BreathCfg      breathCfg_;

    static constexpr char TAG[] = "LedFsm";
};

} // namespace esphome::smart_signage::led
