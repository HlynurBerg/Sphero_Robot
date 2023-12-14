#ifndef SPHERO_ROBOT_MOTORCONTROLLER_HPP
#define SPHERO_ROBOT_MOTORCONTROLLER_HPP

#include <SDL.h> //for User Input. Change when switching to html5 api
#include <cmath> //for absolute value function in NormalizeBelts

class TankSteering { //consider changing to a struct. everything is public
public:
    int left_belt_ = 0;
    int right_belt_ = 0;
};

float AutoStop(int value, int lower_bound, int upper_bound);
TankSteering NormalizeBelts(float left_belt_float, float right_belt_float, float max_speed, float input_strength, float turn_speed);
TankSteering GetTankSteering(const Uint8 *keyboard_state, SDL_joystick *joystick, int distance);
TankSteering FollowMe(float difference, int distance, bool is_valid);


#endif//SPHERO_ROBOT_MOTORCONTROLLER_HPP