import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
import voluptuous as vol, json  

import esphome.components.select as select
import esphome.components.number as number
import esphome.components.button as button

CONF_CURR_PROFILE    = "currProfile"
CONF_KNOB_FN         = "knobFn"
CONF_SESSION_MINS    = "sessionMins"
CONF_RADAR_RANGE_CM  = "radarRangeCm"
CONF_AUDIO_VOL_PCT   = "audioVolPct"
CONF_LED_BRIGHT_PCT  = "ledBrightPct"
CONF_START_BUTTON    = "startButton"
CONF_PROFILE_CONFIG  = "profileConfig" 

smart_ns     = cg.esphome_ns.namespace("smart_signage")
SmartSignage = smart_ns.class_("SmartSignage", cg.Component)

PROFILE_CFG_SCHEMA = vol.Schema({}, extra=vol.ALLOW_EXTRA) 

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID():                 cv.declare_id(SmartSignage),

    cv.Required(CONF_CURR_PROFILE):  cv.use_id(select.Select),
    cv.Required(CONF_KNOB_FN):       cv.use_id(select.Select),
    cv.Required(CONF_SESSION_MINS):  cv.use_id(number.Number),
    cv.Required(CONF_RADAR_RANGE_CM):cv.use_id(number.Number),
    cv.Required(CONF_AUDIO_VOL_PCT): cv.use_id(number.Number),
    cv.Required(CONF_LED_BRIGHT_PCT):cv.use_id(number.Number),
    cv.Required(CONF_START_BUTTON):  cv.use_id(button.Button),

    cv.Required(CONF_PROFILE_CONFIG): PROFILE_CFG_SCHEMA,
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    currProfile   = yield cg.get_variable(config[CONF_CURR_PROFILE])
    knobFn        = yield cg.get_variable(config[CONF_KNOB_FN])
    sessionMins   = yield cg.get_variable(config[CONF_SESSION_MINS])
    radarRangeCm  = yield cg.get_variable(config[CONF_RADAR_RANGE_CM])
    audioVolPct   = yield cg.get_variable(config[CONF_AUDIO_VOL_PCT])
    ledBrightPct  = yield cg.get_variable(config[CONF_LED_BRIGHT_PCT])
    startButton   = yield cg.get_variable(config[CONF_START_BUTTON])

    uiHandles = cg.RawExpression(
        f"esphome::smart_signage::UiHandles{{{currProfile}, "
        f"{sessionMins}, "
        f"{radarRangeCm}, "
        f"{audioVolPct}, "
        f"{ledBrightPct}, "
        f"{startButton}, "
        f"{knobFn}}}"
    )

    profile_cfg   = config[CONF_PROFILE_CONFIG]
    json_literal  = json.dumps(profile_cfg)
    json_literal  = f'R"({json_literal})"' 

    var = cg.new_Pvariable(
        config[CONF_ID],
        uiHandles,
        cg.RawExpression(json_literal) 
    )
    yield cg.register_component(var, config)
