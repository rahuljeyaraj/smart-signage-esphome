#pragma once
#include "active_object.h"
#include "log.h"
#include "radar_fsm.h"
#include "sml.hpp"
#include <etl/flat_map.h>
#include <etl/flat_set.h>
#include <etl/variant.h>

namespace sml = boost::sml;

namespace esphome::smart_signage::ctrl {

enum class IntfId : uint8_t { Radar = 0, /*Imu, Led, Audio,*/ Count };
static constexpr size_t kIntfCnt = static_cast<size_t>(IntfId::Count);
using AoPtrVariant = etl::variant<RadarAO * /*, ImuAO *, LedAO *, AudioAO **/>;
using AoMap = etl::flat_map<IntfId, AoPtrVariant, kIntfCnt>;

// class ImuAO;
// class LedAO;
// class AudioAO;

// Events
struct Setup {
    RadarAO *radarAo;
    /* LedAO *ledAo;
    AudioAO *audioAo;
    ImuAO *imuAo;*/
};

struct Start {
    uint32_t runTimeMins; // Run duration
};
struct Timeout {};
struct Fell {};
struct Rose {};

struct ImuReady {};
struct LedReady {};
struct AudioReady {};

struct ImuError {};
struct LedError {};
struct AudioError {};

using ReadyEvent = etl::variant<RadarReady, ImuReady, LedReady, AudioReady>;
using ErrorEvent = etl::variant<RadarError, ImuError, LedError, AudioError>;
using CtrlEvent =
    etl::variant<Setup, Start, ReadyEvent, ErrorEvent, RadarData, Fell, Rose, Timeout>;

// CtrlFSM
class CtrlFSM {
    using Self = CtrlFSM;

  public:
    CtrlFSM() = default;

    auto operator()() noexcept {
        using namespace boost::sml;
        auto readyGuard = guard<&Self::readyGuard>;

        return make_transition_table(
            *state<Idle> + event<Setup> / &Self::onSetup = state<ReadyWait>,
            state<ReadyWait> + event<ReadyEvent>[&Self::onReadyGuard] = state<Ready>,
            state<Ready> + event<Start> / &Self::onStart = state<Active>,
            state<Active> + event<Timeout> / &Self::onRunTimeout = state<Idle>,

            state<Active> + event<RadarData> / &Self::onRadarData = state<Active>,

            state<Active> + event<Fell> / &Self::onFell = state<Fallen>,
            state<Fallen> + event<Rose> / &Self::onRose = state<Active>,

            state<_> + event<ReadyEvent> = state<Error> // error
        );
    }

    // ––– Actions –––––––––––––––––––––––––––––––––––––––––––
    void onSetup(const Setup &e) {
        aos_.clear();
        aos_.insert(ModuleId::Radar, AoPtrVariant{e.radarAo});
        // e.radarAo->post(Setup{});
        // aos_.insert(ModuleId::Imu, AoPtrVariant{e.imuAo});
        // aos_.insert(ModuleId::Led, AoPtrVariant{e.ledAo});
        // aos_.insert(ModuleId::Audio, AoPtrVariant{e.audioAo});
        LOGI(TAG, "onSetup");
    }

    bool onReadyGuard(const ReadyEvent &) {
        readySeen_.insert(ReadyEvent);
        return readySeen_.size() == kNumModules;
    }

    void onStart(const Start &e) {
        runTimeMins_ = e.runTimeMins;
        LOGI(TAG, "onStart: runTimeMins=%u", runTimeMins_);
        // schedule Timeout via your automation API...
    }

    void onFell(const Fell &) { LOGI(TAG, "onFell: fall detected"); }
    void onRose(const Rose &) { LOGI(TAG, "onRose: rise detected"); }

    void onRunTimeout(const Timeout &) {
        LOGI(TAG, "onRunTimeout: tearing down");
        teardownAll();
    }

    void onRadarError(const RadarError &) { LOGE(TAG, "RadarError!"); }
    void onImuError(const ImuError &) { LOGE(TAG, "ImuError!"); }
    void onLedError(const LedError &) { LOGE(TAG, "LedError!"); }
    void onAudioError(const AudioError &) { LOGE(TAG, "AudioError!"); }

    void onRadarData(const RadarData &e) {
        LOGI(TAG, "onRadarData: detected=%s dist=%u cm ts=%u", e.detected ? "Y" : "N",
             static_cast<unsigned>(e.distanceCm), static_cast<unsigned>(e.timestampTicks));
    }

  private:
    static constexpr char TAG[] = "ctrl";
    AoMap aos_;
    etl::flat_set<ReadyEvent, kIntfCnt> readySeen_;

    // ––– AO storage & ready‐mask –––––––––––––––––––––
    enum AoIndex : uint8_t { IDX_RADAR = 0, IDX_IMU, IDX_LED, IDX_AUDIO, AO_COUNT };

    void *aos_[AO_COUNT]{};
    uint8_t readyMask_{0};
    static constexpr uint8_t allReadyMask_ = (1 << AO_COUNT) - 1;

    // runs for this many minutes once started
    uint32_t runTimeMins_{0};

    // ––– Guard ––––––––––––––––––––––––––––––––––––––––––
    bool readyGuard() const { return readyMask_ == allReadyMask_; }

    // ––– Teardown helper –––––––––––––––––––––––––––––––
    void teardownAll() {
        LOGI(TAG, "teardownAll: cleaning up all modules");
        // e.g. static_cast<RadarAO*>(aos_[IDX_RADAR])->post(Teardown{});
        // …and so on for each AO…
    }

    // ––– State tags –––––––––––––––––––––––––––––––––––––
    struct Idle {};
    struct ReadyReadyWait {};
    struct Ready {};
    struct Active {};
    struct Fallen {};
    struct Error {};
};

// alias for your ActiveObject
using CtrlAO = ActiveObject<CtrlFSM, CtrlEvent>;

} // namespace esphome::smart_signage