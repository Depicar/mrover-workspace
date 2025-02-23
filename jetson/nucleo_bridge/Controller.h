#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <cstring>
#include <vector>
#include <cmath>
#include <mutex>
#include <limits>
#include "Hardware.h"
#include "I2C.h"
#include "ControllerMap.h"
#include <chrono>
#include <thread>

#define OFF 0x00, 0, 0
#define ON 0x0F, 0, 0
#define OPEN 0x10, 4, 0
#define OPEN_PLUS 0x1F, 4, 4
#define CLOSED 0x20, 8, 0
#define CLOSED_PLUS 0x2F, 8, 4
#define CONFIG_PWM 0x30, 2, 0
#define CONFIG_K 0x3F, 12, 0
#define QUAD 0x40, 0, 4
#define ADJUST 0x4F, 4, 0
#define ABS_ENC 0x50, 0, 4
#define LIMIT 0x60, 0, 1
#define CALIBRATED 0x6F, 0, 1
#define LIMIT_ON 0x7F, 1, 0
#define UINT8_POINTER_T reinterpret_cast<uint8_t *>

/*
Virtual Controllers store information about various controller-specific parameters (such as encoder cpr)
The virtual Controller class also has functions representing the possible transactions that can be had with the physical controller.
The virtual Controller will not attempt to communicate with its physical controller unless "activated" by an appropriate LCM message relayed by LCMHandler.h
(e.g. A virtual RA Controller will never attempt to communicate with its physical RA controller unless an RA-related LCM message is
sent. This is to prevent multiple virtual Controller objects from trying to contact the same physical Controller object.)
*/


class Controller
{
public:
    float start_angle = 0.0;
    float torque_scale = 1.0;
    float quad_cpr = std::numeric_limits<float>::infinity();
    float current_angle = 0.0;
    float kP = 0.01;
    float kI = 0.0;
    float kD = 0.0;
    bool calibrated = false;
    int inversion = 1;
    float last_speed = 0;

    std::string name;
    std::mutex current_angle_m;

private:
    Hardware hardware;

    // Wrapper for I2C transact, autofilling the i2c address of the Controller by using ControllerMap::get_i2c_address()
    void transact(uint8_t cmd, uint8_t write_num, uint8_t read_num, uint8_t *writeBuf, uint8_t *read_buf);

    // If this Controller is not live, make it live by configuring the real controller
    void make_live();

    // Helper function to convert raw angle to radians. Also checks if new angle is close to old angle
    void record_angle(int32_t angle);

public:
    // Initialize the Controller. Need to know which type of hardware to use
    Controller(std::string name, std::string type);

    // Sends a calibrated command
    void refresh_calibration_data();

    // Calibrate joint -- should only be used for joint b
    void calibrate_joint();

    // Sends a closed loop command with target angle in radians and optional precalculated torque in Nm
    void closed_loop(float torque, float angle);

    // Sends a config command with PID inputs
    void config(float KP, float KI, float KD);

    // Sends a limit switch enable command
    void limit_switch_enable(bool enable);

    // Handles an open loop command with input [-1.0, 1.0], scaled to PWM limits
    void open_loop(float input);

    // Sends a get angle command
    void quad_angle();

    // Sends a zero command
    void zero();

    // get current angle (safely)
    float get_current_angle();

};

#endif
