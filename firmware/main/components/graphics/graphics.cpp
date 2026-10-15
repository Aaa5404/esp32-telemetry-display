#include <stdio.h>
#include "graphics.h"
#include "esp_system.h"
#include "graph_buffer.hpp"
#include <string.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160

#define GAS_COLOR 0x07E0   // RGB565 green
#define BRAKE_COLOR 0xF800 // RGB565 red
#define WHITE 0xFFFF
static GraphBuffer gasGraph;
static GraphBuffer brakeGraph;

void inline display_draw_pixel(uint16_t *buffer, uint8_t x, uint8_t y, uint16_t color)
{
    if (x >= 128 || y >= 160)
        return;

    buffer[y * 128 + x] = color;
}

void display_draw_line(uint16_t *buffer,
                       int x0, int y0,
                       int x1, int y1,
                       uint16_t color)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (1)
    {
        display_draw_pixel(buffer, x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}
void update_graph(float gas, float brake)
{
    gasGraph.enqueue(gas);
    brakeGraph.enqueue(brake);
}

/**
 * @brief Draws the gas and brake graphs on the framebuffer.
 *
 * @param framebuffer Pointer to the framebuffer where the graphs will be drawn.
 * @param gas The current gas value (0.0 to 1.0).
 * @param brake The current brake value (0.0 to 1.0).
 */
void drawGraph(uint16_t *framebuffer, float gas, float brake)
{

    update_graph(gas, brake);
    // Clear framebuffer
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
        framebuffer[i] = 0x0000;
    }

    // Draw gas
    for (int x = 1; x < gasGraph.size(); x++)
    {
        int y0 = SCREEN_WIDTH - 1 - (int)(gasGraph.get(x - 1) * (SCREEN_WIDTH - 1));

        int y1 = SCREEN_WIDTH - 1 - (int)(gasGraph.get(x) * (SCREEN_WIDTH - 1));

        display_draw_line(
            framebuffer,
            y0, x - 1,
            y1, x,
            GAS_COLOR);
    }

    // Draw brake
    for (int x = 1; x < brakeGraph.size(); x++)
    {
        int y0 = SCREEN_WIDTH - 1 - (int)(brakeGraph.get(x - 1) * (SCREEN_WIDTH - 1));

        int y1 = SCREEN_WIDTH - 1 - (int)(brakeGraph.get(x) * (SCREEN_WIDTH - 1));

        display_draw_line(
            framebuffer,
            y0, x - 1,
            y1, x,
            BRAKE_COLOR);
    }
}

void draw_test_zigzag(uint16_t *framebuffer)
{
    // Clear framebuffer
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
        framebuffer[i] = 0x0000;
    }

    int x0 = 0;
    int y0 = 0;
    for (int y = 0; y < SCREEN_HEIGHT; y += 5)
    {
        int x1 = (y * 3) % 128;
        int y1 = y; // y has constant steps since it is the time variable (orientation of screen is opposite of manufacturer's datasheet)
        display_draw_line(framebuffer, x0, y0, x1, y1, GAS_COLOR);
        x0 = x1;
        y0 = y1;
    }
}

static inline int value_to_col(float v)
{
    if (!(v > 0.0f))
        return 0; // also catches NaN
    if (v > 1.0f)
        v = 1.0f;
    return (int)(v * (SCREEN_WIDTH - 1) + 0.5f); // 0.0 -> 0, 1.0 -> 127
}

// Draw the span between two columns, inclusive, so the trace stays connected
// and a pixel is still drawn when the value doesn't change.
void display_draw_run(uint16_t *colbuffer, int from, int to, uint16_t color)
{
    const int lo = from < to ? from : to;
    const int hi = from < to ? to : from;
    for (int i = lo; i <= hi; i++)
        display_draw_pixel(colbuffer, i, 0, color);
}

void draw_column(uint16_t *colbuffer, float gas, float brake)
{
    memset(colbuffer, 0, SCREEN_WIDTH * sizeof(uint16_t));

    // Read the previous samples BEFORE update_graph() pushes the new ones.
    const int gas_prev = value_to_col(gasGraph.get_previous());
    const int brake_prev = value_to_col(brakeGraph.get_previous());

    update_graph(gas, brake);

    // Brake is drawn second, so it wins where the two overlap.
    display_draw_run(colbuffer, gas_prev, value_to_col(gas), GAS_COLOR);
    display_draw_run(colbuffer, brake_prev, value_to_col(brake), BRAKE_COLOR);
}