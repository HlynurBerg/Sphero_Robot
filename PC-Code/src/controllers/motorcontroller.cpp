#include "../../include/controllers/motorcontroller.hpp"
#include <SDL_ttf.h>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow("Tank Steering", 100, 100, 1280, 720, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_Joystick* joystick = nullptr;
    if(SDL_NumJoysticks() > 0) {
        joystick = SDL_JoystickOpen(0); // Open the first available joystick
    }

    int buttonX = 500, buttonY = 400, buttonW = 100, buttonH = 50; //Change these to move button. Maybe hardcode into function later.
    SDL_Rect buttonRect = {buttonX, buttonY, buttonW, buttonH};
    bool followMeMode = false;
    bool runLoop = true;
    while (runLoop) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                runLoop = false;
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.x >= buttonX && e.button.x <= (buttonX + buttonW) &&
                    e.button.y >= buttonY && e.button.y <= (buttonY + buttonH)) {
                    followMeMode = !followMeMode; // Toggle the mode, change later if more modes are added
                }
            }
        }

        // Set background color
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Render button
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &buttonRect);

        // Update the screen
        SDL_RenderPresent(renderer);

        if (followMeMode) {
            float difference = 0.5;//TODO colorTracker();
            float forwards = 0.5;
            TankSteering result = followMe(difference, forwards);
            std::cout << "Left Belt: " << result.leftBelt << ", Right Belt: " << result.rightBelt << std::endl;

        }
        else {
            const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
            TankSteering result = getTankSteering(keyboardState, joystick);
            std::cout << "Left Belt: " << result.leftBelt << ", Right Belt: " << result.rightBelt << std::endl;
        }

        SDL_Delay(10);
    }

    if(joystick) {
        SDL_JoystickClose(joystick);
    }

    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}