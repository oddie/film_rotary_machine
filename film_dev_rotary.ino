#include <Wire.h>
#include <LiquidCrystal.h>
#include <RTClib.h>
#include <math.h>
#include "motor_drive.h"
#include "keypad.h"
#include "timer.h"
#include "custom_chars.h"
#include "spinner.h"
#include "menu_cursor.h"

/** DEFINITIONS **/

// The pin to check buttons on a LCD keypad
#define LCD_BUTTONS_PIN_IN A1

// The pin to control a motor speed
#define MOTOR_SPEED_PIN_OUT 10

// The pins to control motor spin direction
#define MOTOR_FORWARD_PIN_OUT 2
#define MOTOR_BACKWARD_PIN_OUT 3

// The analog pin to read a potentiometer value
#define SPEED_WHEEL_PIN_IN A0

#define LCD_RS 8
#define LCD_E 9
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7

#define STATE_SETUP 0
#define STATE_RUNNING 1
#define STATE_PAUSED 2
#define STATE_SET_MINUTES 3
#define STATE_SET_SECONDS 4
#define STATE_SET_TURN_SECONDS 5

#define MENU_START 0
#define MENU_STOP 1
#define MENU_MINUTES 2
#define MENU_SECONDS 3
#define MENU_TURN_SECONDS 4

#define MENU_START_CURSOR_X 0
#define MENU_START_CURSOR_Y 0
#define MENU_STOP_CURSOR_X 8
#define MENU_STOP_CURSOR_Y 0
#define MENU_MINUTES_CURSOR_X 0
#define MENU_MINUTES_CURSOR_Y 1
#define MENU_SECONDS_CURSOR_X 4
#define MENU_SECONDS_CURSOR_Y 1
#define MENU_TURN_SECONDS_CURSOR_X 12
#define MENU_TURN_SECONDS_CURSOR_Y 1

#define CURSOR_BLINK_TICKS 10

#define DEFAULT_TIMER_MINUTES 6
#define DEFAULT_TIMER_SECONDS 0
#define DEFAULT_TURN_SECONDS 10

/** CUSTOM CHARS **/
uint8_t cc_arrow_right[8] = {
    0B00000,
    0B01000,
    0B00100,
    0B00010,
    0B00010,
    0B00100,
    0B01000,
    0B00000};

uint8_t cc_arrow_up[8] = {
    0B00100,
    0B01110,
    0B10101,
    0B00100,
    0B00100,
    0B00100,
    0B00100,
    0B00100};

uint8_t cc_arrow_down[8] = {
    0B00100,
    0B00100,
    0B00100,
    0B00100,
    0B00100,
    0B10101,
    0B01110,
    0B00100};

uint8_t cc_arrow_up_down[8] = {
    0B00100,
    0B01110,
    0B10101,
    0B00100,
    0B10101,
    0B01110,
    0B00100,
    0B00000};

uint8_t cc_spinner_sprite_1[8] = {
    0B00000,
    0B00000,
    0B01010,
    0B10001,
    0B10001,
    0B10001,
    0B01110,
    0B00000};

uint8_t cc_spinner_sprite_2[8] = {
    0B00000,
    0B00000,
    0B01110,
    0B10001,
    0B10000,
    0B10001,
    0B01110,
    0B00000};

uint8_t cc_spinner_sprite_3[8] = {
    0B00000,
    0B00000,
    0B01110,
    0B10001,
    0B10001,
    0B10001,
    0B01010,
    0B00000};

uint8_t cc_spinner_sprite_4[8] = {
    0B00000,
    0B00000,
    0B01110,
    0B10001,
    0B00001,
    0B10001,
    0B01110,
    0B00000};

/** END CUSTOM CHARS **/

/** DEFINITIONS END **/

unsigned short _btn;
unsigned short _prev_btn;

uint8_t _current_state = STATE_SETUP;
uint8_t _prev_state = STATE_SETUP;

uint8_t _current_menu = MENU_START;
uint8_t _prev_menu = MENU_START;

