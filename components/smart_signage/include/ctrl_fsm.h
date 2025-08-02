// #pragma once
// #include "active_object.h"
// #include "log.h"
// #include "radar_fsm.h"
// #include "sml.hpp"
// #include <etl/flat_map.h>
// #include <etl/flat_set.h>
// #include <etl/variant.h>

// namespace sml = boost::sml;

// namespace esphome::smart_signage::ctrl {

// class FSM {
//     using Self = FSM;

//   public:
//     FSM() = default;

//     auto operator()() noexcept {
//         using namespace boost::sml;
//         return make_transition_table(
//             *"idle"_s + event<Setup> / &Self::onSetup = "ready_wait"_s,              //
//             "ready_wait"_s + event<radar::SetupDone>[&Self::SetupGuard] = "ready"_s, //
//             "ready"_s + event<Start> / &Self::onStart = "active"_s,
//             "active"_s + event<Timeout> / &Self::onRunTimeout = "idle"_s,
//             "active"_s + event<radar::Data> / &Self::onEvtRadarData = "active"_s,
//             // "active"_s + event<imu::Fell> / &Self::onFell = "fallen"_s,
//             // "fallen"_s + event<imu::Rose> / &Self::onRose = "active"_s,
//             state<_> + event<InitError> = "error"_s // error
//         );
//     }

//     // Guards
//     bool SetupGuard(const radar::SetupDone &e) {
//         // readySeen_.insert(e);
//         // etl::visit([this](auto const &ev) { readySeen_.insert(ev); }, e);
//         // return readySeen_.size() == kIntfCnt;
//         return true;
//     }

//     // Actions
//     void onSetup(const Setup &) {
//         radar::RxEvent radarSetup(radar::Setup{});
//         radarQ.post(&radarSetup);

//         LOGI(TAG, "onSetup");
//     }

//     void onStart(const Start &e) {
//         radar::RxEvent radarStart(radar::Start{});
//         radarQ.post(&radarStart);

//         runTimeMins_ = e.runTimeMins;
//         LOGI(TAG, "onStart: runTimeMins=%u", runTimeMins_);
//         // schedule Timeout via your automation API...
//     }

//     // void onFell(const Fell &) { LOGI(TAG, "onFell: fall detected"); }
//     // void onRose(const Rose &) { LOGI(TAG, "onRose: rise detected"); }

//     void onRunTimeout(const Timeout &) {
//         radar::RxEvent radarStop(radar::Stop{});
//         radarQ.post(&radarStop);

//         radar::RxEvent radarTeardown(radar::Teardown{});
//         radarQ.post(&radarTeardown);

//         LOGI(TAG, "onRunTimeout: tearing down");
//         teardownAll();
//     }

//     void onError(const InitError &) { LOGE(TAG, "InitError!"); }

//     void onEvtRadarData(const radar::Data &e) {
//         LOGI(TAG, "onEvtRadarData: detected=%s dist=%u cm ts=%u", e.detected ? "Y" : "N",
//              static_cast<unsigned>(e.distanceCm), static_cast<unsigned>(e.timestampTicks));
//     }

//   private:
//     static constexpr char TAG[] = "ctrl";

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
//     // struct Setup {};
//     // struct Ready {};
//     // struct Active {};
//     // struct Fallen {};
//     // struct Error {};
// };

// } // namespace esphome::smart_signage::ctrl