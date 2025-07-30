import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
import esphome.components.number as number
import esphome.components.button as button

CONF_RADIUS_NUM     = "radius_number"
CONF_DURATION_NUM   = "duration_number"
CONF_VOLUME_NUM     = "volume_number"
CONF_BRIGHTNESS_NUM = "brightness_number"
CONF_START_BUTTON   = "start_button"

smart_ns = cg.esphome_ns.namespace("smart_signage")
SmartSignage = smart_ns.class_("SmartSignage", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SmartSignage),

    cv.Required(CONF_RADIUS_NUM):     cv.use_id(number.Number),
    cv.Required(CONF_DURATION_NUM):   cv.use_id(number.Number),
    cv.Required(CONF_VOLUME_NUM):     cv.use_id(number.Number),
    cv.Required(CONF_BRIGHTNESS_NUM): cv.use_id(number.Number),
    cv.Required(CONF_START_BUTTON):   cv.use_id(button.Button),
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    # nothing else needed; YAML on_value / on_press automations call C++ directly