uint8_t _timer_minutes = DEFAULT_TIMER_MINUTES;
uint8_t _timer_seconds = DEFAULT_TIMER_SECONDS;
int _run_timer_total_seconds = DEFAULT_TIMER_MINUTES * 60;

uint8_t _turn_seconds = DEFAULT_TURN_SECONDS;
uint8_t _run_turn_seconds = DEFAULT_TIMER_SECONDS;

uint8_t _rtc_prev_sec = 0;

DateTime _now;

RTC_DS3231 _rtc;
LiquidCrystal _lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
MotorDrive _motor(MOTOR_FORWARD_PIN_OUT, MOTOR_BACKWARD_PIN_OUT, SPEED_WHEEL_PIN_IN, MOTOR_SPEED_PIN_OUT);
Keypad _keypad(LCD_BUTTONS_PIN_IN);
Timer _timer(&_rtc);
Spinner _spinner(&_lcd, CC_SPINNER_SPRITE_1_INDEX, CC_SPINNER_SPRITE_4_INDEX);
MenuCursor _menu_cursor(&_lcd, (uint8_t)CC_ARROW_RIGHT_INDEX);

void setup()
{
  Serial.begin(9600);

  // Initialize the motor drive
  _motor.init();

  // Initialize the keypad
  _keypad.init();

  // Initialize the LCD
  _lcd.begin(16, 2);
  _lcd.createChar((uint8_t)CC_ARROW_RIGHT_INDEX, (uint8_t *)&cc_arrow_right);
  _lcd.createChar((uint8_t)CC_ARROW_UP_INDEX, (uint8_t *)&cc_arrow_up);
  _lcd.createChar((uint8_t)CC_ARROW_DOWN_INDEX, (uint8_t *)&cc_arrow_down);
  _lcd.createChar((uint8_t)CC_ARROW_UPDOWN_INDEX, (uint8_t *)&cc_arrow_up_down);
  _lcd.createChar((uint8_t)CC_SPINNER_SPRITE_1_INDEX, (uint8_t *)&cc_spinner_sprite_1);
  _lcd.createChar((uint8_t)CC_SPINNER_SPRITE_2_INDEX, (uint8_t *)&cc_spinner_sprite_2);
  _lcd.createChar((uint8_t)CC_SPINNER_SPRITE_3_INDEX, (uint8_t *)&cc_spinner_sprite_3);
  _lcd.createChar((uint8_t)CC_SPINNER_SPRITE_4_INDEX, (uint8_t *)&cc_spinner_sprite_4);

  // Initialize the Wire
  Wire.begin();

  _rtc.begin();

  draw_setup_screen();
}

void loop()
{
  // Adjust motor speed
  _motor.adjust_speed();

  // Read current time
  _now = _rtc.now();

  // Read keypad buttons
  _btn = _keypad.read_buttons();

  if (_btn != _prev_btn && _btn != KEYPAD_BTN_NONE)
  {
    // debounce button presses
    // Serial.println(debug_btn_state(_btn));
    process_state();
  }

  _prev_btn = _btn;

  check_timer(_now);

  draw_state();

  _prev_state = _current_state;
  _prev_menu = _current_menu;

  delay(10);
}

/** IMPLEMENTATION **/

void process_state()
{
  switch (_btn)
  {
  case KEYPAD_BTN_RIGHT:
  case KEYPAD_BTN_LONG_RIGHT:
    btn_right();
    break;
  case KEYPAD_BTN_UP:
  case KEYPAD_BTN_LONG_UP:
    btn_up();
    break;
  case KEYPAD_BTN_DOWN:
  case KEYPAD_BTN_LONG_DOWN:
    btn_down();
    break;
  case KEYPAD_BTN_LEFT:
  case KEYPAD_BTN_LONG_LEFT:
    btn_left();
    break;
  case KEYPAD_BTN_SELECT:
  case KEYPAD_BTN_LONG_SELECT:
    btn_select();
    break;
  default:
    break;
  }
}

