/**
 * Toda OS — Mobile Smartphone Graphical User Interface Implementation
 *
 * Implements a modern mobile smartphone interface:
 * - Top Status Bar with 5G, Wi-Fi, battery % and clock
 * - Rounded squircle app icons
 * - Frosted glass dock
 * - Interactive dialer
 *
 * Copyright (c) Toda OS Project
 */

#include "gui.h"
#include "../drivers/display/fb.h"
#include "../drivers/uart/uart.h"

static gui_screen_t current_screen = GUI_SCREEN_HOMESCREEN;

void gui_init(void) {
    uart_print("[GUI]      Starting Toda Mobile UI Compositor...\n");
    fb_init();
    gui_render();
    uart_print("[GUI]      Mobile Phone UI rendered successfully! 📱\n");
}

void gui_set_screen(gui_screen_t screen) {
    current_screen = screen;
    gui_render();
}

/**
 * Top Status Bar (Height: 32px)
 * Left: Clock ("10:45")
 * Center: Toda OS Speaker / Camera notch
 * Right: Signal Bars, 5G badge, Battery Icon
 */
void gui_render_statusbar(const char* time_str, int battery_percent) {
    /* Status bar background (sleek translucent black) */
    fb_draw_rect(0, 0, FB_WIDTH, 32, 0xCC000000);

    /* Time on top-left */
    fb_draw_string(20, 8, time_str, COLOR_WHITE, COLOR_TRANSPARENT);

    /* Center Camera / Dynamic Island Notch */
    fb_draw_rounded_rect(FB_WIDTH / 2 - 40, 6, 80, 16, 8, 0xFF000000);
    fb_draw_circle(FB_WIDTH / 2 + 20, 14, 3, 0xFF1C2128); /* Camera lens */

    /* Right: Cellular Signal Bars (4 ascending bars) */
    int sig_x = FB_WIDTH - 120;
    fb_draw_rect(sig_x,      20, 3, 4, COLOR_WHITE);
    fb_draw_rect(sig_x + 5,  17, 3, 7, COLOR_WHITE);
    fb_draw_rect(sig_x + 10, 14, 3, 10, COLOR_WHITE);
    fb_draw_rect(sig_x + 15, 11, 3, 13, COLOR_WHITE);

    /* "5G" Text Badge */
    fb_draw_string(sig_x + 24, 10, "5G", 0xFF58A6FF, COLOR_TRANSPARENT);

    /* Battery Icon Container */
    int bat_x = FB_WIDTH - 55;
    fb_draw_rounded_rect(bat_x, 10, 26, 13, 2, COLOR_TEXT_MUTED);
    fb_draw_rect(bat_x + 26, 13, 2, 6, COLOR_TEXT_MUTED); /* Battery nipple */

    /* Green battery charge level */
    int fill_w = (22 * battery_percent) / 100;
    if (fill_w > 22) fill_w = 22;
    uint32_t bat_color = battery_percent > 20 ? COLOR_ACCENT_GREEN : COLOR_ACCENT_RED;
    fb_draw_rect(bat_x + 2, 12, fill_w, 9, bat_color);
}

/**
 * Modern Deep Gradient Mobile Wallpaper
 */
void gui_render_wallpaper(void) {
    /* Rich purple-blue vertical gradient */
    fb_draw_gradient_v(0, 0, FB_WIDTH, FB_HEIGHT, 0xFF160B30, 0xFF090D16);
}

/**
 * Draws a rounded squircle App Icon with icon glyph and label
 */
void gui_draw_app_icon(int x, int y, const char* name, uint32_t bg_color, char glyph) {
    int icon_size = 64;

    /* Drop shadow effect */
    fb_draw_rounded_rect(x + 2, y + 4, icon_size, icon_size, 16, 0x44000000);

    /* Main App Squircle Body */
    fb_draw_rounded_rect(x, y, icon_size, icon_size, 16, bg_color);

    /* Inner subtle gradient/highlight */
    fb_draw_rounded_rect(x + 2, y + 2, icon_size - 4, (icon_size / 2), 14, 0x22FFFFFF);

    /* Draw large Glyph in the center of the icon */
    char glyph_str[2] = {glyph, '\0'};
    fb_draw_string_large(x + 24, y + 16, glyph_str, COLOR_WHITE);

    /* App Name Label under icon */
    int label_len = 0;
    const char* p = name;
    while (*p++) label_len++;
    int text_x = x + (icon_size / 2) - (label_len * 8 / 2);
    fb_draw_string(text_x, y + icon_size + 8, name, COLOR_TEXT_WHITE, COLOR_TRANSPARENT);
}

/**
 * Mobile Home Screen
 */
