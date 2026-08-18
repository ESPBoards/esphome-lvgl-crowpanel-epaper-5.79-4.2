#pragma once

// UC8276C e-paper controller - command and parameter definitions.
// Used by the v1.2 CrowPanel 4.2" revision, which replaced the SSD1683 of the
// original board. v1.2 is what the silkscreen prints next to the SKU - Elecrow
// also calls it the "green sticker" board, but not every v1.2 ships with one.
// Commands follow the UC8276 datasheet; the waveforms in epd_4p2_uc8276.cpp
// come from Elecrow's demo driver.
#include <cstddef>
#include <cstdint>

namespace esphome {
namespace crowpanel_epaper {

// Commands
constexpr uint8_t UC_PANEL_SETTING     = 0x00;
constexpr uint8_t UC_POWER_SETTING     = 0x01;
constexpr uint8_t UC_POWER_OFF         = 0x02;
constexpr uint8_t UC_POWER_ON          = 0x04;
constexpr uint8_t UC_BOOSTER_SOFT_ST   = 0x06;
constexpr uint8_t UC_DEEP_SLEEP        = 0x07;
constexpr uint8_t UC_DATA_START_1      = 0x10;  // "old" / previous frame plane
constexpr uint8_t UC_DISPLAY_REFRESH   = 0x12;
constexpr uint8_t UC_DATA_START_2      = 0x13;  // "new" / current frame plane
constexpr uint8_t UC_LUT_VCOM          = 0x20;
constexpr uint8_t UC_LUT_WW            = 0x21;
constexpr uint8_t UC_LUT_BW            = 0x22;
constexpr uint8_t UC_LUT_WB            = 0x23;
constexpr uint8_t UC_LUT_BB            = 0x24;
constexpr uint8_t UC_PLL_CONTROL       = 0x30;
constexpr uint8_t UC_VCOM_INTERVAL     = 0x50;
constexpr uint8_t UC_TCON_SETTING      = 0x60;
constexpr uint8_t UC_RESOLUTION        = 0x61;
constexpr uint8_t UC_VCOM_DC_SETTING   = 0x82;
constexpr uint8_t UC_POWER_SAVING      = 0xE3;

// LUT layout. Each of 0x20-0x24 holds six 7-byte phase groups; shorter tables
// are zero-padded on upload, leaving the unused phases inactive.
constexpr size_t  UC_LUT_GROUP_LEN     = 7;
constexpr size_t  UC_LUT_REG_LEN       = 42;

// Parameters
// RAM polarity is inverted relative to the SSD1683 models: a set bit renders
// black, so white is 0x00 here.
constexpr uint8_t UC_PIXEL_WHITE       = 0x00;
constexpr uint8_t UC_DEEP_SLEEP_CHECK  = 0xA5;

}  // namespace crowpanel_epaper
}  // namespace esphome