void btn_up()
{
  if (_current_state == STATE_RUNNING || _current_state == STATE_PAUSED)
  {
    return;
  }

  switch (_current_menu)
  {
  case MENU_MINUTES:
    if (_current_state == STATE_SET_MINUTES)
      set_timer_minutes(_timer_minutes + 1, 0, 60);    
    else
      set_state_and_menu(STATE_SETUP, MENU_START);
    break;
  case MENU_SECONDS:
    if (_current_state == STATE_SET_SECONDS)
      set_timer_seconds(_timer_seconds + 1, 0, 59);
    else
      set_state_and_menu(STATE_SETUP, MENU_START);
    break;
  case MENU_TURN_SECONDS:
    if (_current_state == STATE_SET_TURN_SECONDS)
      set_turn_seconds(_turn_seconds + 1, 5, 60);
    else
    {
      set_state_and_menu(STATE_SETUP, MENU_START);
    }
    break;
  default:
    break;
  }
}

void btn_down()
{
  if (_current_state == STATE_RUNNING || _current_state == STATE_PAUSED)
  {
    return;
  }

  switch (_current_menu)
  {
  case MENU_MINUTES:
    if (_current_state == STATE_SET_MINUTES)
      set_timer_minutes(_timer_minutes - 1, 0, 60);
    break;
  case MENU_SECONDS:
    if (_current_state == STATE_SET_SECONDS)
      set_timer_seconds(_timer_seconds - 1, 0, 59);
    break;
  case MENU_TURN_SECONDS:
    if (_current_state == STATE_SET_TURN_SECONDS)
      set_turn_seconds(_turn_seconds - 1, 5, 60);
    break;
  case MENU_START:
    set_state_and_menu(STATE_SETUP, MENU_MINUTES);
    break;
  default:
    break;
  }
}

void btn_left()
{
  if (_current_state == STATE_RUNNING)
  {
    return;
  }

  switch (_current_menu)
  {
  case MENU_SECONDS:
    if (_current_state == STATE_SETUP)
      set_state_and_menu(STATE_SETUP, MENU_MINUTES);
    break;
  case MENU_TURN_SECONDS:
    if (_current_state == STATE_SETUP)
      set_state_and_menu(STATE_SETUP, MENU_SECONDS);
    break;
  case MENU_STOP:
    if (_current_state == STATE_PAUSED)
      set_state_and_menu(STATE_PAUSED, MENU_START);
    break;
  default:
    break;
  }
}

void btn_right()
{
  if (_current_state == STATE_RUNNING)
  {
    return;
  }

  switch (_current_menu)
  {
  case MENU_MINUTES:
    if (_current_state == STATE_SETUP)
      set_state_and_menu(STATE_SETUP, MENU_SECONDS);
    break;
  case MENU_SECONDS:
    if (_current_state == STATE_SETUP)
      set_state_and_menu(STATE_SETUP, MENU_TURN_SECONDS);
    break;
  case MENU_START:
    if (_current_state == STATE_PAUSED)
      set_state_and_menu(STATE_PAUSED, MENU_STOP);
    break;
  default:
    break;
  }
}

