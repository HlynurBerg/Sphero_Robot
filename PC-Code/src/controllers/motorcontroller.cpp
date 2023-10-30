#include "../../include/controllers/motorcontroller.hpp"

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