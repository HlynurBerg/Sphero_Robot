#include <control/motorcontroller.hpp>

TankSteering normalizeBelts(float leftBeltFloat, float rightBeltFloat, float maxSpeed, float inputStrength, float turnSpeed) {
    //Make both belts have a value of between -1 and 1
    if ((fabs(leftBeltFloat) > fabs(rightBeltFloat)) and (round(leftBeltFloat) != 0)) {
        rightBeltFloat = rightBeltFloat/fabs(leftBeltFloat);
        leftBeltFloat = leftBeltFloat/fabs(leftBeltFloat);
    }
    else if (round(rightBeltFloat) != 0) {
        leftBeltFloat = leftBeltFloat/fabs(rightBeltFloat);
        rightBeltFloat = rightBeltFloat/fabs(rightBeltFloat);
    }
    //Make the leftBelt and rightBelt have integer values between -255 and 255
    TankSteering normalized;
    //turn slower when not driving. It was hard to control without this.
    if (leftBeltFloat == -rightBeltFloat) {
        normalized.leftBelt = round(leftBeltFloat * 255 * maxSpeed * inputStrength * turnSpeed);
        normalized.rightBelt = round(rightBeltFloat * 255 * maxSpeed * inputStrength * turnSpeed);
    }
    else {
        normalized.leftBelt = round(leftBeltFloat * 255 * maxSpeed * inputStrength);
        normalized.rightBelt = round(rightBeltFloat * 255 * maxSpeed * inputStrength);
    }
    return normalized;
}


TankSteering getTankSteering(const Uint8* keyboardState, SDL_Joystick* joystick) {

    float leftBeltFloat = 0.0, rightBeltFloat = 0.0;

    //Checking keypresses
    bool isUpPressed = keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP];
    bool isDownPressed = keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN];
    bool isLeftPressed = keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT];
    bool isRightPressed = keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT];

    //Tune this variable to tune steering strength while driving
    float steer = 0.5;
    //Max speed of the robot
    float maxSpeed = 0.3;
    //Tilt of the joystick
    float inputStrength = 1;
    //Speed of rotation
    float turnSpeed = 0.5;

    //KBM inputs
    if(isUpPressed || isDownPressed || isLeftPressed || isRightPressed) {

        // Adding up inputs
        if (isUpPressed) {
            leftBeltFloat++;
            rightBeltFloat++;
        }
        if (isDownPressed) {
            leftBeltFloat--;
            rightBeltFloat--;
        }
        if (isLeftPressed) {
            leftBeltFloat = leftBeltFloat - steer;
            rightBeltFloat = rightBeltFloat + steer;
        }
        if (isRightPressed) {
            leftBeltFloat = leftBeltFloat + steer;
            rightBeltFloat = rightBeltFloat - steer;
        }
    }

    //Joystick input
    else {
        int x = SDL_JoystickGetAxis(joystick, 0);
        int y = SDL_JoystickGetAxis(joystick, 1);

        inputStrength = sqrt(x * x + y * y)/32768;
        if (inputStrength > 1) {
            inputStrength = 1;
        }

        if (inputStrength > 0.2) {
            leftBeltFloat = -y + steer*x;
            rightBeltFloat = -y - steer*x;
        }
        //Deadzone
        else {
            leftBeltFloat = 0;
            rightBeltFloat = 0;
        }
    }
    return normalizeBelts(leftBeltFloat, rightBeltFloat, maxSpeed, inputStrength, turnSpeed);
}

TankSteering followMe(float difference, float forwards) {
    //parameters for how the robot should drive while autonomous
    float inputStrength = 1;
    float maxSpeed = 0.3;
    float steer = 0.5;
    float turnSpeed = 0.3;
    float leftBeltFloat = forwards + steer*difference;
    float rightBeltFloat = forwards - steer*difference;

    TankSteering result = normalizeBelts(leftBeltFloat, rightBeltFloat, maxSpeed, inputStrength, turnSpeed);
    return result;
}