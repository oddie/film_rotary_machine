#ifndef MotorDrive_h
#define MotorDrive_h

#define MOTOR_STOPPED 0
#define MOTOR_SPIN_FORWARD 1
#define MOTOR_SPIN_BACKWARD 2

class MotorDrive
{

public:
    MotorDrive(
        uint8_t motor_forward_pin_out,
        uint8_t motor_backward_pin_out,
        uint8_t motor_speed_in,
        uint8_t motor_speed_out);

    void init();

    void stop();

    void pause();

    void resume();

    void turn_rotation();

    void run_forward();

    void run_backward();

    void adjust_speed();

    uint8_t current_direction();

private:
    uint8_t _motor_forward_pin_out;
    uint8_t _motor_backward_pin_out;
    uint8_t _motor_speed_in;
    uint8_t _motor_speed_out;
    uint8_t _current_spin_direction;
    uint8_t _current_speed;

    int read_speed_pin();

    void set_motor_speed(int speed);
};

#endif