#pragma once
#include <etl/variant.h>
#include "sml.hpp"
#include "log.h"
#include "ctrl/ctrl_q.h"
#include "led/led_event.h"
#include "led/hal/iled_hal.h"
#include "timer/itimer.h"

namespace sml = boost::sml;

namespace esphome::smart_signage::led {

/*──────────────────────────  LED FSM  ──────────────────────────*/
class FSM {
    using Self = FSM;

  public:
    explicit FSM(ctrl::Q &q, hal::ILedHal &hal, timer::ITimer &t);

    /*───────── Sub-SM: Breathing (Low ↔ High) ─────────*/
    struct Breathing {
        auto operator()() const noexcept {
            using namespace sml;
            return make_transition_table(
                // clang-format off
                *state<Low>  + event<EvtFadeEnd>  / &Self::onFadeInEnd  = state<High>,
                 state<High> + event<EvtFadeEnd> / &Self::onFadeOutEnd = state<Low>
                // clang-format on
            );
        }
    };

    /*───────── Sub-SM: Ready modes ─────────*/
    struct Ready {
        auto operator()() const noexcept {
            using namespace sml;
            return make_transition_table(
                // clang-format off
                *state<Off> + event<CmdOn>      / &Self::onCmdOn      = state<Solid>,
                state<Off> + event<CmdBreathe> / &Self::onCmdBreathe = state<Breathing>,

                // wildcard handles everything else, but avoids exit/entry when redundant
                state<_>   + event<CmdOn>      / &Self::onCmdOn      = state<Solid>,
                state<_>   + event<CmdBreathe> / &Self::onCmdBreathe = state<Breathing>,
                state<_>   + event<CmdOff>     / &Self::onCmdOff     = state<Off>
                // clang-format on
            );
        }
    };

    /*───────── Top-level SM ─────────*/
    auto operator()() noexcept {
        using namespace sml;
        return make_transition_table(
            // clang-format off
            *state<Idle>  + event<CmdSetup>     [ &Self::isReadyGuard ]  = state<Ready>,
             state<Idle>  + event<CmdSetup>                              = state<Error>,
             state<Ready> + event<CmdTeardown>  / &Self::onCmdTeardown   = state<Idle>,
             state<Error> + sml::on_entry<_>    / &Self::onError
            // clang-format on
        );
    }

  private:
    /*───────── Guards ─────────*/
    bool isReadyGuard(const CmdSetup &);

    /*───────── Actions ─────────*/
    void onCmdSetup(const CmdSetup &);
    void onCmdTeardown(const CmdTeardown &);

    void onCmdOn(const CmdOn &);
    void onCmdOff(const CmdOff &);
    void onCmdBreathe(const CmdBreathe &);

    void onFadeInEnd(const EvtFadeEnd &);
    void onFadeOutEnd(const EvtFadeEnd &);

    /*───────── Error handler ─────────*/
    void onError();

    /*───────── Helpers / data ─────────*/
    bool stubHardwareInit(); // returns true when LED HW ready

    ctrl::Q       &ctrlQ_;
    hal::ILedHal  &hal_;
    timer::ITimer &timer_;

    /*───────── State tags (no data) ─────────*/
    struct Idle {};
    struct Error {};
    // struct Ready {}; Sub states:
    struct Off {};
    struct Solid {};
    // struct Breathing {}; Sub states:
    struct Low {};
    struct High {};

    static constexpr char TAG[] = "LedFSm";
};

/*───────────────────────────────────────────────────────────────*/
} // namespace esphome::smart_signage::led
