#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "esp_log.h"
#include "esp_err.h"

#include "components/st7735.h"
// =========================
// Pin definitions
// =========================
#define PIN_CS 10   // FSPICS0
#define PIN_MOSI 11 // FSPID
#define PIN_SCLK 12 // FSPICLK

#define PIN_DC 9 // DC = 0  -> command  DC = 1  -> data
#define PIN_RES 8
#define PIN_BL 7
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160
static spi_device_handle_t display;

typedef struct
{
    uint8_t cmd;
    const uint8_t *data;
    uint8_t data_len;
    uint16_t delay_ms;
} lcd_init_cmd_t;

DRAM_ATTR static const lcd_init_cmd_t init_commands[] = {

    // 7735R init, part 1 (red or green tab)

    //  1: Software reset, 0 args, w/delay
    //     150 ms delay
    {ST77XX_SWRESET, NULL, 0, 150},

    //  2: Out of sleep mode, 0 args, w/delay
    //     500 ms delay
    {ST77XX_SLPOUT, NULL, 0, 500},

    //  3: Framerate ctrl - normal mode, 3 arg:
    //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    {ST7735_FRMCTR1, (uint8_t[]){0x01, 0x2C, 0x2D}, 3, 0},

    //  4: Framerate ctrl - idle mode, 3 args:
    //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    {ST7735_FRMCTR2, (uint8_t[]){0x01, 0x2C, 0x2D}, 3, 0},

    //  5: Framerate - partial mode, 6 args:
    //     Dot inversion mode
    //     Line inversion mode
    {ST7735_FRMCTR3, (uint8_t[]){0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D}, 6, 0},

    //  6: Display inversion ctrl, 1 arg:
    //     No inversion
    {ST7735_INVCTR, (uint8_t[]){0x07}, 1, 0},

    //  7: Power control, 3 args, no delay:
    //     -4.6V
    //     AUTO mode
    {ST7735_PWCTR1, (uint8_t[]){0xA2, 0x02, 0x84}, 3, 0},

    //  8: Power control, 1 arg, no delay:
    //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
    {ST7735_PWCTR2, (uint8_t[]){0xC5}, 1, 0},

    //  9: Power control, 2 args, no delay:
    //     Opamp current small
    //     Boost frequency
    {ST7735_PWCTR3, (uint8_t[]){0x0A, 0x00}, 2, 0},

    // 10: Power control, 2 args, no delay:
    //     BCLK/2,
    //     opamp current small & medium low
    {ST7735_PWCTR4, (uint8_t[]){0x8A, 0x2A}, 2, 0},

    // 11: Power control, 2 args, no delay:
    {ST7735_PWCTR5, (uint8_t[]){0x8A, 0xEE}, 2, 0},

    // 12: Power control, 1 arg, no delay:
    {ST7735_VMCTR1, (uint8_t[]){0x0E}, 1, 0},

    // 13: Don't invert display, no args
    {ST77XX_INVOFF, NULL, 0, 0},

    // 14: Mem access ctl (directions), 1 arg:
    //     row/col addr, bottom-top refresh
    {ST77XX_MADCTL, (uint8_t[]){0xC0}, 1, 0}, // My display is rgb not bgr bit 3 set to 0.

    // 15: set color mode, 1 arg, no delay:
    //     16-bit color
    {ST77XX_COLMOD, (uint8_t[]){0x05}, 1, 0},
    // 7735R init, part 2 (green tab only)

    //  1: Column addr set, 4 args, no delay:
    //     XSTART = 0
    //     XEND = 127
    {ST77XX_CASET, (uint8_t[]){0x00, 0x02, 0x00, 0x7F + 0x02}, 4, 0},

    //  2: Row addr set, 4 args, no delay:
    //     XSTART = 0
    //     XEND = 159
    {ST77XX_RASET, (uint8_t[]){0x00, 0x01, 0x00, 0x9F + 0x01}, 4, 0},

    // 7735R init, part 3 (red or green tab)

    //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
    //     (Not entirely necessary, but provides
    //      accurate colors)
    {ST7735_GMCTRP1, (uint8_t[]){0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10}, 16, 0},

    //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
    //     (Not entirely necessary, but provides
    //      accurate colors)
    {ST7735_GMCTRN1, (uint8_t[]){0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10}, 16, 0},

    //  3: Normal display on, no args, w/delay
    //     10 ms delay
    {ST77XX_NORON, NULL, 0, 10},

    //  4: Main screen turn on, no args w/delay
    //     100 ms delay
    {ST77XX_DISPON, NULL, 0, 100},
};

