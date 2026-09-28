#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "st7735.h"
#include "graphics.h"
#include "display.h"

#define PIN_CS 10   // FSPICS0
#define PIN_MOSI 11 // FSPID
#define PIN_SCLK 12 // FSPICLK

#define PIN_DC 9 // DC = 0  -> command  DC = 1  -> data
#define PIN_RES 8
#define PIN_BL 7
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160
static uint16_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; // 2 bytes for color

void app_main(void)
{

    display_spi_init();
    display_gpio_init();
    display_reset();

    lcd_init();
    gpio_set_level(PIN_BL, 1);

    vTaskDelay(pdMS_TO_TICKS(500));
    display_fill_screen(RED);
    vTaskDelay(pdMS_TO_TICKS(500));
    display_fill_screen(GREEN);
    vTaskDelay(pdMS_TO_TICKS(500));
    display_fill_screen(BLUE);

    draw_test_zigzag(framebuffer);

    display_draw_buffer(framebuffer);
    for (int i = 0; i < 100; i++)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        drawGraph(framebuffer, (float)i / 100.0f, (float)(100 - i) / 100.0f);
        display_draw_buffer(framebuffer);
        if (i == 90)
        {
            i = 0;
        }
    }
}