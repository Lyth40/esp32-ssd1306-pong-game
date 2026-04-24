#include "oled.h"
#include <string.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS    5
#define PIN_NUM_DC   16
#define PIN_NUM_RST  17

static spi_device_handle_t oled_spi;
static uint8_t oled_buffer[OLED_WIDTH * OLED_PAGES];

static void oled_gpio_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_NUM_DC) | (1ULL << PIN_NUM_RST),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);
}

static esp_err_t oled_spi_init(spi_host_device_t host)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = OLED_WIDTH * OLED_PAGES
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 8 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(host, &devcfg, &oled_spi));

    return ESP_OK;
}

static void oled_reset(void)
{
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
}

static void oled_send_cmd(uint8_t cmd)
{
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd
    };

    gpio_set_level(PIN_NUM_DC, 0);
    ESP_ERROR_CHECK(spi_device_transmit(oled_spi, &t));
}

static void oled_send_data(const uint8_t *data, size_t len)
{
    if (len == 0) return;

    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data
    };

    gpio_set_level(PIN_NUM_DC, 1);
    ESP_ERROR_CHECK(spi_device_transmit(oled_spi, &t));
}

static void oled_configure(void)
{
    oled_send_cmd(0xAE);

    oled_send_cmd(0x20);
    oled_send_cmd(0x00);

    oled_send_cmd(0xB0);

    oled_send_cmd(0xC8);
    oled_send_cmd(0x00);
    oled_send_cmd(0x10);

    oled_send_cmd(0x40);

    oled_send_cmd(0x81);
    oled_send_cmd(0x7F);

    oled_send_cmd(0xA1);
    oled_send_cmd(0xA6);

    oled_send_cmd(0xA8);
    oled_send_cmd(0x3F);

    oled_send_cmd(0xA4);
    oled_send_cmd(0xD3);
    oled_send_cmd(0x00);

    oled_send_cmd(0xD5);
    oled_send_cmd(0x80);

    oled_send_cmd(0xD9);
    oled_send_cmd(0xF1);

    oled_send_cmd(0xDA);
    oled_send_cmd(0x12);

    oled_send_cmd(0xDB);
    oled_send_cmd(0x40);

    oled_send_cmd(0x8D);
    oled_send_cmd(0x14);

    oled_send_cmd(0xAF);
}

esp_err_t oled_init(spi_host_device_t host)
{
    oled_gpio_init();
    ESP_ERROR_CHECK(oled_spi_init(host));
    oled_reset();
    oled_configure();

    memset(oled_buffer, 0x00, sizeof(oled_buffer));
    oled_update();

    return ESP_OK;
}

void oled_clear(void)
{
    memset(oled_buffer, 0x00, sizeof(oled_buffer));
}


void oled_update(void)
{
    oled_send_cmd(0x21);
    oled_send_cmd(0x00);
    oled_send_cmd(0x7F);

    oled_send_cmd(0x22);
    oled_send_cmd(0x00);
    oled_send_cmd(0x07);

    oled_send_data(oled_buffer, sizeof(oled_buffer));
}

void oled_draw_pixel(uint8_t x, uint8_t y, bool on)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) {
        return;
    }

    uint16_t index = x + (y / 8) * OLED_WIDTH;
    uint8_t bit = 1 << (y % 8);

    if (on) {
        oled_buffer[index] |= bit;
    } else {
        oled_buffer[index] &= ~bit;
    }
}

void oled_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
    int err = dx + dy, e2;

    while (true) {
        oled_draw_pixel(x0, y0, on);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void oled_draw_circle(uint8_t xc, uint8_t yc, uint8_t radius, bool on)
{
    int x=  0;
    int y=  radius;
    int d= 3 - 2 * radius;

    // while(y >= x)
    // {
    //     oled_draw_pixel(xc+x, yc+y, on);
    //     oled_draw_pixel(xc-x, yc+y, on);
    //     oled_draw_pixel(xc+x, yc-y, on);
    //     oled_draw_pixel(xc-x, yc-y, on);
    //     oled_draw_pixel(xc+y, yc+x, on);
    //     oled_draw_pixel(xc-y, yc+x, on);
    //     oled_draw_pixel(xc+y, yc-x, on);
    //     oled_draw_pixel(xc-y, yc-x, on);

    //     if (d > 0) { 
    //         y--;
    //         d = d + 4 * (x - y) + 10;
    //     } else {
    //         d = d + 4 * x + 6;
    //     }
    //     x++;
    // }
     while (y >= x)
    {
        oled_draw_line(xc - x, yc + y, xc + x, yc + y, on);
        oled_draw_line(xc - x, yc - y, xc + x, yc - y, on);

        oled_draw_line(xc - y, yc + x, xc + y, yc + x, on);
        oled_draw_line(xc - y, yc - x, xc + y, yc - x, on);

        x++;

        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            d = d + 4 * x + 6;
        }
    }
}

void oled_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on)
{
    for (int i = x; i < x + w; i++)
    {
        for (int j = y; j < y + h; j++)
        {
            oled_draw_pixel(i, j, on);
        }
    }
}