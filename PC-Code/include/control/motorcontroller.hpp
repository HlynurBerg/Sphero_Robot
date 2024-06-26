#ifndef SPHERO_ROBOT_MOTORCONTROLLER_HPP
#define SPHERO_ROBOT_MOTORCONTROLLER_HPP

#include <SDL.h>
#include <cmath>
#include "sensors/sensor_processing.hpp"

class TankSteering {
public:
    int left_belt_ = 0;
    int right_belt_ = 0;
};

TankSteering NormalizeBelts(float left_belt_float, float right_belt_float, float max_speed, float input_strength, float turn_speed);
TankSteering GetTankSteering(const Uint8 *keyboard_state, SDL_Joystick *joystick, int distance, float max_speed);
TankSteering FollowMe(float difference, int distance, bool is_valid, float max_speed);


#endif//SPHERO_ROBOT_MOTORCONTROLLER_HPP