import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

import esphome.components.select as select
import esphome.components.number as number
import esphome.components.button as button

# ─── YAML keys ─────────────────────────────────────────────────────────────
CONF_PROFILES     = "profiles"
CONF_EVENTS       = "events"
CONF_NAME         = "name"

CONF_PROFILE_SEL  = "profile_select"
CONF_RADIUS_NUM   = "radius_number"
CONF_DURATION_NUM = "duration_number"
CONF_VOLUME_NUM   = "volume_number"
CONF_BRIGHT_NUM   = "brightness_number"
CONF_START_BTN    = "start_button"

smart_ns     = cg.esphome_ns.namespace("smart_signage")
SmartSignage = smart_ns.class_("SmartSignage", cg.Component)

# ─── Events: every value must end up as a list[str] ────────────────────────
EVENTS_SCHEMA = cv.Schema({
    cv.Optional("setup_done"):  cv.ensure_list(cv.string_strict),
    cv.Optional("setup_error"): cv.ensure_list(cv.string_strict),
    cv.Optional("start"):       cv.ensure_list(cv.string_strict),
    cv.Optional("detected"):    cv.ensure_list(cv.string_strict),
    cv.Optional("fallen"):      cv.ensure_list(cv.string_strict),
    cv.Optional("rose"):        cv.ensure_list(cv.string_strict),
    cv.Optional("run_end"):     cv.ensure_list(cv.string_strict),
    cv.Optional("low_battery"): cv.ensure_list(cv.string_strict),
})

PROFILE_SCHEMA = cv.Schema({
    cv.Required("id"):            cv.string_strict,
    cv.Required(CONF_NAME):       cv.string_strict,
    cv.Required(CONF_EVENTS):     EVENTS_SCHEMA,
})

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SmartSignage),

    cv.Required(CONF_PROFILE_SEL):  cv.use_id(select.Select),
    cv.Required(CONF_RADIUS_NUM):   cv.use_id(number.Number),
    cv.Required(CONF_DURATION_NUM): cv.use_id(number.Number),
    cv.Required(CONF_VOLUME_NUM):   cv.use_id(number.Number),
    cv.Required(CONF_BRIGHT_NUM):   cv.use_id(number.Number),
    cv.Required(CONF_START_BTN):    cv.use_id(button.Button),

    cv.Required(CONF_PROFILES): cv.ensure_list(PROFILE_SCHEMA),
}).extend(cv.COMPONENT_SCHEMA)

# ─── C++ code-gen ──────────────────────────────────────────────────────────
def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    for prof in config[CONF_PROFILES]:
        prof_name = prof[CONF_NAME]
        for evt, paths in prof[CONF_EVENTS].items():
            # Build: std::vector<std::string>{ "a", "b", "c" }
            vec_literal = (
                "std::vector<std::string>{" +
                ", ".join([f'"{p}"' for p in paths]) +
                "}"
            )
            cg.add(cg.RawExpression(
                f'{var}->add_event_map("{prof_name}", "{evt}", {vec_literal});'
            ))



# import esphome.codegen as cg
# import esphome.config_validation as cv
# from esphome.const import CONF_ID
# import esphome.components.select as select
# import esphome.components.number as number
# import esphome.components.button as button

# CONF_PROFILE_SELECT = "profile_select"
# CONF_RADIUS_NUM     = "radius_number"
# CONF_DURATION_NUM   = "duration_number"
# CONF_VOLUME_NUM     = "volume_number"
# CONF_BRIGHTNESS_NUM = "brightness_number"
# CONF_START_BUTTON   = "start_button"

# smart_ns = cg.esphome_ns.namespace("smart_signage")
# SmartSignage = smart_ns.class_("SmartSignage", cg.Component)

# CONFIG_SCHEMA = cv.Schema({
#     cv.GenerateID(): cv.declare_id(SmartSignage),

#     cv.Required(CONF_PROFILE_SELECT): cv.use_id(select.Select),
#     cv.Required(CONF_RADIUS_NUM):     cv.use_id(number.Number),
#     cv.Required(CONF_DURATION_NUM):   cv.use_id(number.Number),
#     cv.Required(CONF_VOLUME_NUM):     cv.use_id(number.Number),
#     cv.Required(CONF_BRIGHTNESS_NUM): cv.use_id(number.Number),
#     cv.Required(CONF_START_BUTTON):   cv.use_id(button.Button),
# }).extend(cv.COMPONENT_SCHEMA)

# def to_code(config):
#     var = cg.new_Pvariable(config[CONF_ID])
#     yield cg.register_component(var, config)
#     # nothing else needed; YAML on_value / on_press automations call C++ directly
