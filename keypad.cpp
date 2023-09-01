#include <Arduino.h>
#include "keypad.h"

Keypad::Keypad(unsigned short keypad_pin_in)
{
    _keypad_pin_in = keypad_pin_in;
}

void Keypad::init()
{
    pinMode(_keypad_pin_in, INPUT);
}

int Keypad::read_buttons()
{
    int analog_key_value = analogRead(_keypad_pin_in);
    _ticks_count++;

    if (_ticks_count > KEYPAD_LONGPRESS_TICK_THRESHOLD)
    {
        // prevent integer overflow
        _ticks_count = KEYPAD_LONGPRESS_TICK_THRESHOLD;
    }

    if (analog_key_value < 50)
        return _ticks_count >= KEYPAD_LONGPRESS_TICK_THRESHOLD ? KEYPAD_BTN_LONG_RIGHT : KEYPAD_BTN_RIGHT;

    if (analog_key_value < 250)
        return _ticks_count >= KEYPAD_LONGPRESS_TICK_THRESHOLD ? KEYPAD_BTN_LONG_UP : KEYPAD_BTN_UP;

    if (analog_key_value < 450)
        return _ticks_count >= KEYPAD_LONGPRESS_TICK_THRESHOLD ? KEYPAD_BTN_LONG_DOWN : KEYPAD_BTN_DOWN;

    if (analog_key_value < 650)
        return _ticks_count >= KEYPAD_LONGPRESS_TICK_THRESHOLD ? KEYPAD_BTN_LONG_LEFT : KEYPAD_BTN_LEFT;

    if (analog_key_value < 850)
        return _ticks_count >= KEYPAD_LONGPRESS_TICK_THRESHOLD ? KEYPAD_BTN_LONG_SELECT : KEYPAD_BTN_SELECT;

    _ticks_count = 0;

    return KEYPAD_BTN_NONE;
}