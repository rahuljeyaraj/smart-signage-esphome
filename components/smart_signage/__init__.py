import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

smart_signage_ns = cg.esphome_ns.namespace("smart_signage")
SmartSignage = smart_signage_ns.class_("SmartSignage", cg.Component)
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(SmartSignage),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
