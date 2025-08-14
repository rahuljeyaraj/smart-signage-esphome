#pragma once

#include <etl/variant.h>
#include <etl/bitset.h>
#include <type_traits>
#include "sml.hpp"
#include "log.h"

#include "ctrl/ctrl_event.h"
#include "radar/radar_q.h"
#include "imu/imu_q.h"
#include "led/led_q.h"
#include "audio/audio_q.h"
#include "timer/itimer.h"
#include "profile_settings.h"
#include "user_intf.h"
#include "common.h"

namespace radar = esphome::smart_signage::radar;
namespace imu   = esphome::smart_signage::imu;
namespace led   = esphome::smart_signage::led;

namespace esphome::smart_signage::ctrl {

class FSM {
    using Self = FSM;

  public:
    explicit FSM(radar::Q &radarQ, imu::Q &imuQ, led::Q &ledQ, audio::Q &audioQ,
        timer::ITimer &timer, ProfileSettings &profiles, UserIntf &ui);

    /*──────────────────────── State machine ────────────────────────*/
    auto operator()() noexcept {
        using namespace boost::sml;
        return make_transition_table(
            // clang-format off
            *state<Idle>     + event<CmdSetup>          / &Self::onCmdSetup         = state<Setup>

            // // For future, if any interface hangs on setup command we need a timeout (currently not used)
            // ,state<Setup>    + event<EvtTimerEnd>   / &Self::onSetupTimeout     = state<Error>

            // // TODO: Try to make it with just one guard function
            // ,state<Setup>    + event<EvtRadarReady>     [ &Self::guardRadarReady ]  = state<Ready>
            // ,state<Setup>    + event<EvtImuReady>       [ &Self::guardImuReady   ]  = state<Ready>
            // ,state<Setup>    + event<EvtLedReady>       [ &Self::guardLedReady   ]  = state<Ready>
            // ,state<Setup>    + event<EvtAudioReady>     [ &Self::guardAudioReady ]  = state<Ready>

            // ,state<Ready>    + event<CmdStart>          / &Self::onCmdStart         = state<Active>
            // ,state<Ready>    + event<CmdTeardown>       / &Self::onCmdTeardown      = state<Idle>

            // ,state<Active>   + event<EvtRadarData>      / &Self::onEvtRadarData
            // ,state<Active>   + event<CmdStop>           / &Self::onCmdStop          = state<Ready>
            // ,state<Active>   + event<EvtTimerEnd>       / &Self::onSessionEnd       = state<Idle>
            // ,state<Active>   + event<EvtImuFell>        / &Self::onEvtImuFell       = state<Fallen>

            // ,state<Fallen>   + event<EvtImuRose>        / &Self::onEvtImuRose       = state<Active>

            // ,state<_>        + event<EvtUiProfileUpdate>      / &Self::onUiProfileUpdate
            // ,state<_>        + event<EvtUiSessionMinsUpdate>  / &Self::onUiSessionMinsUpdate
            // ,state<_>        + event<EvtUiRangeCmUpdate>      / &Self::onUiRangeCmUpdate
            // ,state<_>        + event<EvtUiAudioVolUpdate>     / &Self::onUiAudioVolUpdate
            // ,state<_>        + event<EvtUiLedBrightUpdate>    / &Self::onUiLedBrightUpdate

            // ,state<_>        + event<EvtRadarError>                                 = state<Error>
            // ,state<_>        + event<EvtImuError>                                   = state<Error>
            // ,state<_>        + event<EvtLedError>                                   = state<Error>
            // ,state<_>        + event<EvtAudioError>                                 = state<Error>

            // ,state<Error>    + on_entry<_>              / &Self::onError
            // // clang-format on
        );
    }

  private:
    /*──────────── Interface bookkeeping ───────────────────────────*/
    enum class Intf : uint8_t { Radar, Imu, Led, Audio };
    static constexpr size_t kIntfCnt = 4;

    /*──────────── Guards ──────────────────────────────────────────*/
    bool guardRadarReady(const EvtRadarReady &);
    bool guardImuReady(const EvtImuReady &);
    bool guardLedReady(const EvtLedReady &);
    bool guardAudioReady(const EvtAudioReady &);

    /*──────────── Actions ─────────────────────────────────────────*/
    void onCmdSetup(const CmdSetup &);
    void onCmdStart(const CmdStart &);
    void onCmdStop(const CmdStop &);
    void onCmdTeardown(const CmdTeardown &);
    void onEvtRadarData(const EvtRadarData &);
    void onEvtImuFell(const EvtImuFell &);
    void onEvtImuRose(const EvtImuRose &);
    void onSetupTimeout();
    void onSessionEnd();
    void onUiProfileUpdate(const EvtUiProfileUpdate &);
    void onUiSessionMinsUpdate(const EvtUiSessionMinsUpdate &);
    void onUiRangeCmUpdate(const EvtUiRangeCmUpdate &);
    void onUiAudioVolUpdate(const EvtUiAudioVolUpdate &);
    void onUiLedBrightUpdate(const EvtUiLedBrightUpdate &);
    void onError();

    /*──────────── Helper ─────────────────────────────────────────*/
    ProfileName resolveActiveProfile_();

    static constexpr char TAG[] = "ctrlFSM";

    /*──────────── Resources ────────────────────────────────────────────*/
    radar::Q        &radarQ_;
    imu::Q          &imuQ_;
    led::Q          &ledQ_;
    audio::Q        &audioQ_;
    timer::ITimer   &timer_;
    ProfileSettings &profiles_;
    UserIntf        &ui_;

    /*──────────── Data ────────────────────────────────────────────*/
    uint32_t              sessionMins_{0};
    etl::bitset<kIntfCnt> readyBits_{};

    /*──────────── State tags (no data) ────────────────────────────*/
    struct Idle {};
    struct Setup {};
    struct Ready {};
    struct Active {};
    struct Fallen {};
    struct Error {};
};

} // namespace esphome::smart_signage::ctrl
