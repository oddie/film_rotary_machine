#include <LiquidCrystal.h>
#include "custom_chars.h"

#ifndef Spinner_h
#define Spinner_h

#define SPINNER_FRAME_TICKS 10

class Spinner
{

public:
    Spinner(LiquidCrystal *lcd, uint8_t first_sprite_index, uint8_t last_sprite_index);

    void clear();
    void draw();
    void start_animation();
    void stop_animation();
    void turn_rotation();
    bool is_animated();
    void reset();

private:
    LiquidCrystal *_lcd;
    uint8_t _current_frame;
    uint8_t _first_sprite_index;
    uint8_t _last_sprite_index;
    uint8_t _current_sprite_index;
    uint8_t _ticks_count;
    bool _is_animated;
    bool _spin_forward;

    uint8_t _next_sprite_index();
    uint8_t _prev_sprite_index();
};

#endif