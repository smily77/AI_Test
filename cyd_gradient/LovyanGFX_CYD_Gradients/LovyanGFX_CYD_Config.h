#pragma once

#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
public:
  LGFX() {
    auto bus_cfg = _bus_instance.config();
    bus_cfg.spi_host = SPI2_HOST;
    bus_cfg.spi_mode = 0;
    bus_cfg.freq_write = 40000000;
    bus_cfg.freq_read = 16000000;
    bus_cfg.spi_3wire = false;
    bus_cfg.use_lock = true;
    bus_cfg.dma_channel = 1;
    bus_cfg.pin_sclk = 14;
    bus_cfg.pin_mosi = 13;
    bus_cfg.pin_miso = 12;
    bus_cfg.pin_dc = 2;
    bus_cfg.pin_cs = 15;
    _bus_instance.config(bus_cfg);
    _panel_instance.setBus(&_bus_instance);

    auto panel_cfg = _panel_instance.config();
    panel_cfg.pin_cs = 15;
    panel_cfg.pin_rst = 4;
    panel_cfg.pin_busy = -1;
    panel_cfg.memory_width = 240;
    panel_cfg.memory_height = 320;
    panel_cfg.panel_width = 240;
    panel_cfg.panel_height = 320;
    panel_cfg.offset_x = 0;
    panel_cfg.offset_y = 0;
    panel_cfg.offset_rotation = 0;
    panel_cfg.dummy_read_pixel = 8;
    panel_cfg.dummy_read_bits = 1;
    panel_cfg.readable = true;
    panel_cfg.invert = false;
    panel_cfg.rgb_order = false;
    panel_cfg.dlen_16bit = false;
    panel_cfg.bus_shared = false;
    _panel_instance.config(panel_cfg);

    auto light_cfg = _light_instance.config();
    light_cfg.pin_bl = 21;
    light_cfg.invert = false;
    light_cfg.freq = 12000;
    light_cfg.pwm_channel = 7;
    _light_instance.config(light_cfg);
    _panel_instance.setLight(&_light_instance);

    setPanel(&_panel_instance);
  }

private:
  lgfx::Bus_SPI _bus_instance;
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Light_PWM _light_instance;
};
