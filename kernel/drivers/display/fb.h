/**
 * Toda OS — Framebuffer Display Driver Header
 * Mobile Screen Graphics Engine (32-bit ARGB)
 *
 * Target: QEMU ARM64 Mobile Framebuffer
 * Resolution: 480 x 800 (Portrait Mobile Aspect Ratio)
 *
 * Copyright (c) Toda OS Project
 */

#ifndef TODA_FB_H
#define TODA_FB_H

#include "../../core/kernel.h"

/* Mobile Screen Dimensions */
#define FB_WIDTH        480
#define FB_HEIGHT       800
#define FB_BPP          32              /* 32 bits per pixel (ARGB8888) */
#define FB_STRIDE       (FB_WIDTH * 4)  /* Bytes per scanline */
#define FB_SIZE         (FB_WIDTH * FB_HEIGHT * 4)

/* Standard Mobile Color Palette (ARGB 32-bit) */
#define COLOR_BLACK         0xFF000000
#define COLOR_WHITE         0xFFFFFFFF
#define COLOR_TRANSPARENT   0x00000000

/* Toda OS Modern Mobile UI Theme */
#define COLOR_BG_TOP        0xFF0D1117   /* Deep Midnight Blue/Grey */
#define COLOR_BG_BOTTOM     0xFF161B22   /* Dark Slate */
#define COLOR_ACCENT_BLUE   0xFF1F6FEB   /* iOS/Android Electric Blue */
#define COLOR_ACCENT_GREEN  0xFF2EA043   /* Phone Call Emerald Green */
#define COLOR_ACCENT_ORANGE 0xFFDB6D28   /* Toda Store Orange */
#define COLOR_ACCENT_PURPLE 0xFF8957E5   /* Camera Violet */
#define COLOR_ACCENT_RED    0xFFDA3633   /* End Call / Alert Red */
#define COLOR_GLASS         0x8821262D   /* Translucent Frosted Glass */
#define COLOR_DOCK          0xAA161B22   /* Translucent Dock Bar */
#define COLOR_TEXT_MUTED    0xFF8B949E   /* Subtitle Grey */
#define COLOR_TEXT_WHITE    0xFFF0F6FC   /* Bright White Text */

/* Driver Initialization */
result_t fb_init(void);
uint32_t* fb_get_buffer(void);

/* 2D Graphics Primitives */
void fb_clear(uint32_t color);
void fb_draw_pixel(int x, int y, uint32_t color);
void fb_draw_rect(int x, int y, int w, int h, uint32_t color);
void fb_draw_rounded_rect(int x, int y, int w, int h, int radius, uint32_t color);
void fb_draw_gradient_v(int x, int y, int w, int h, uint32_t top_col, uint32_t bot_col);
void fb_draw_circle(int cx, int cy, int radius, uint32_t color);
void fb_draw_circle_outline(int cx, int cy, int radius, int thickness, uint32_t color);

/* Typography & Text */
void fb_draw_char(int x, int y, char c, uint32_t fg, uint32_t bg);
void fb_draw_string(int x, int y, const char* str, uint32_t fg, uint32_t bg);
void fb_draw_string_large(int x, int y, const char* str, uint32_t fg); /* 2x Scale */
void fb_draw_string_huge(int x, int y, const char* str, uint32_t fg);  /* 4x Scale for Clock */

#endif /* TODA_FB_H */

