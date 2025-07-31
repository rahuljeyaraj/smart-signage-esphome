#pragma once
#include "active_object.h"
#include "logger.h"
#include "sml.hpp"
#include <etl/variant.h>

namespace esphome {
namespace smart_signage {

namespace sml = boost::sml;

/** Events the radar FSM understands */
struct Configure {
    uint16_t detDistCm;
    uint32_t sampleIntMs;
};
struct Activate {};
struct Deactivate {};
struct TimerPoll {};

using RadarEvent = etl::variant<Configure, Activate, Deactivate, TimerPoll>;

/** Your Radar FSM functor (Boost.SML) */
class RadarFSM {
    using Self = RadarFSM;

  public:
    uint16_t detDistCm_{0};
    uint32_t sampleIntMs_{0};

    auto operator()() {
        using namespace sml;
        return make_transition_table(
            *state<Idle> + event<Configure> / &Self::act_configure = state<Ready>,
            state<Ready> + event<Activate> / &Self::act_activate = state<Active>,
            state<Active> + event<TimerPoll> / &Self::act_poll = state<Active>,
            state<Active> + event<Deactivate> / &Self::act_deactivate = state<Ready>);
    }

    void act_configure(const Configure &cfg) {
        detDistCm_ = cfg.detDistCm;
        sampleIntMs_ = cfg.sampleIntMs;
        LOGI("RadarFSM", "Configured: dist=%u cm, interval=%u ms", detDistCm_, sampleIntMs_);
    }
    void act_activate(const Activate &) { LOGI("RadarFSM", "Activated"); }
    void act_poll(const TimerPoll &) {
        LOGI("RadarFSM", "Polling… (maxDist=%u cm, Δt=%u ms)", detDistCm_, sampleIntMs_);
    }
    void act_deactivate(const Deactivate &) { LOGI("RadarFSM", "Deactivated"); }

  private:
    struct Idle {};
    struct Ready {};
    struct Active {};
};

/** Alias for your concrete driver type */
using RadarDriver = ActiveObject<RadarFSM, RadarEvent>;

} // namespace smart_signage
} // namespace esphome
