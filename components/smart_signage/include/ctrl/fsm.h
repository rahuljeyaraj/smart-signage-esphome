#pragma once
#include "ctrl/event.h"
#include "log.h"
#include "radar/q.h"
#include "sml.hpp"
#include <etl/variant.h>

namespace esphome::smart_signage::ctrl {

class FSM {
    using Self = FSM;

  public:
    explicit FSM(radar::Q &radarQ) : radarQ_(radarQ) {}

    auto operator()() noexcept {
        using namespace boost::sml;
        return make_transition_table(
            *state<Idle> + event<Setup> / &Self::onSetup                  = state<ReadyWait>,
            state<ReadyWait> + event<radar::SetupDone>[&Self::SetupGuard] = state<Ready>,
            state<Ready> + event<Start> / &Self::onStart                  = state<Active>,
            state<Active> + event<Timeout> / &Self::onRunTimeout          = state<Idle>,
            state<Active> + event<radar::Data> / &Self::onRadarData       = state<Active>,
            // state<Active> + event<imu::Fell> / &Self::onFell           = state<Fallen>,
            // state<Fallen> + event<imu::Rose> / &Self::onRose           = state<Active>,
            state<_> + event<radar::InitError> = state<Error>);
    }

    // Guards
    bool SetupGuard(const radar::SetupDone &e) {
        // readySeen_.insert(e);
        // etl::visit([this](auto const &ev) { readySeen_.insert(ev); }, e);
        // return readySeen_.size() == kIntfCnt;
        return true;
    }

    // Actions
    void onSetup(const Setup &) {
        radarQ_.post(radar::Setup{});
        LOGI(TAG, "onSetup");
    }

    void onStart(const Start &e) {
        radarQ_.post(radar::Start{});

        runTimeMins_ = e.runTimeMins;
        LOGI(TAG, "onStart: runTimeMins=%u", runTimeMins_);
        // schedule Timeout via your automation API...
    }

    // void onFell(const Fell &) { LOGI(TAG, "onFell: fall detected"); }
    // void onRose(const Rose &) { LOGI(TAG, "onRose: rise detected"); }

    void onRunTimeout(const Timeout &) {
        radarQ_.post(radar::Stop{});
        radarQ_.post(radar::Teardown{});

        LOGI(TAG, "onRunTimeout: tearing down");
        teardownAll();
    }

    void onError(const InitError &) { LOGE(TAG, "InitError!"); }

    void onRadarData(const radar::Data &e) {
        LOGI(TAG, "onRadarData: detected=%s dist=%u cm ts=%u", e.detected ? "Y" : "N",
             static_cast<unsigned>(e.distanceCm), static_cast<unsigned>(e.timestampTicks));
    }

  private:
    static constexpr char TAG[] = "ctrl";

    radar::Q &radarQ_;
    uint32_t  runTimeMins_{0};

    // ––– Teardown helper –––––––––––––––––––––––––––––––
    void teardownAll() {
        LOGI(TAG, "teardownAll: cleaning up all modules");
        // e.g. static_cast<RadarAO*>(aos_[IDX_RADAR])->post(Teardown{});
        // …and so on for each AO…
    }

    // ––– State tags –––––––––––––––––––––––––––––––––––––
    struct Idle {};
    struct ReadyWait {};
    struct Ready {};
    struct Active {};
    struct Fallen {};
    struct Error {};
};

} // namespace esphome::smart_signage::ctrl
