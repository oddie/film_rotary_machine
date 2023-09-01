#include <LiquidCrystal.h>

#ifndef Menucursor_h
#define Menucursor_h

#define CURSOR_BLINK_TICKS 30

class MenuCursor
{

public:
    MenuCursor(LiquidCrystal *lcd, uint8_t char_index);
    void draw(uint8_t x, uint8_t y);
    void redraw_last();
    void start_blinking();
    void stop_blinking();
    bool is_blinking();
    void clear_at(uint8_t x, uint8_t y);

private : 
    LiquidCrystal *_lcd;
    bool _is_blinking;
    uint8_t _char_index;
    uint8_t _last_x;
    uint8_t _last_y;
    uint8_t _blink_ticks_count;
    bool _is_blink_hide;
};

#endif