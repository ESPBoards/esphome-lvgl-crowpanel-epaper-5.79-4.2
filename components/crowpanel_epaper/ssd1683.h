#pragma once

// SSD1683 e-paper controller — command and parameter definitions.
// Cascade mode: OR command byte with TGT_SECONDARY to address the second chip.
#include <cstdint>

namespace esphome {
namespace crowpanel_epaper {

// Commands
constexpr uint8_t CMD_SET_MUX          = 0x01;
constexpr uint8_t CMD_DEEP_SLEEP       = 0x10;
constexpr uint8_t CMD_DATA_ENTRY_MODE  = 0x11;
constexpr uint8_t CMD_DISPLAY_UPDATE   = 0x20;
constexpr uint8_t CMD_UPDATE_CTRL1     = 0x21;
constexpr uint8_t CMD_UPDATE_SEQ       = 0x22;
constexpr uint8_t CMD_WRITE_RAM        = 0x24;
constexpr uint8_t CMD_BORDER_WAVEFORM  = 0x3C;
constexpr uint8_t CMD_SET_X_ADDR       = 0x44;
constexpr uint8_t CMD_SET_Y_ADDR       = 0x45;
constexpr uint8_t CMD_SET_X_CTR        = 0x4E;
constexpr uint8_t CMD_SET_Y_CTR        = 0x4F;
constexpr uint8_t CMD_SOFT_RESET       = 0x12;

// Cascade target selector (OR with command byte)
constexpr uint8_t TGT_PRIMARY          = 0x00;
constexpr uint8_t TGT_SECONDARY        = 0x80;

// Parameters
constexpr uint8_t BORDER_FULL          = 0x05;
constexpr uint8_t BORDER_PARTIAL       = 0x80;
constexpr uint8_t SEQ_FULL_UPDATE      = 0xF7;
constexpr uint8_t SEQ_PARTIAL_UPDATE   = 0xFF;
constexpr uint8_t DEEP_SLEEP_MODE1     = 0x01;
constexpr uint8_t ENTRY_X_INC_Y_INC   = 0x03;
constexpr uint8_t ENTRY_X_DEC_Y_INC   = 0x02;
constexpr uint8_t CTRL1_SINGLE        = 0x00;
constexpr uint8_t CTRL1_CASCADE       = 0x10;

// Sequence encoding for spi_send_sequence_()
constexpr uint8_t SEQ_END   = 0xFF;
constexpr uint8_t DELAY_BIT = 0x80;
constexpr uint8_t ARG_MASK  = 0x7F;

}  // namespace crowpanel_epaper
}  // namespace esphome
