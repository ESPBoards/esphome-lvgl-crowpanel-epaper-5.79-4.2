#include "crowpanel_epaper.h"
#include "ssd1683.h"
#include "esphome/core/log.h"

namespace esphome {
namespace crowpanel_epaper {

static const char *const TAG = "epd_4p2";

static const uint8_t SEQ_INIT[] = {
    CMD_SOFT_RESET, 0x00,
    CMD_SET_MUX, 0x03, 0x2B, 0x01, 0x00,          // MUX = 300
    CMD_UPDATE_CTRL1, 0x02, 0x40, CTRL1_SINGLE,
    CMD_BORDER_WAVEFORM, 0x01, BORDER_FULL,
    CMD_DATA_ENTRY_MODE, 0x01, ENTRY_X_INC_Y_INC,
    CMD_SET_X_ADDR, 0x02, 0x00, 0x31,              // 0..49 (400px)
    CMD_SET_Y_ADDR, 0x04, 0x00, 0x00, 0x2B, 0x01,  // 0..299
    CMD_SET_X_CTR, 0x01, 0x00,
    CMD_SET_Y_CTR, 0x02, 0x00, 0x00,
    SEQ_END, SEQ_END,
};

void CrowPanelEPaper4P2In::init_display_() {
  ESP_LOGD(TAG, "Initialising 4.2\" single-chip display");
  spi_send_sequence_(SEQ_INIT);
}

void CrowPanelEPaper4P2In::prepare_update_() {
  spi_command_(CMD_BORDER_WAVEFORM);
  spi_data_(is_full_update_ ? BORDER_FULL : BORDER_PARTIAL);

  spi_command_(CMD_UPDATE_CTRL1);
  spi_data_(is_full_update_ ? 0x40 : 0x00);
  spi_data_(CTRL1_SINGLE);

  spi_command_(CMD_SET_X_CTR);
  spi_data_(0x00);
  spi_command_(CMD_SET_Y_CTR);
  spi_data_(0x00);
  spi_data_(0x00);

  // On full refresh, write buffer to OLD RAM (0x26) first so the controller
  // has a valid baseline for subsequent partial refresh diffs.
  if (is_full_update_) {
    spi_command_(0x26);
    spi_start_data_();
    for (size_t i = 0; i < get_buffer_length_(); i++)
      spi_write_byte_(buffer_[i]);
    spi_end_data_();

    spi_command_(CMD_SET_X_CTR);
    spi_data_(0x00);
    spi_command_(CMD_SET_Y_CTR);
    spi_data_(0x00);
    spi_data_(0x00);
  }

  spi_command_(CMD_WRITE_RAM);
  spi_start_data_();
}

void CrowPanelEPaper4P2In::send_data_() {
  size_t total = get_buffer_length_();
  for (size_t i = 0; i < total; i++)
    spi_write_byte_(buffer_[i]);
  spi_end_data_();
}

}  // namespace crowpanel_epaper
}  // namespace esphome
