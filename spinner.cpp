#include "spinner.h"

Spinner::Spinner(LiquidCrystal *lcd, uint8_t first_sprite_index, uint8_t last_sprite_index)
{
  _lcd = lcd;
  _first_sprite_index = first_sprite_index;
  _current_sprite_index = first_sprite_index;
  _last_sprite_index = last_sprite_index;
  _is_animated = false;
  _spin_forward = true;
  _ticks_count = 0;
}

void Spinner::clear()
{
  _lcd->setCursor(0, 15);
  _lcd->write(" ");
}

void Spinner::draw()
{
  if (_is_animated)
  {
    _ticks_count++;
    if (_ticks_count > SPINNER_FRAME_TICKS)
    {
      _ticks_count = 0;
      if (_spin_forward)
      {
        _current_sprite_index = _next_sprite_index();
      }
      else
      {
        _current_sprite_index = _prev_sprite_index();
      }
    }
  }
  _lcd->setCursor(15, 0);
  _lcd->write(_current_sprite_index);
}

void Spinner::reset()
{
  stop_animation();
  _current_sprite_index = _first_sprite_index;
  _spin_forward = true;
}

void Spinner::start_animation()
{
  _is_animated = true;
}
void Spinner::stop_animation()
{
  _is_animated = false;
}

bool Spinner::is_animated()
{
  return _is_animated;
}

void Spinner::turn_rotation()
{
  _spin_forward = !_spin_forward;
}

uint8_t Spinner::_next_sprite_index()
{
  if (_current_sprite_index + 1 > _last_sprite_index)
  {
    return _first_sprite_index;
  }

  return _current_sprite_index + 1;
}

uint8_t Spinner::_prev_sprite_index()
{
  if (_current_sprite_index == 0 || _current_sprite_index - 1 < _first_sprite_index)
  {
    return _last_sprite_index;
  }

  return _current_sprite_index - 1;
}
