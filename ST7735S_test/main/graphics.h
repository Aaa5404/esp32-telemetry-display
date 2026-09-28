#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    // expose high level drawing functions only
    void drawGraph(uint16_t *framebuffer, float gas, float brake);
    void draw_test_zigzag(uint16_t *framebuffer);

#ifdef __cplusplus
}
#endif

#endif