void btn_select()
{
  if (_current_state == STATE_RUNNING)
  {
    set_state_and_menu(STATE_PAUSED, MENU_START);
    pause_machine();
    return;
  }

  switch (_current_menu)
  {
  case MENU_START:
    if (_current_state == STATE_PAUSED)
    {
      set_state_and_menu(STATE_RUNNING, MENU_START);
      resume_machine();
    }
    else
    {
      set_state_and_menu(STATE_RUNNING, MENU_START);
      start_machine();
    }
    break;
  case MENU_STOP:
    if (_current_state == STATE_PAUSED)
    {
      set_state_and_menu(STATE_SETUP, MENU_START);
      stop_machine();
    }
    break;
  case MENU_MINUTES:
    if (_current_state == STATE_SET_MINUTES)
    {
      _menu_cursor.stop_blinking();
      set_state_and_menu(STATE_SETUP, MENU_MINUTES);
    }
    else
    {
      _menu_cursor.start_blinking();
      set_state_and_menu(STATE_SET_MINUTES, MENU_MINUTES);
    }
    break;
  case MENU_SECONDS:
    if (_current_state == STATE_SET_SECONDS)
    {
      _menu_cursor.stop_blinking();
      set_state_and_menu(STATE_SETUP, MENU_SECONDS);
    }
    else
    {
      _menu_cursor.start_blinking();
      set_state_and_menu(STATE_SET_SECONDS, MENU_SECONDS);
    }
    break;
  case MENU_TURN_SECONDS:
    if (_current_state == STATE_SET_TURN_SECONDS)
    {
      _menu_cursor.stop_blinking();
      set_state_and_menu(STATE_SETUP, MENU_TURN_SECONDS);
    }
    else
    {
      _menu_cursor.start_blinking();
      set_state_and_menu(STATE_SET_TURN_SECONDS, MENU_TURN_SECONDS);
    }
    break;
  default:
    break;
  }
}

void set_state_and_menu(unsigned short state, unsigned short menu)
{
  _current_state = state;
  _current_menu = menu;
}

void set_timer_minutes(int minutes, int min, int max)
{
  if (minutes < min)
  {
    _timer_minutes = min;
    return;
  }

  if (minutes > max)
  {
    _timer_minutes = max;
    return;
  }

  _timer_minutes = minutes;

  // Serial.println("_timer_minutes: " + _timer_minutes);
}

void set_timer_seconds(int seconds, int min, int max)
{
  if (seconds < min)
  {
    _timer_seconds = min;
    return;
  }

  if (seconds > max)
  {
    _timer_seconds = max;
    return;
  }

  _timer_seconds = seconds;

  // Serial.println("_timer_seconds: " + _timer_seconds);
}

void set_turn_seconds(int seconds, int min, int max)
{
  if (seconds < min)
  {
    _turn_seconds = min;
    return;
  }

  if (seconds > max)
  {
    _turn_seconds = max;
    return;
  }

  _turn_seconds = seconds;
  // Serial.println("_turn_seconds: " + _turn_seconds);
}

void start_machine()
{
  _run_turn_seconds = _turn_seconds;
  _run_timer_total_seconds = _timer_minutes * 60 + _timer_seconds;
  _lcd.setCursor(MENU_TURN_SECONDS_CURSOR_X + 1, MENU_TURN_SECONDS_CURSOR_Y);
  _lcd.write((uint8_t)CC_ARROW_UP_INDEX);
  _spinner.start_animation();
  _motor.run_forward();
  _rtc_prev_sec = _now.second();
  // Serial.println("started");
}

void pause_machine()
{
  _spinner.stop_animation();
  _motor.pause();
  // Serial.println("paused");
}

void resume_machine()
{
  _spinner.start_animation();
  _motor.resume();
  // Serial.println("resumed");
}

void stop_machine()
{
  set_state_and_menu(STATE_SETUP, MENU_START);
  _spinner.reset();
  _motor.stop();
  // Serial.println("stopped");
}

void registerCustomChars()
{
}

void draw_setup_screen()
{
  _lcd.clear();

  _menu_cursor.draw(MENU_START_CURSOR_X, MENU_START_CURSOR_Y);

  _lcd.setCursor(MENU_START_CURSOR_X + 1, MENU_START_CURSOR_Y);
  _lcd.print("Start");

  _spinner.draw();

  _lcd.setCursor(MENU_MINUTES_CURSOR_X + 1, MENU_MINUTES_CURSOR_Y);
  _lcd.print(str_zero_prefix(_timer_minutes) + ":" + str_zero_prefix(_timer_seconds));

  _lcd.setCursor(MENU_TURN_SECONDS_CURSOR_X + 1, MENU_TURN_SECONDS_CURSOR_Y);
  _lcd.write((uint8_t)CC_ARROW_UPDOWN_INDEX);
  _lcd.setCursor(MENU_TURN_SECONDS_CURSOR_X + 2, MENU_TURN_SECONDS_CURSOR_Y);
  _lcd.print(str_zero_prefix(_turn_seconds));

  // Serial.println(_timer_minutes);
}

