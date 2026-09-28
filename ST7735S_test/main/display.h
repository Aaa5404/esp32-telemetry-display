#ifndef DISPLAY_H
#define DISPLAY_H
#include <stdint.h>
#include "esp_system.h"

void display_spi_init(void);
void display_gpio_init(void);
void display_reset(void);
void display_write_command(uint8_t command);
void display_write_data(const uint8_t *data, size_t length);

void lcd_init(void);
void display_set_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void display_fill_screen(uint16_t color);
void display_draw_buffer(const uint16_t *buffer);

#endif