void display_spi_init(void)
{
    // Initialize spi pins
    spi_bus_config_t bus_config = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 128 * 160 * 2,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t device_config = {
        .clock_speed_hz = 20 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = PIN_CS,
        .queue_size = 7,
    };
    // Attach LCD to spi bus
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &device_config, &display));
}
void display_gpio_init(void)
{
    gpio_config_t io_config = {
        .pin_bit_mask =
            (1ULL << PIN_DC) |
            (1ULL << PIN_RES) |
            (1ULL << PIN_BL),

        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_config));
}

void display_reset(void)
{
    gpio_set_level(PIN_RES, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    gpio_set_level(PIN_RES, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
}
void display_write_command(uint8_t command)
{
    gpio_set_level(PIN_DC, 0);

    spi_transaction_t transaction = {
        .length = 8,
        .tx_buffer = &command,
    };

    ESP_ERROR_CHECK(spi_device_transmit(display, &transaction));
}

void display_write_data(const uint8_t *data, size_t length)
{
    gpio_set_level(PIN_DC, 1);

    spi_transaction_t transaction = {
        .length = length * 8,
        .tx_buffer = data,
    };

    ESP_ERROR_CHECK(spi_device_transmit(display, &transaction));
}

void lcd_init(void)
{
    // Execute all the commands in the init_commands array

    size_t count = sizeof(init_commands) / sizeof(init_commands[0]);

    for (size_t i = 0; i < count; i++)
    {
        display_write_command(init_commands[i].cmd);

        if (init_commands[i].data_len > 0)
        {
            display_write_data(
                init_commands[i].data,
                init_commands[i].data_len);
        }

        if (init_commands[i].delay_ms > 0)
        {
            vTaskDelay(pdMS_TO_TICKS(init_commands[i].delay_ms));
        }
    }
}
void display_set_window(uint8_t x0, uint8_t y0,
                        uint8_t x1, uint8_t y1)
{
    // Column address set
    display_write_command(ST77XX_CASET);

    uint8_t column_data[] = {
        0x00, x0,
        0x00, x1};

    display_write_data(column_data, 4);

    // Row address set
    display_write_command(ST77XX_RASET);

    uint8_t row_data[] = {
        0x00, y0,
        0x00, y1};

    display_write_data(row_data, 4);

    // Start writing to display memory
    display_write_command(ST77XX_RAMWR);
}

void display_fill_screen(uint16_t color)
{
    display_set_window(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);

    // 128 * 160 pixels
    // 2 bytes per pixel (RGB565)
    static uint8_t buffer[128 * 2];

    for (int x = 0; x < 128; x++)
    {
        buffer[x * 2] = color >> 8;
        buffer[x * 2 + 1] = color & 0xFF;
    }

    for (int y = 0; y < 160; y++)
    {
        display_write_data(buffer, sizeof(buffer));
    }
}
void display_draw_buffer(const uint16_t *buffer)
{
    display_set_window(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);

    const size_t total_bytes = SCREEN_WIDTH * SCREEN_HEIGHT * 2;
    const size_t chunk_bytes = total_bytes / 2; // 20,480 bytes, under the 32 KB limit

    display_write_data((const uint8_t *)buffer, chunk_bytes);
    display_write_data((const uint8_t *)buffer + chunk_bytes, total_bytes - chunk_bytes);
}

void display_init(void)
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
}