#pragma once
// user_intf.h — header-only, STATELESS UI <-> CTRL bridge using ProfileName (15-char etl)

#include <cstdint>
#include <vector>
#include <string>
#include <etl/vector.h>

#include "log.h" // SS_LOGD/SS_LOGI/SS_LOGW/SS_LOGE (ASCII only)

#include "esphome/components/select/select.h"
#include "esphome/components/number/number.h"
#include "esphome/components/button/button.h"

#include "common.h"
#include "ctrl/ctrl_q.h" // ctrl::Q with post(...)

namespace esphome::smart_signage {

struct UiHandles {
    esphome::select::Select *currProfile  = nullptr;
    esphome::number::Number *sessionMins  = nullptr;
    esphome::number::Number *radarRangeCm = nullptr;
    esphome::number::Number *audioVolPct  = nullptr;
    esphome::number::Number *ledBrightPct = nullptr;
    esphome::button::Button *startButton  = nullptr;
};

template <size_t MAX_PROFILES>
class UserIntf {
  public:
    static constexpr const char *TAG = "UserIntf";

    explicit UserIntf(const UiHandles &ui, ctrl::Q &ctrlQ) : ui_(ui), ctrlQ_(ctrlQ) {
        attachCallbacks_();
        SS_LOGD("%s: constructed", TAG);
    }

    // ─────────────── CTRL → UI (no echo back) ───────────────

    // Set the select's options (ETL -> std::vector<std::string>)
    void setProfileOptions(const etl::vector<ProfileName, MAX_PROFILES> &opts) {
        std::vector<std::string> stdOpts;
        stdOpts.reserve(opts.size());
        for (const auto &lbl : opts) stdOpts.emplace_back(lbl.c_str());

        if (ui_.currProfile) {
            suppressEvents_([&] { ui_.currProfile->traits.set_options(stdOpts); });
        }
        SS_LOGI("%s: profile options set, count=%u", TAG, (unsigned) opts.size());
    }

    // Select a profile by ProfileName
    void setCurrentProfile(const ProfileName &name) {
        if (ui_.currProfile) {
            suppressEvents_([&] { ui_.currProfile->publish_state(name.c_str()); });
            SS_LOGI("%s: current profile -> \"%s\"", TAG, name.c_str());
        }
    }

    // Numbers: just publish (stateless); clamp where it makes sense
    void setSessionMins(uint32_t mins) {
        if (ui_.sessionMins) suppressEvents_([&] { ui_.sessionMins->publish_state((float) mins); });
        SS_LOGI("%s: session mins -> %u", TAG, mins);
    }

    void setRadarRangeCm(uint32_t cm) {
        if (ui_.radarRangeCm) suppressEvents_([&] { ui_.radarRangeCm->publish_state((float) cm); });
        SS_LOGI("%s: radar range cm -> %u", TAG, cm);
    }

    void setAudioVolPct(uint8_t pct) {
        if (pct > 100) pct = 100;
        if (ui_.audioVolPct) suppressEvents_([&] { ui_.audioVolPct->publish_state((float) pct); });
        SS_LOGI("%s: audio vol pct -> %hhu", TAG, pct);
    }

    void setLedBrightPct(uint8_t pct) {
        if (pct > 100) pct = 100;
        if (ui_.ledBrightPct)
            suppressEvents_([&] { ui_.ledBrightPct->publish_state((float) pct); });
        SS_LOGI("%s: led bright pct -> %hhu", TAG, pct);
    }

  private:
    UiHandles ui_{};
    ctrl::Q  &ctrlQ_;
    bool      suppressing_{false}; // only to prevent echo during programmatic publishes

    template <typename F>
    void suppressEvents_(F &&f) {
        bool old     = suppressing_;
        suppressing_ = true;
        f();
        suppressing_ = old;
    }

    // ─────────────── UI → CTRL wiring ───────────────
    void attachCallbacks_() {
        // Start button
        if (ui_.startButton) {
            // prefer add_on_press_callback if available
            ui_.startButton->add_on_press_callback([this]() {
                if (suppressing_) return;
                ctrlQ_.post(ctrl::EvtUiStartPressed{});
                SS_LOGI("%s: UI->CTRL StartPressed", TAG);
            });
        }

        // Profile select — (value, index); we only use the value, which we truncate to ProfileName.
        if (ui_.currProfile) {
            ui_.currProfile->add_on_state_callback([this](std::string value, size_t /*index*/) {
                if (suppressing_) return;
                ctrl::EvtUiProfileUpdate ev{};
                ev.profileName = value.c_str(); // converts + truncates to 15 char ProfileName
                ctrlQ_.post(ev);
                SS_LOGI("%s: UI->CTRL ProfileUpdate value=\"%s\"", TAG, value.c_str());
            });
        }

        // Session minutes
        if (ui_.sessionMins) {
            ui_.sessionMins->add_on_state_callback([this](float v) {
                if (suppressing_) return;
                uint32_t val = (v < 0.f) ? 0U : (uint32_t) v;
                ctrlQ_.post(ctrl::EvtUiSessionMinsUpdate{val});
                SS_LOGI("%s: UI->CTRL SessionMinsUpdate %u", TAG, val);
            });
        }

        // Radar range cm
        if (ui_.radarRangeCm) {
            ui_.radarRangeCm->add_on_state_callback([this](float v) {
                if (suppressing_) return;
                uint32_t val = (v < 0.f) ? 0U : (uint32_t) v;
                ctrlQ_.post(ctrl::EvtUiRangeCmUpdate{val});
                SS_LOGI("%s: UI->CTRL RangeCmUpdate %u", TAG, val);
            });
        }

        // Audio volume pct (0..100 clamp)
        if (ui_.audioVolPct) {
            ui_.audioVolPct->add_on_state_callback([this](float v) {
                if (suppressing_) return;
                int iv = (v < 0.f) ? 0 : (int) v;
                if (iv > 100) iv = 100;
                ctrlQ_.post(ctrl::EvtUiAudioVolUpdate{(uint8_t) iv});
                SS_LOGI("%s: UI->CTRL AudioVolUpdate %hhu", TAG, (uint8_t) iv);
            });
        }

        // LED brightness pct (0..100 clamp)
        if (ui_.ledBrightPct) {
            ui_.ledBrightPct->add_on_state_callback([this](float v) {
                if (suppressing_) return;
                int iv = (v < 0.f) ? 0 : (int) v;
                if (iv > 100) iv = 100;
                ctrlQ_.post(ctrl::EvtUiLedBrightUpdate{(uint8_t) iv});
                SS_LOGI("%s: UI->CTRL LedBrightUpdate %hhu", TAG, (uint8_t) iv);
            });
        }
    }
};
using UserIntfT = UserIntf<SS_MAX_PROFILES>;

} // namespace esphome::smart_signage