void gui_render_homescreen(void) {
    gui_render_wallpaper();

    /* Status bar */
    gui_render_statusbar("10:45", 98);

    /* Top Toda OS Search / Brand Pill */
    int pill_w = 400;
    int pill_x = (FB_WIDTH - pill_w) / 2;
    fb_draw_rounded_rect(pill_x, 50, pill_w, 44, 22, 0x33FFFFFF);
    fb_draw_string(pill_x + 20, 64, "Search Toda OS, Apps & Web...", COLOR_TEXT_MUTED, COLOR_TRANSPARENT);

    /* App Grid (4 Columns x 2 Rows) */
    int start_x = 36;
    int col_gap = 108;
    int row1_y  = 130;
    int row2_y  = 240;

    /* Row 1 */
    gui_draw_app_icon(start_x + col_gap * 0, row1_y, "Phone",    COLOR_ACCENT_GREEN,  'P');
    gui_draw_app_icon(start_x + col_gap * 1, row1_y, "Messages", COLOR_ACCENT_BLUE,   'M');
    gui_draw_app_icon(start_x + col_gap * 2, row1_y, "Settings", 0xFF484F58,          'S');
    gui_draw_app_icon(start_x + col_gap * 3, row1_y, "Store",    COLOR_ACCENT_ORANGE, 'T');

    /* Row 2 */
    gui_draw_app_icon(start_x + col_gap * 0, row2_y, "Camera",   COLOR_ACCENT_PURPLE, 'C');
    gui_draw_app_icon(start_x + col_gap * 1, row2_y, "Browser",  0xFF1B7C84,          'B');
    gui_draw_app_icon(start_x + col_gap * 2, row2_y, "Music",    0xFFD03B7C,          'A');
    gui_draw_app_icon(start_x + col_gap * 3, row2_y, "Files",    0xFFB08800,          'F');

    /* Center Toda OS Quick Status Card */
    int card_y = 360;
    int card_w = 420;
    int card_x = (FB_WIDTH - card_w) / 2;
    fb_draw_rounded_rect(card_x, card_y, card_w, 140, 20, 0x4421262D);
    fb_draw_string_large(card_x + 20, card_y + 18, "TODA OS GENESIS", 0xFF58A6FF);
    fb_draw_string(card_x + 20, card_y + 56, "Hardware: Qualcomm Snapdragon Ready", COLOR_WHITE, COLOR_TRANSPARENT);
    fb_draw_string(card_x + 20, card_y + 76, "Security: Verified Boot (Locked)", COLOR_ACCENT_GREEN, COLOR_TRANSPARENT);
    fb_draw_string(card_x + 20, card_y + 96, "Kernel: Bare-Metal ARM64 (No Linux)", COLOR_TEXT_MUTED, COLOR_TRANSPARENT);
    fb_draw_string(card_x + 20, card_y + 116, "Status: System Running Normally", 0xFF7EE787, COLOR_TRANSPARENT);

    /* Bottom Frosted Glass Dock (Holds 4 Main Apps) */
    int dock_h = 96;
    int dock_y = FB_HEIGHT - dock_h - 28;
    int dock_w = 440;
    int dock_x = (FB_WIDTH - dock_w) / 2;
    fb_draw_rounded_rect(dock_x, dock_y, dock_w, dock_h, 32, 0x55161B22);

    /* 4 Primary Dock Apps */
    int dock_app_x = dock_x + 24;
    int dock_gap   = 102;
    int icon_y     = dock_y + 16;
    gui_draw_app_icon(dock_app_x + dock_gap * 0, icon_y, "", COLOR_ACCENT_GREEN,  'P');
    gui_draw_app_icon(dock_app_x + dock_gap * 1, icon_y, "", COLOR_ACCENT_BLUE,   'M');
    gui_draw_app_icon(dock_app_x + dock_gap * 2, icon_y, "", 0xFF1B7C84,          'B');
    gui_draw_app_icon(dock_app_x + dock_gap * 3, icon_y, "", COLOR_ACCENT_PURPLE, 'C');

    /* Modern Mobile Home Swipe Indicator Bar */
    int bar_w = 140;
    int bar_h = 5;
    int bar_x = (FB_WIDTH - bar_w) / 2;
    int bar_y = FB_HEIGHT - 12;
    fb_draw_rounded_rect(bar_x, bar_y, bar_w, bar_h, 2, 0xAAFFFFFF);
}

/**
 * Interactive Phone Dialer Screen
 */
void gui_render_dialer(const char* dialed_number) {
    gui_render_wallpaper();
    gui_render_statusbar("10:45", 98);

    /* Back / Home arrow */
    fb_draw_string(20, 50, "< Home", 0xFF58A6FF, COLOR_TRANSPARENT);

    /* Large Dialed Number Display Area */
    int num_box_y = 90;
    fb_draw_rect(0, num_box_y, FB_WIDTH, 80, 0x33000000);
    fb_draw_string_large(40, num_box_y + 26, dialed_number, COLOR_WHITE);

    /* 3x4 Circular Keypad */
    static const char* const keys[12] = {
        "1", "2", "3",
        "4", "5", "6",
        "7", "8", "9",
        "*", "0", "#"
    };

    int key_r = 34;
    int start_kx = 90;
    int start_ky = 240;
    int k_gap_x  = 150;
    int k_gap_y  = 90;

    for (int i = 0; i < 12; i++) {
        int row = i / 3;
        int col = i % 3;
        int cx = start_kx + col * k_gap_x;
        int cy = start_ky + row * k_gap_y;

        /* Circle button */
        fb_draw_circle(cx, cy, key_r, 0x33FFFFFF);

        /* Number character */
        fb_draw_string_large(cx - 8, cy - 14, keys[i], COLOR_WHITE);
    }

    /* Big Green "CALL" Button */
    int call_y = 630;
    fb_draw_circle(FB_WIDTH / 2, call_y, 42, COLOR_ACCENT_GREEN);
    fb_draw_string_large(FB_WIDTH / 2 - 16, call_y - 14, "C", COLOR_WHITE);

    /* Bottom Home Indicator Bar */
    int bar_w = 140;
    fb_draw_rounded_rect((FB_WIDTH - bar_w) / 2, FB_HEIGHT - 12, bar_w, 5, 2, 0xAAFFFFFF);
}

/**
 * Master Render Routine
 */
void gui_render(void) {
    switch (current_screen) {
        case GUI_SCREEN_HOMESCREEN:
            gui_render_homescreen();
            break;
        case GUI_SCREEN_DIALER:
            gui_render_dialer("+91 98765 43210");
            break;
        default:
            gui_render_homescreen();
            break;
    }
}

