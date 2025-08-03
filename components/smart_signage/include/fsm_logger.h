#pragma once

#include "log.h"
#include "sml.hpp"
#include <cstring> // for std::strncmp

namespace esphome::smart_signage {

namespace sml = boost::sml;

// Hey, this is a bit of a quick-and-dirty hack for debugging only.

// Strips the fixed prefix from type names if present
inline const char *strip_prefix(const char *full_name) {
    constexpr const char prefix[]   = "esphome::smart_signage::";
    constexpr size_t     prefix_len = sizeof(prefix) - 1; // exclude null terminator
    if (std::strncmp(full_name, prefix, prefix_len) == 0) { return full_name + prefix_len; }
    // return full_name;
    return ""; // return empty string if prefix not found
}

struct FsmLogger {

    explicit FsmLogger(const char *tag) : TAG(tag) {}

    template <class SM, class TEvent>
    void log_process_event(const TEvent &) {
        const char *name = strip_prefix(sml::aux::get_type_name<TEvent>());
        if (name[0] != '\0') { // Only print if not empty
            LOGD("[process_event] %s", name);
        }
    }

    template <class SM, class TGuard, class TEvent>
    void log_guard(const TGuard &, const TEvent &, bool result) {
        LOGD("[guard] %s %s",
            strip_prefix(sml::aux::get_type_name<TEvent>()),
            (result ? "[OK]" : "[Reject]"));
    }

    template <class SM, class TAction, class TEvent>
    void log_action(const TAction &, const TEvent &) {
        // LOGD("[action] %s", strip_prefix(sml::aux::get_type_name<TAction>()));
    }

    template <class SM, class TSrcState, class TDstState>
    void log_state_change(const TSrcState &src, const TDstState &dst) {
        LOGD("[transition] %s -> %s", strip_prefix(src.c_str()), strip_prefix(dst.c_str()));
    }

  private:
    const char *TAG;
};

} // namespace esphome::smart_signage
