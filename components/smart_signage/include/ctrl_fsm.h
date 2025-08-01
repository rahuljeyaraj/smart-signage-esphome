// #pragma once
// #include "active_object.h"
// #include "common.h"
// #include "radar_fsm.h"
// #include "sml.hpp"
// #include <etl/flat_map.h>
// #include <etl/flat_set.h>
// #include <etl/variant.h>

// namespace sml = boost::sml;

// namespace esphome::smart_signage::ctrl {

// enum class IntfId : uint8_t { Radar = 0, /*Imu, Led, Audio,*/ Count };
// static constexpr size_t kIntfCnt = static_cast<size_t>(IntfId::Count);
// using AoPtrVariant = etl::variant<radar::AO * /*, ImuAO *, LedAO *, AudioAO **/>;
// using AoMap = etl::flat_map<IntfId, AoPtrVariant, kIntfCnt>;

// // class ImuAO;
// // class LedAO;
// // class AudioAO;

// // Events
// struct Setup {
//     radar::AO *radarAo;
//     /* led::AO *ledAo;
//     audio::AO *audioAo;
//     imu::AO *imuAo;*/
// };

// struct Start {
//     uint32_t runTimeMins; // Run duration
// };
// struct Timeout {};
// struct Fell {};
// struct Rose {};

// struct ImuReady {};
// struct LedReady {};
// struct AudioReady {};

// struct ImuError {};
// struct LedError {};
// struct AudioError {};

// using Ready = etl::variant<radar::Ready, ImuReady, LedReady, AudioReady>;
// using Error = etl::variant<radar::Error, ImuError, LedError, AudioError>;
// using Event = etl::variant<Setup, Start, Ready, Error, radar::Data, Fell, Rose, Timeout>;

// // CtrlFSM
// class FSM {
//     using Self = FSM;

//   public:
//     FSM() = default;

//     auto operator()() noexcept {
//         using namespace boost::sml;
//         return make_transition_table(
//             *"idle"_s + event<Setup> / &Self::onSetup = "ready_wait"_s,     //
//             "ready_wait"_s + event<Ready>[&Self::onReadyGuard] = "ready"_s, //
//             "ready"_s + event<Start> / &Self::onStart = "active"_s,
//             "active"_s + event<Timeout> / &Self::onRunTimeout = "idle"_s,
//             "active"_s + event<radar::Data> / &Self::onRadarData = "active"_s,
//             "active"_s + event<Fell> / &Self::onFell = "fallen"_s,
//             "fallen"_s + event<Rose> / &Self::onRose = "active"_s,
//             state<_> + event<Error> = "error"_s // error
//         );
//     }

//     // Guards
//     bool onReadyGuard(const Ready &e) {
//         // readySeen_.insert(e);
//         // etl::visit([this](auto const &ev) { readySeen_.insert(ev); }, e);
//         // return readySeen_.size() == kIntfCnt;
//         return true;
//     }

//     // Actions
//     void onSetup(const Setup &e) {
//         aos_.radarAo = e.radarAo;
//         aos_.radarAo->post(radar::Setup{});

//         // e.radarAo->post(Setup{});
//         // aos_.insert(ModuleId::Imu, AoPtrVariant{e.imuAo});
//         // aos_.insert(ModuleId::Led, AoPtrVariant{e.ledAo});
//         // aos_.insert(ModuleId::Audio, AoPtrVariant{e.audioAo});
//         LOGI(TAG, "onSetup");
//     }

//     void onStart(const Start &e) {
//         runTimeMins_ = e.runTimeMins;
//         LOGI(TAG, "onStart: runTimeMins=%u", runTimeMins_);
//         // schedule Timeout via your automation API...
//     }

//     void onFell(const Fell &) { LOGI(TAG, "onFell: fall detected"); }
//     void onRose(const Rose &) { LOGI(TAG, "onRose: rise detected"); }

//     void onRunTimeout(const Timeout &) {
//         LOGI(TAG, "onRunTimeout: tearing down");
//         teardownAll();
//     }

//     void onError(const Error &) { LOGE(TAG, "Error!"); }

//     void onRadarData(const radar::Data &e) {
//         LOGI(TAG, "onRadarData: detected=%s dist=%u cm ts=%u", e.detected ? "Y" : "N",
//              static_cast<unsigned>(e.distanceCm), static_cast<unsigned>(e.timestampTicks));
//     }

//   private:
//     static constexpr char TAG[] = "ctrl";
//     struct IntfAO {
//         radar::AO *radarAo;
//         /* led::AO *ledAo;
//         audio::AO *audioAo;
//         imu::AO *imuAo;*/
//     } aos_;
//     // etl::flat_set<Ready, kIntfCnt> readySeen_;

//     // runs for this many minutes once started
//     uint32_t runTimeMins_{0};

//     // ––– Teardown helper –––––––––––––––––––––––––––––––
//     void teardownAll() {
//         LOGI(TAG, "teardownAll: cleaning up all modules");
//         // e.g. static_cast<RadarAO*>(aos_[IDX_RADAR])->post(Teardown{});
//         // …and so on for each AO…
//     }

//     // ––– State tags –––––––––––––––––––––––––––––––––––––
//     // struct Idle {};
//     // struct ReadyWait {};
//     // struct Ready {};
//     // struct Active {};
//     // struct Fallen {};
//     // struct Error {};
// };

// } // namespace esphome::smart_signage::ctrl