#pragma once

#include <etl/variant.h>
#include <etl/bitset.h> // NEW – readiness tracking
#include <type_traits>  // NEW – for compile-time dispatch
#include "sml.hpp"
#include "log.h"

#include "ctrl/ctrl_event.h"
#include "radar/radar_q.h"
#include "imu/imu_q.h"
#include "led/led_q.h"
#include "audio/audio_q.h"

namespace radar = esphome::smart_signage::radar;
namespace imu   = esphome::smart_signage::imu;
namespace led   = esphome::smart_signage::led;

namespace esphome::smart_signage::ctrl {

class FSM {
    using Self = FSM;

  public:
    explicit FSM(radar::Q &radarQ, imu::Q &imuQ, led::Q &ledQ, audio::Q &audioQ);

    /*──────────────────────── State machine ────────────────────────*/
    auto operator()() noexcept {
        using namespace boost::sml;
        return make_transition_table(
            // clang-format off
            *state<Idle>     + event<CmdSetup>      / &Self::onCmdSetup           = state<Setup>

            ,state<Setup>    + event<EvtTimeout>    / &Self::onSetupTimeout       = state<Error>
            ,state<Setup>    + event<EvtRadarReady> [ &Self::guardRadarReady ]    = state<Ready>
            ,state<Setup>    + event<EvtImuReady>   [ &Self::guardImuReady   ]    = state<Ready>
            ,state<Setup>    + event<EvtLedReady>   [ &Self::guardLedReady   ]    = state<Ready>
            ,state<Setup>    + event<EvtAudioReady> [ &Self::guardAudioReady ]    = state<Ready>

            ,state<Ready>    + event<CmdStart>      / &Self::onCmdStart           = state<Active>
            ,state<Ready>    + event<CmdTeardown>   / &Self::onCmdTeardown        = state<Idle>

            ,state<Active>   + event<CmdStop>       / &Self::onCmdStop            = state<Ready>
            ,state<Active>   + event<EvtTimeout>    / &Self::onActiveTimeout      = state<Idle>
            ,state<Active>   + event<EvtRadarData>  / &Self::onEvtRadarData       = state<Active>
            ,state<Active>   + event<EvtImuFell>    / &Self::onEvtImuFell         = state<Fallen>

            ,state<Fallen>   + event<EvtImuRose>    / &Self::onEvtImuRose         = state<Active>

            ,state<_>        + event<EvtRadarError>                               = state<Error>
            ,state<_>        + event<EvtImuError>                                 = state<Error>
            ,state<_>        + event<EvtLedError>                                 = state<Error>
            ,state<_>        + event<EvtAudioError>                               = state<Error>

            ,state<Error>    + on_entry<_>          / &Self::onError
            // clang-format on
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
    void onSetupTimeout(const EvtTimeout &);
    void onActiveTimeout(const EvtTimeout &);
    void onError();

    static constexpr char TAG[] = "ctrlFSM";

    /*──────────── Data ────────────────────────────────────────────*/
    radar::Q             &radarQ_;
    imu::Q               &imuQ_;
    led::Q               &ledQ_;
    audio::Q             &audioQ_;
    uint32_t              runTimeMins_{0};
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
