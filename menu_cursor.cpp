#include "menu_cursor.h"

MenuCursor::MenuCursor(LiquidCrystal *lcd, uint8_t char_index)
{
    _lcd = lcd;
    _is_blinking = false;
    _last_x = 0;
    _last_y = 0;
    _blink_ticks_count = 0;
    _is_blink_hide = false;
    _char_index = char_index;
}

void MenuCursor::start_blinking()
{
    _is_blinking = true;
}

void MenuCursor::stop_blinking()
{
    _is_blink_hide = false;
    _blink_ticks_count = 0;
    _is_blinking = false;
}

bool MenuCursor::is_blinking()
{
    return _is_blinking;
}

void MenuCursor::draw(uint8_t x, uint8_t y)
{
    if (_last_x != x || _last_y != y)
    {
        clear_at(_last_x, _last_y);
        _last_x = x;
        _last_y = y;
    }

    _lcd->setCursor(x, y);

    if (_is_blinking)
    {
        _blink_ticks_count++;
        if (_blink_ticks_count > CURSOR_BLINK_TICKS)
        {
            _blink_ticks_count = 0;
            _is_blink_hide = !_is_blink_hide;
        }
        if (_is_blink_hide)
        {
            clear_at(x, y);
        }
        else
        {
            _lcd->write(_char_index);
        }
    }
    else
    {
        _lcd->write(_char_index);
    }
}

void MenuCursor::redraw_last() {
  draw(_last_x, _last_y);
}

void MenuCursor::clear_at(uint8_t x, uint8_t y)
{
    _lcd->setCursor(x, y);
    _lcd->print(" ");
}