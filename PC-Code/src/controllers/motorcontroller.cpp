#include "../../include/controllers/motorcontroller.hpp"


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
        else {
            leftBeltFloat = 0;
            rightBeltFloat = 0;
        }
    }

    // Normalize
    if ((fabs(leftBeltFloat) > fabs(rightBeltFloat)) and (round(leftBeltFloat) != 0)) {
        rightBeltFloat = rightBeltFloat/fabs(leftBeltFloat);
        leftBeltFloat = leftBeltFloat/fabs(leftBeltFloat);
    }
    else if (round(rightBeltFloat) != 0) {
        leftBeltFloat = leftBeltFloat/fabs(rightBeltFloat);
        rightBeltFloat = rightBeltFloat/fabs(rightBeltFloat);
    }

    TankSteering steering;
    steering.leftBelt = round(leftBeltFloat * 255 * maxSpeed * inputStrength);
    steering.rightBelt = round(rightBeltFloat * 255 * maxSpeed * inputStrength);

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

    SDL_Joystick* joystick = nullptr;
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

        const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
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