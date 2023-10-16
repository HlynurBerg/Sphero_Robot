#include <iostream>
#include <SDL.h>

struct TankSteering {
    float leftBelt;
    float rightBelt;
};

TankSteering getTankSteering(const Uint8* keyboardState, SDL_Joystick* joystick) {

    TankSteering steering = {0.0, 0.0};

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

    //KBM inputs
    if(isUpPressed || isDownPressed || isLeftPressed || isRightPressed) {

        // Adding up inputs
        if (isUpPressed) {
            steering.leftBelt++;
            steering.rightBelt++;
        }
        if (isDownPressed) {
            steering.leftBelt--;
            steering.rightBelt--;
        }
        if (isLeftPressed) {
            steering.leftBelt = steering.leftBelt - steer;
            steering.rightBelt = steering.rightBelt + steer;
        }
        if (isRightPressed) {
            steering.leftBelt = steering.leftBelt + steer;
            steering.rightBelt = steering.rightBelt - steer;
        }
    }

    //Joystick input
    else {
        int x = SDL_JoystickGetAxis(joystick, 0); // Left joystick horizontal axis (left is -32768, right is 32767)
        int y = SDL_JoystickGetAxis(joystick, 1); // Left joystick vertical axis (up is -32768, down is 32767)

        //Joystick has a square sensor, but a round physical limit. At som points, the max value of the sensor
        // is exceeded so detail is lost. In order to compensate for this, we draw a smaller circle inside
        // the square sensor, and consider everything outside the circle to be at maximum input strength.
        inputStrength = sqrt(x * x + y * y)/32768;

        if (inputStrength > 1) {
            inputStrength = 1;
        }

        // Check if the magnitude is greater than the deadzone
        if (inputStrength > 0.2) {
            steering.leftBelt = -y + steer*x;
            steering.rightBelt = -y - steer*x;
        }
        else {
            steering.leftBelt = 0;
            steering.rightBelt = 0;
        }
    }
    // Normalizing
    if (abs(steering.leftBelt) > abs(steering.rightBelt)) {
        steering.rightBelt = steering.rightBelt/abs(steering.leftBelt);
        steering.leftBelt = steering.leftBelt/abs(steering.leftBelt);
    }
    else {
        steering.leftBelt = steering.leftBelt/abs(steering.rightBelt);
        steering.rightBelt = steering.rightBelt/abs(steering.rightBelt);
    }

    // Convert to RawMotorModesEnum value range
    steering.leftBelt = round(steering.leftBelt*255*maxSpeed*inputStrength);
    steering.rightBelt = round(steering.rightBelt*255*maxSpeed*inputStrength);

    return steering;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow("Tank Steering", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Joystick* joystick = NULL;
    if(SDL_NumJoysticks() > 0) {
        joystick = SDL_JoystickOpen(0); // Open the first available joystick
    }

    bool runLoop = true;
    while (runLoop) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                runLoop = false;
            }
        }

        const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
        TankSteering result = getTankSteering(keyboardState, joystick);
        std::cout << "Left Belt: " << result.leftBelt << ", Right Belt: " << result.rightBelt << std::endl;

        SDL_Delay(10);
    }

    if(joystick) {
        SDL_JoystickClose(joystick);
    }

    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}