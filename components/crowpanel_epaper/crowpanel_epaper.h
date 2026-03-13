#pragma once

#include "esphome/components/display/display_buffer.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace crowpanel_epaper {

// Panel dimensions
constexpr uint16_t WIDTH_4P2   = 400;
constexpr uint16_t HEIGHT_4P2  = 300;
constexpr uint16_t WIDTH_5P79  = 792;
constexpr uint16_t HEIGHT_5P79 = 272;

enum class State : uint8_t {
  UNINIT,
  INIT_RESET_HIGH,
  INIT_RESET_LOW,
  INIT_RESET_DONE,
  INIT_COMMANDS,
  INIT_WAIT,
  IDLE,
  UPDATE_PREPARE,
  UPDATE_SEND_DATA,
  UPDATE_REFRESH,
  UPDATE_WAIT,
  DEEP_SLEEP,
};

// ---------------------------------------------------------------------------
// Base class — software SPI, state machine, pixel buffer
// ---------------------------------------------------------------------------
class CrowPanelEPaper : public display::DisplayBuffer {
 public:
  void set_clk_pin(GPIOPin *pin)   { clk_pin_ = pin; }
  void set_mosi_pin(GPIOPin *pin)  { mosi_pin_ = pin; }
  void set_cs_pin(GPIOPin *pin)    { cs_pin_ = pin; }
  void set_dc_pin(GPIOPin *pin)    { dc_pin_ = pin; }
  void set_reset_pin(GPIOPin *pin) { reset_pin_ = pin; }
  void set_busy_pin(GPIOPin *pin)  { busy_pin_ = pin; }

  void set_full_update_every(uint32_t n) { full_update_every_ = n; }
  void set_invert_colors(bool inv)       { invert_colors_ = inv; }
  void force_full_update()               { force_full_ = true; }

  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;
  void on_safe_shutdown() override;
  void fill(Color color) override;
  display::DisplayType get_display_type() override {
    return display::DisplayType::DISPLAY_TYPE_BINARY;
  }

 protected:
  // Software SPI primitives
  void spi_setup_pins_();
  void spi_write_byte_(uint8_t data);
  void spi_command_(uint8_t cmd);
  void spi_data_(uint8_t data);
  void spi_start_command_();
  void spi_end_command_();
  void spi_start_data_();
  void spi_end_data_();
  void spi_send_sequence_(const uint8_t *seq);

  bool is_busy_();

  // Subclass hooks
  virtual void init_display_() = 0;
  virtual void prepare_update_() = 0;
  virtual void send_data_() = 0;
  virtual int native_width_() const = 0;
  virtual int native_height_() const = 0;

  // DisplayBuffer overrides
  uint32_t get_buffer_length_();
  int get_width_internal() override;
  int get_height_internal() override;
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  bool rotate_pixel_(int x, int y, int *out_x, int *out_y);

  // Pins
  GPIOPin *clk_pin_{nullptr};
  GPIOPin *mosi_pin_{nullptr};
  GPIOPin *cs_pin_{nullptr};
  GPIOPin *dc_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  GPIOPin *busy_pin_{nullptr};

  // State machine
  State state_{State::UNINIT};
  uint32_t state_start_{0};
  bool needs_update_{false};
  bool is_full_update_{false};
  bool force_full_{false};
  uint32_t update_count_{0};
  uint32_t full_update_every_{10};
  bool invert_colors_{false};
};

// ---------------------------------------------------------------------------
// 4.2" — single SSD1683 (400x300)
// ---------------------------------------------------------------------------
class CrowPanelEPaper4P2In : public CrowPanelEPaper {
 protected:
  void init_display_() override;
  void prepare_update_() override;
  void send_data_() override;
  int native_width_() const override  { return WIDTH_4P2; }
  int native_height_() const override { return HEIGHT_4P2; }
};

// ---------------------------------------------------------------------------
// 5.79" — dual SSD1683 cascade (792x272)
// ---------------------------------------------------------------------------
class CrowPanelEPaper5P79In : public CrowPanelEPaper {
 protected:
  void init_display_() override;
  void prepare_update_() override;
  void send_data_() override;
  int native_width_() const override  { return WIDTH_5P79; }
  int native_height_() const override { return HEIGHT_5P79; }
};

}  // namespace crowpanel_epaper
}  // namespace esphome
