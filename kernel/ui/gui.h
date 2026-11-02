/**
 * Toda OS — Mobile Smartphone Graphical User Interface Header
 *
 * Full Touch & Graphical UI Shell:
 * - Status Bar (Clock, 5G Signal, Battery with level)
 * - Mobile Wallpaper (Deep Gradient)
 * - App Grid (Phone, SMS, Settings, Store, Camera, Music, Files, Browser)
 * - Glass Bottom Dock & Home Swipe Bar
 * - Interactive Phone Dialer Screen
 *
 * Copyright (c) Toda OS Project
 */

#ifndef TODA_GUI_H
#define TODA_GUI_H

#include "../core/kernel.h"

/* UI Screens */
typedef enum {
    GUI_SCREEN_LOCKSCREEN = 0,
    GUI_SCREEN_HOMESCREEN,
    GUI_SCREEN_DIALER,
    GUI_SCREEN_SETTINGS
} gui_screen_t;

/* Initialization & Main Loop */
void gui_init(void);
void gui_render(void);
void gui_set_screen(gui_screen_t screen);

/* Screen Renderers */
void gui_render_statusbar(const char* time_str, int battery_percent);
void gui_render_wallpaper(void);
void gui_render_lockscreen(const char* time_str);
void gui_render_homescreen(void);
void gui_render_dialer(const char* dialed_number);

/* App Icon Drawer */
void gui_draw_app_icon(int x, int y, const char* name, uint32_t bg_color, char glyph);

#endif /* TODA_GUI_H */