void draw_state()
{

  if (_spinner.is_animated())
    _spinner.draw();

  if (_menu_cursor.is_blinking())
    _menu_cursor.redraw_last();

  if (_prev_state != _current_state || _prev_menu != _current_menu)
  {
    switch (_current_state)
    {
    case STATE_RUNNING:
      _menu_cursor.draw(MENU_START_CURSOR_X, MENU_START_CURSOR_Y);
      _lcd.print("Pause ");
      _lcd.setCursor(MENU_STOP_CURSOR_X + 1, MENU_STOP_CURSOR_Y);
      _lcd.print("    ");
      break;
    case STATE_PAUSED:
      _menu_cursor.draw(MENU_START_CURSOR_X, MENU_START_CURSOR_Y);
      _lcd.print("Paused");
      _lcd.setCursor(MENU_STOP_CURSOR_X + 1, MENU_STOP_CURSOR_Y);
      _lcd.print("Stop");
      if (_current_menu == MENU_START)
      {
        _menu_cursor.draw(MENU_START_CURSOR_X, MENU_START_CURSOR_Y);
      }
      else if (_current_menu == MENU_STOP)
      {
        _menu_cursor.draw(MENU_STOP_CURSOR_X, MENU_STOP_CURSOR_Y);
      }
      break;
    case STATE_SETUP:
      if (_prev_state == STATE_PAUSED || _prev_state == STATE_RUNNING)
      {
        _lcd.clear();
        draw_setup_screen();
      }
      else
      {
        switch (_current_menu)
        {
        case MENU_START:
          _menu_cursor.draw(MENU_START_CURSOR_X, MENU_START_CURSOR_Y);
          _lcd.setCursor(MENU_SECONDS_CURSOR_X, MENU_SECONDS_CURSOR_Y);
          _lcd.print(str_zero_prefix(_timer_seconds) + " ");
          break;
        case MENU_MINUTES:
          _menu_cursor.draw(MENU_MINUTES_CURSOR_X, MENU_MINUTES_CURSOR_Y);
          _lcd.setCursor(MENU_MINUTES_CURSOR_X + 1, MENU_MINUTES_CURSOR_Y);
          _lcd.print(str_zero_prefix(_timer_minutes));
          _lcd.setCursor(MENU_SECONDS_CURSOR_X, MENU_SECONDS_CURSOR_Y);
          _lcd.print(str_zero_prefix(_timer_seconds) + " ");
          break;
        case MENU_SECONDS:
          _menu_cursor.draw(MENU_SECONDS_CURSOR_X, MENU_SECONDS_CURSOR_Y);
          _lcd.setCursor(MENU_SECONDS_CURSOR_X + 1, MENU_SECONDS_CURSOR_Y);
          _lcd.print(str_zero_prefix(_timer_seconds));
          break;
        case MENU_TURN_SECONDS:
          _menu_cursor.draw(MENU_TURN_SECONDS_CURSOR_X, MENU_TURN_SECONDS_CURSOR_Y);
          _lcd.setCursor(MENU_TURN_SECONDS_CURSOR_X + 2, MENU_TURN_SECONDS_CURSOR_Y);
          _lcd.print(str_zero_prefix(_turn_seconds));
          _lcd.setCursor(MENU_SECONDS_CURSOR_X, MENU_SECONDS_CURSOR_Y);
          _lcd.print(str_zero_prefix(_timer_seconds) + " ");
          break;
        default:
          break;
        }
      }
      break;
    default:
      break;
    }
  }

  switch (_current_state)
  {
  case STATE_SET_MINUTES:
    _lcd.setCursor(MENU_MINUTES_CURSOR_X + 1, MENU_MINUTES_CURSOR_Y);
    _lcd.print(str_zero_prefix(_timer_minutes));
    break;
  case STATE_SET_SECONDS:
    _lcd.setCursor(MENU_SECONDS_CURSOR_X + 1, MENU_SECONDS_CURSOR_Y);
    _lcd.print(str_zero_prefix(_timer_seconds));
    break;
  case STATE_SET_TURN_SECONDS:
    _lcd.setCursor(MENU_TURN_SECONDS_CURSOR_X + 2, MENU_TURN_SECONDS_CURSOR_Y);
    _lcd.print(str_zero_prefix(_turn_seconds));
    break;
  case STATE_RUNNING:
    _lcd.setCursor(MENU_MINUTES_CURSOR_X + 1, MENU_MINUTES_CURSOR_Y);
    _lcd.print(str_zero_prefix(trunc(_run_timer_total_seconds / 60)));
    _lcd.setCursor(MENU_SECONDS_CURSOR_X, MENU_SECONDS_CURSOR_Y);
    _lcd.print(str_zero_prefix(_run_timer_total_seconds - trunc(_run_timer_total_seconds / 60) * 60));
    _lcd.setCursor(MENU_TURN_SECONDS_CURSOR_X + 2, MENU_TURN_SECONDS_CURSOR_Y);
    _lcd.print(str_zero_prefix(_run_turn_seconds));

    break;
  default:
    break;
  }
}

