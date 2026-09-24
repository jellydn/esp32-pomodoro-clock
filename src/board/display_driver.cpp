#include "display_driver.h"

#include <Arduino.h>

#include "board_config.h"

namespace board {

namespace {
constexpr std::uint8_t kBacklightChannel = 0;
constexpr std::uint32_t kBacklightFrequency = 5000;
constexpr std::uint8_t kBacklightResolution = 8;
}  // namespace

bool DisplayDriver::begin() {
  ledcSetup(kBacklightChannel, kBacklightFrequency, kBacklightResolution);
  ledcAttachPin(kBacklight, kBacklightChannel);
  setBrightness(100);
  return gfx_.begin();
}

void DisplayDriver::showBootScreen() {
  gfx_.fillScreen(BLACK);
  gfx_.setTextColor(WHITE);
  gfx_.setTextSize(3);
  gfx_.setCursor(42, 96);
  gfx_.println("JC4827W543");
  gfx_.setTextSize(2);
  gfx_.setCursor(72, 142);
  gfx_.println("Display initialized");
}

void DisplayDriver::setBrightness(std::uint8_t percent) {
  const std::uint8_t bounded = percent > 100 ? 100 : percent;
  ledcWrite(kBacklightChannel, static_cast<std::uint8_t>(bounded * 255U / 100U));
}

void DisplayDriver::registerLvgl() {
  lv_disp_draw_buf_init(&drawBuffer_, pixels_, nullptr, 480 * 20);
  lv_disp_drv_init(&displayDriver_);
  displayDriver_.hor_res = kWidth;
  displayDriver_.ver_res = kHeight;
  displayDriver_.flush_cb = flush;
  displayDriver_.draw_buf = &drawBuffer_;
  displayDriver_.user_data = this;
  lv_disp_drv_register(&displayDriver_);
}

void DisplayDriver::flush(lv_disp_drv_t* driver, const lv_area_t* area, lv_color_t* pixels) {
  auto* self = static_cast<DisplayDriver*>(driver->user_data);
  const std::int32_t width = area->x2 - area->x1 + 1;
  const std::int32_t height = area->y2 - area->y1 + 1;
  self->gfx_.draw16bitRGBBitmap(area->x1, area->y1,
                               reinterpret_cast<std::uint16_t*>(pixels), width, height);
  lv_disp_flush_ready(driver);
}

}  // namespace board
