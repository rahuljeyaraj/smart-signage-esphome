#pragma once
#include <etl/variant.h>
#include "sml.hpp"
#include "log.h"

#include "ctrl/ctrl_q.h"       // talk back to the controller
#include "audio/audio_event.h" // CmdSetup, CmdPlay, …

namespace sml = boost::sml;

namespace esphome::smart_signage::audio {

/*──────────────────────────  AUDIO FSM  ──────────────────────────*/
class FSM {
    using Self = FSM;

  public:
    explicit FSM(ctrl::Q &q); // controller queue for ready / error events

    /*───────── Top-level SM ─────────*/
    auto operator()() noexcept {
        using namespace sml;
        return make_transition_table(
            // clang-format off
            *state<Idle>   + event<CmdSetup>    [ &Self::isReadyGuard ]     = state<Ready>,
             state<Idle>   + event<CmdSetup>                                = state<Error>,

             state<Ready>  + event<CmdPlay>     / &Self::onCmdPlay          = state<Playing>,
             state<Ready>  + event<CmdTeardown> / &Self::onCmdTeardown      = state<Idle>,

             state<Playing>+ event<CmdStop>     / &Self::onCmdStop          = state<Ready>,
             state<Playing>+ event<CmdTeardown> / &Self::onCmdTeardown      = state<Idle>,

             state<Error>  + sml::on_entry<_>   / &Self::onError
            // clang-format on
        );
    }

  private:
    /*───────── Guards ─────────*/
    bool isReadyGuard(const CmdSetup &);

    /*───────── Actions ─────────*/
    void onCmdTeardown(const CmdTeardown &);

    void onCmdPlay(const CmdPlay &);
    void onCmdStop(const CmdStop &);

    /*───────── Error handler ─────────*/
    void onError();

    /*───────── Helpers ─────────*/
    bool stubHardwareInit(); // returns true once I²S / DAC etc. are up

    static constexpr char TAG[] = "audio_fsm";
    ctrl::Q              &ctrlQ_;

    /*───────── State tags (no data) ─────────*/
    struct Idle {};
    struct Ready {};
    struct Playing {};
    struct Error {};
};

} // namespace esphome::smart_signage::audio