String str_zero_prefix(int num) {
  if (num < 10) {
    return String("0") + num;
  }

  return String(num);
}

void check_timer(DateTime dt) {
  if (dt.second() == _rtc_prev_sec) {
    return;
  }

  _rtc_prev_sec = dt.second();

  if (_current_state == STATE_RUNNING) {
    _run_timer_total_seconds--;
    if (_run_timer_total_seconds <= 0) {
      stop_machine();
    } else {
      _run_turn_seconds--;
      if (_run_turn_seconds <= 0) {
        _run_turn_seconds = _turn_seconds;
        _lcd.setCursor(MENU_TURN_SECONDS_CURSOR_X + 1, MENU_TURN_SECONDS_CURSOR_Y);
        
        if (_motor.current_direction() == MOTOR_SPIN_FORWARD) {
          _lcd.write((uint8_t)CC_ARROW_DOWN_INDEX);  
        } else {
          _lcd.write((uint8_t)CC_ARROW_UP_INDEX);
        }

        _spinner.turn_rotation();
        _motor.turn_rotation();
      }
    }
  }
}

String debug_btn_state(uint8_t st)
{

  switch (st)
  {
  case KEYPAD_BTN_RIGHT:
    return String("KEYPAD_BTN_RIGHT");
    break;
  case KEYPAD_BTN_LONG_RIGHT:
    return String("KEYPAD_BTN_LONG_RIGHT");
    break;
  case KEYPAD_BTN_UP:
    return String("KEYPAD_BTN_UP");
    break;
  case KEYPAD_BTN_LONG_UP:
    return String("KEYPAD_BTN_LONG_UP");
    break;
  case KEYPAD_BTN_DOWN:
    return String("KEYPAD_BTN_DOWN");
    break;
  case KEYPAD_BTN_LONG_DOWN:
    return String("KEYPAD_BTN_LONG_DOWN");
    break;
  case KEYPAD_BTN_LEFT:
    return String("KEYPAD_BTN_LEFT");
    break;
  case KEYPAD_BTN_LONG_LEFT:
    return String("KEYPAD_BTN_LONG_LEFT");
    break;
  case KEYPAD_BTN_SELECT:
    return String("KEYPAD_BTN_SELECT");
    break;
  case KEYPAD_BTN_LONG_SELECT:
    return String("KEYPAD_BTN_LONG_SELECT");
    break;
  default:
    return String("KEYPAD_BTN_NONE");
    break;
  }
}
