#include <Arduino.h>
#include "motor_drive.h"

MotorDrive::MotorDrive(
    uint8_t motor_forward_pin_out,
    uint8_t motor_backward_pin_out,
    uint8_t motor_speed_in,
    uint8_t motor_speed_out)
{
    _motor_forward_pin_out = motor_forward_pin_out;
    _motor_backward_pin_out = motor_backward_pin_out;
    _motor_speed_in = motor_speed_in;
    _motor_speed_out = motor_speed_out;

    _current_speed = 0;
    _current_spin_direction = MOTOR_STOPPED;
};

void MotorDrive::init()
{
    pinMode(_motor_speed_out, OUTPUT);
    pinMode(_motor_forward_pin_out, OUTPUT);
    pinMode(_motor_backward_pin_out, OUTPUT);
    pinMode(_motor_speed_in, INPUT);
}

void MotorDrive::stop()
{
    _current_spin_direction = MOTOR_STOPPED;
    digitalWrite(_motor_forward_pin_out, LOW);
    digitalWrite(_motor_backward_pin_out, LOW);
}

void MotorDrive::pause()
{
    digitalWrite(_motor_forward_pin_out, LOW);
    digitalWrite(_motor_backward_pin_out, LOW);
}

void MotorDrive::resume()
{

    if (_current_spin_direction == MOTOR_STOPPED)
    {
        return;
    }

    if (_current_spin_direction == MOTOR_SPIN_FORWARD)
    {
        run_forward();
    }
    else
    {
        run_backward();
    }
}

void MotorDrive::run_forward()
{
    digitalWrite(_motor_backward_pin_out, LOW);
    digitalWrite(_motor_forward_pin_out, HIGH);
    _current_spin_direction = MOTOR_SPIN_FORWARD;
}

void MotorDrive::run_backward()
{
    digitalWrite(_motor_forward_pin_out, LOW);
    digitalWrite(_motor_backward_pin_out, HIGH);
    _current_spin_direction = MOTOR_SPIN_BACKWARD;
}

void MotorDrive::turn_rotation()
{
    if (_current_spin_direction == MOTOR_STOPPED)
    {
        return;
    }

    if (_current_spin_direction == MOTOR_SPIN_FORWARD)
    {
        run_backward();
    }
    else
    {
        run_forward();
    }
}

void MotorDrive::adjust_speed()
{
    int speed_raw_value = read_speed_pin();
    int speed = map(speed_raw_value, 0, 1023, 0, 255);

    if (_current_speed - 2 <= speed && speed <= _current_speed + 2)
    {
        // Debounce 2 steps
        return;
    }

    _current_speed = speed;
    set_motor_speed(_current_speed);
}

int MotorDrive::read_speed_pin()
{
    return analogRead(_motor_speed_in);
}

void MotorDrive::set_motor_speed(int speed)
{
    analogWrite(_motor_speed_out, speed);
}

uint8_t MotorDrive::current_direction() {
  return _current_spin_direction;
}
