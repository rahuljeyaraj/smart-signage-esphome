#include "active_object.h"
#include "radar/fsm.h"
#include "radar/q.h"

namespace esphome::smart_signage {
namespace radar {

using AO = ActiveObject<FSM, Q>;

} // namespace radar
} // namespace esphome::smart_signage