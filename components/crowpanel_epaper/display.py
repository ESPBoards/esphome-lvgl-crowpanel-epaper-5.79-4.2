from esphome import pins
import esphome.codegen as cg
from esphome.components import display
import esphome.config_validation as cv
from esphome.const import (
    CONF_BUSY_PIN, CONF_CLK_PIN, CONF_CS_PIN, CONF_DC_PIN,
    CONF_FULL_UPDATE_EVERY, CONF_ID, CONF_LAMBDA, CONF_MODEL,
    CONF_MOSI_PIN, CONF_PAGES, CONF_RESET_PIN,
)

CONF_INVERT_COLORS = "invert_colors"

ns = cg.esphome_ns.namespace("crowpanel_epaper")
Base = ns.class_("CrowPanelEPaper", display.DisplayBuffer)

MODELS = {
    "4.20in": ns.class_("CrowPanelEPaper4P2In", Base),
    "5.79in": ns.class_("CrowPanelEPaper5P79In", Base),
}

PIN_SCHEMA = {
    cv.Required(CONF_CLK_PIN):   pins.gpio_output_pin_schema,
    cv.Required(CONF_MOSI_PIN):  pins.gpio_output_pin_schema,
    cv.Required(CONF_CS_PIN):    pins.gpio_output_pin_schema,
    cv.Required(CONF_DC_PIN):    pins.gpio_output_pin_schema,
    cv.Required(CONF_RESET_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_BUSY_PIN):  pins.gpio_input_pin_schema,
}

CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend({
        cv.GenerateID(): cv.declare_id(Base),
        cv.Required(CONF_MODEL): cv.one_of(*MODELS, lower=True),
        cv.Optional(CONF_FULL_UPDATE_EVERY, default=10): cv.positive_int,
        cv.Optional(CONF_INVERT_COLORS, default=False): cv.boolean,
        **PIN_SCHEMA,
    }),
    cv.has_at_most_one_key(CONF_PAGES, CONF_LAMBDA),
)

SPI_PINS = [
    (CONF_CLK_PIN,   "set_clk_pin"),
    (CONF_MOSI_PIN,  "set_mosi_pin"),
    (CONF_CS_PIN,    "set_cs_pin"),
    (CONF_DC_PIN,    "set_dc_pin"),
    (CONF_RESET_PIN, "set_reset_pin"),
    (CONF_BUSY_PIN,  "set_busy_pin"),
]


async def to_code(config):
    cls = MODELS[config[CONF_MODEL]]
    var = cg.Pvariable(config[CONF_ID], cls.new(), cls)

    for conf_key, setter in SPI_PINS:
        pin = await cg.gpio_pin_expression(config[conf_key])
        cg.add(getattr(var, setter)(pin))

    cg.add(var.set_full_update_every(config[CONF_FULL_UPDATE_EVERY]))
    cg.add(var.set_invert_colors(config[CONF_INVERT_COLORS]))
    await display.register_display(var, config)

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA],
            [(display.DisplayRef, "it")],
            return_type=cg.void,
        )
        cg.add(var.set_writer(lambda_))
