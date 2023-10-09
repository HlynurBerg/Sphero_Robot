#include <iostream>
#include <SDL.h>


struct TankSteering {
    float leftBelt;
    float rightBelt;
};

TankSteering getTankSteering(const Uint8* keyboardState, SDL_Joystick* joystick) {
    TankSteering steering = {0.0, 0.0};

    bool isUpPressed = keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP];
    bool isDownPressed = keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN];
    bool isLeftPressed = keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT];
    bool isRightPressed = keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT];

    //TODO: TEST THIS!!!
    if(joystick) {
        int x = SDL_JoystickGetAxis(joystick, 0); // Left thumbstick horizontal axis
        int y = SDL_JoystickGetAxis(joystick, 1); // Left thumbstick vertical axis

        isUpPressed |= y < -16000;   // Threshold to consider as input
        isDownPressed |= y > 16000;  // TODO: Make seperate logic for controller, no hard limits on input
        isLeftPressed |= x < -16000;
        isRightPressed |= x > 16000;
    }

    //Tune this variable to tune steering strength while driving
    float steer = 0.5;

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

    // Normalizing
    if (abs(steering.leftBelt) > abs(steering.rightBelt)) {
        steering.rightBelt = steering.rightBelt/abs(steering.leftBelt);
        steering.leftBelt = steering.leftBelt/abs(steering.leftBelt);
    }
    else {
        steering.leftBelt = steering.leftBelt/abs(steering.rightBelt);
        steering.rightBelt = steering.rightBelt/abs(steering.rightBelt);
    }

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