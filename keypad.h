#ifndef Keypad_h
#define Keypad_h

#define KEYPAD_LONGPRESS_TICK_THRESHOLD 40

#define KEYPAD_BTN_RIGHT 0
#define KEYPAD_BTN_LONG_RIGHT 1
#define KEYPAD_BTN_UP 2
#define KEYPAD_BTN_LONG_UP 3
#define KEYPAD_BTN_DOWN 4
#define KEYPAD_BTN_LONG_DOWN 5
#define KEYPAD_BTN_LEFT 6
#define KEYPAD_BTN_LONG_LEFT 7
#define KEYPAD_BTN_SELECT 8
#define KEYPAD_BTN_LONG_SELECT 9
#define KEYPAD_BTN_NONE 10

class Keypad
{

public:
    Keypad(unsigned short keypad_pin_in);

    void init();

    int read_buttons();

private:
    int _keypad_pin_in;
    unsigned int _ticks_count;
};

#endif