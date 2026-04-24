#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "driver/spi_master.h"
#include "esp_err.h"

#define OLED_WIDTH   128
#define OLED_HEIGHT   64
#define OLED_PAGES   (OLED_HEIGHT / 8)

esp_err_t oled_init(spi_host_device_t host);
void oled_clear(void);
void oled_update(void);
void oled_draw_pixel(uint8_t x, uint8_t y, bool on);
void oled_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on);
void oled_draw_circle (uint8_t xc, uint8_t yc, uint8_t radius, bool on);
void oled_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on);