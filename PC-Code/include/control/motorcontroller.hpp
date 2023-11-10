#ifndef SPHERO_ROBOT_MOTORCONTROLLER_HPP
#define SPHERO_ROBOT_MOTORCONTROLLER_HPP

#include <SDL.h> //for User Input. Change when switching to html5 api
#include <cmath> //for absolute value function in normalizeBelts

class TankSteering { //consider changing to a struct. everything is public
public:
    int leftBelt;
    int rightBelt;
};

TankSteering normalizeBelts(float leftBeltFloat, float rightBeltFloat, float maxSpeed, float inputStrength, float turnSpeed);
TankSteering getTankSteering(const Uint8 *keyboardState, SDL_Joystick *joystick);
TankSteering followMe(float difference, float forwards);

#endif//SPHERO_ROBOT_MOTORCONTROLLER_HPP
