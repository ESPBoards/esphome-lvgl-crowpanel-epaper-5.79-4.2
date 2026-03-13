#include "crowpanel_epaper.h"
#include "ssd1683.h"
#include "esphome/core/log.h"
#include <cstring>

namespace esphome {
namespace crowpanel_epaper {

static const char *const TAG = "crowpanel_epaper";

// Refresh sequences (shared by all models)
static const uint8_t SEQ_REFRESH_FULL[] = {
    CMD_UPDATE_SEQ, 0x01, SEQ_FULL_UPDATE,
    CMD_DISPLAY_UPDATE, DELAY_BIT, 10,
    SEQ_END, SEQ_END,
};
static const uint8_t SEQ_REFRESH_PARTIAL[] = {
    CMD_UPDATE_SEQ, 0x01, SEQ_PARTIAL_UPDATE,
    CMD_DISPLAY_UPDATE, DELAY_BIT, 10,
    SEQ_END, SEQ_END,
};
static const uint8_t SEQ_SLEEP[] = {
    CMD_DEEP_SLEEP, 0x01, DEEP_SLEEP_MODE1,
    SEQ_END, SEQ_END,
};

static constexpr uint32_t RESET_PULSE_MS = 100;
static constexpr uint32_t POST_RESET_MS = 10;
static constexpr uint32_t BUSY_TIMEOUT_MS = 2000;
static constexpr uint32_t REFRESH_TIMEOUT_MS = 60000;

// -- Software SPI -----------------------------------------------------------
void CrowPanelEPaper::spi_setup_pins_() {
  dc_pin_->setup();
  dc_pin_->digital_write(true);
  cs_pin_->setup();
  cs_pin_->digital_write(true);
  clk_pin_->setup();
  clk_pin_->digital_write(false);
  mosi_pin_->setup();
  if (reset_pin_)
    reset_pin_->setup();
  if (busy_pin_)
    busy_pin_->setup();
}

void CrowPanelEPaper::spi_write_byte_(uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    clk_pin_->digital_write(false);
    mosi_pin_->digital_write(data & 0x80);
    clk_pin_->digital_write(true);
    data <<= 1;
  }
}

void CrowPanelEPaper::spi_start_command_() {
  dc_pin_->digital_write(false);
  cs_pin_->digital_write(false);
}

void CrowPanelEPaper::spi_end_command_() {
  cs_pin_->digital_write(true);
  dc_pin_->digital_write(true);
}

void CrowPanelEPaper::spi_start_data_() {
  dc_pin_->digital_write(true);
  cs_pin_->digital_write(false);
}

void CrowPanelEPaper::spi_end_data_() {
  cs_pin_->digital_write(true);
}

void CrowPanelEPaper::spi_command_(uint8_t cmd) {
  spi_start_command_();
  spi_write_byte_(cmd);
  spi_end_command_();
}

void CrowPanelEPaper::spi_data_(uint8_t data) {
  spi_start_data_();
  spi_write_byte_(data);
  spi_end_data_();
}

void CrowPanelEPaper::spi_send_sequence_(const uint8_t *seq) {
  if (!seq)
    return;
  uint32_t i = 0;
  while (true) {
    uint8_t cmd = seq[i++];
    uint8_t argc = seq[i++];
    if (cmd == SEQ_END && argc == SEQ_END)
      break;
    spi_command_(cmd);
    if (argc & DELAY_BIT) {
      delay(seq[i++]);  // NOLINT
      argc &= ARG_MASK;
    }
    for (uint8_t j = 0; j < argc; j++)
      spi_data_(seq[i++]);
  }
}

bool CrowPanelEPaper::is_busy_() {
  return busy_pin_ && busy_pin_->digital_read();
}

// -- Lifecycle --------------------------------------------------------------
void CrowPanelEPaper::setup() {
  ESP_LOGD(TAG, "Setting up e-paper display...");
  init_internal_(get_buffer_length_());
  fill(display::COLOR_OFF);
  spi_setup_pins_();
  state_ = State::INIT_RESET_HIGH;
  state_start_ = millis();
}

void CrowPanelEPaper::loop() {
  uint32_t now = millis();

  switch (state_) {
    case State::UNINIT:
      break;

    case State::INIT_RESET_HIGH:
      if (reset_pin_) {
        reset_pin_->digital_write(true);
        state_ = State::INIT_RESET_LOW;
        state_start_ = now;
      } else {
        state_ = State::INIT_COMMANDS;
      }
      break;

    case State::INIT_RESET_LOW:
      if (now - state_start_ >= RESET_PULSE_MS) {
        reset_pin_->digital_write(false);
        state_ = State::INIT_RESET_DONE;
        state_start_ = now;
      }
      break;

    case State::INIT_RESET_DONE:
      if (now - state_start_ >= POST_RESET_MS) {
        reset_pin_->digital_write(true);
        state_ = State::INIT_COMMANDS;
        state_start_ = now;
      }
      break;

    case State::INIT_COMMANDS:
      if (now - state_start_ >= POST_RESET_MS) {
        init_display_();
        state_ = State::INIT_WAIT;
        state_start_ = now;
      }
      break;

    case State::INIT_WAIT:
      if (!is_busy_()) {
        ESP_LOGD(TAG, "Display ready");
        state_ = State::IDLE;
      } else if (now - state_start_ > BUSY_TIMEOUT_MS) {
        ESP_LOGW(TAG, "Busy timeout during init, proceeding anyway");
        state_ = State::IDLE;
      }
      break;

    case State::IDLE:
      if (needs_update_) {
        needs_update_ = false;
        update_count_++;
        is_full_update_ = force_full_ || update_count_ == 1 ||
                          update_count_ % full_update_every_ == 0;
        force_full_ = false;
        ESP_LOGD(TAG, "%s refresh #%u",
                 is_full_update_ ? "Full" : "Partial", update_count_);
        state_ = State::UPDATE_PREPARE;
        state_start_ = now;
      }
      break;

    case State::UPDATE_PREPARE:
      if (!is_busy_()) {
        prepare_update_();
        state_ = State::UPDATE_SEND_DATA;
        state_start_ = now;
      } else if (now - state_start_ > BUSY_TIMEOUT_MS) {
        ESP_LOGW(TAG, "Busy timeout before update, proceeding");
        prepare_update_();
        state_ = State::UPDATE_SEND_DATA;
        state_start_ = now;
      }
      break;

    case State::UPDATE_SEND_DATA:
      send_data_();
      state_ = State::UPDATE_REFRESH;
      state_start_ = millis();
      break;

    case State::UPDATE_REFRESH:
      spi_send_sequence_(is_full_update_ ? SEQ_REFRESH_FULL : SEQ_REFRESH_PARTIAL);
      state_ = State::UPDATE_WAIT;
      state_start_ = millis();
      break;

    case State::UPDATE_WAIT:
      if (!is_busy_()) {
        ESP_LOGD(TAG, "Refresh done");
        state_ = State::IDLE;
      } else if (now - state_start_ > REFRESH_TIMEOUT_MS) {
        ESP_LOGW(TAG, "Refresh timeout after %u ms", REFRESH_TIMEOUT_MS);
        state_ = State::IDLE;
      }
      break;

    case State::DEEP_SLEEP:
      break;
  }
}

void CrowPanelEPaper::update() {
  do_update_();
  needs_update_ = true;
}

void CrowPanelEPaper::on_safe_shutdown() {
  spi_send_sequence_(SEQ_SLEEP);
  state_ = State::DEEP_SLEEP;
}

void CrowPanelEPaper::dump_config() {
  LOG_DISPLAY("", "CrowPanel E-Paper", this);
  LOG_PIN("  CLK:   ", clk_pin_);
  LOG_PIN("  MOSI:  ", mosi_pin_);
  LOG_PIN("  CS:    ", cs_pin_);
  LOG_PIN("  DC:    ", dc_pin_);
  LOG_PIN("  Reset: ", reset_pin_);
  LOG_PIN("  Busy:  ", busy_pin_);
  ESP_LOGCONFIG(TAG, "  Full update every: %u", full_update_every_);
  ESP_LOGCONFIG(TAG, "  Invert: %s", YESNO(invert_colors_));
}

// -- Pixel buffer -----------------------------------------------------------

uint32_t CrowPanelEPaper::get_buffer_length_() {
  return (uint32_t) native_width_() * native_height_() / 8u;
}

int CrowPanelEPaper::get_width_internal() {
  if (rotation_ == display::DISPLAY_ROTATION_90_DEGREES ||
      rotation_ == display::DISPLAY_ROTATION_270_DEGREES)
    return native_height_();
  return native_width_();
}

int CrowPanelEPaper::get_height_internal() {
  if (rotation_ == display::DISPLAY_ROTATION_90_DEGREES ||
      rotation_ == display::DISPLAY_ROTATION_270_DEGREES)
    return native_width_();
  return native_height_();
}

bool CrowPanelEPaper::rotate_pixel_(int x, int y, int *ox, int *oy) {
  int nw = native_width_(), nh = native_height_();
  switch (rotation_) {
    case display::DISPLAY_ROTATION_0_DEGREES:   *ox = x;          *oy = y;          break;
    case display::DISPLAY_ROTATION_90_DEGREES:  *ox = y;          *oy = nw - 1 - x; break;
    case display::DISPLAY_ROTATION_180_DEGREES: *ox = nw - 1 - x; *oy = nh - 1 - y; break;
    case display::DISPLAY_ROTATION_270_DEGREES: *ox = nh - 1 - y; *oy = x;          break;
    default: return false;
  }
  return *ox >= 0 && *ox < nw && *oy >= 0 && *oy < nh;
}

void CrowPanelEPaper::draw_absolute_pixel_internal(int x, int y, Color color) {
  int rx, ry;
  if (!rotate_pixel_(x, y, &rx, &ry))
    return;

  // Mirror X: bit 7 of each byte is the leftmost pixel
  rx = native_width_() - 1 - rx;

  uint32_t byte_off = ((uint32_t) ry * native_width_() + rx) / 8u;
  uint8_t bit = 7 - (rx % 8);
  if (byte_off >= get_buffer_length_())
    return;

  bool ink = color.is_on();
  if (invert_colors_)
    ink = !ink;

  if (ink)
    buffer_[byte_off] &= ~(1 << bit);  // 0 = black
  else
    buffer_[byte_off] |= (1 << bit);   // 1 = white
}

void CrowPanelEPaper::fill(Color color) {
  bool ink = color.is_on();
  if (invert_colors_)
    ink = !ink;
  if (buffer_)
    std::memset(buffer_, ink ? 0x00 : 0xFF, get_buffer_length_());
}

}  // namespace crowpanel_epaper
}  // namespace esphome
