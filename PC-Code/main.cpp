#include <SDL.h>
#include <iostream>
#include <thread>
#include "communications/client.hpp"

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Tank Steering", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Joystick *joystick = nullptr;
    if (SDL_NumJoysticks() > 0) {
        joystick = SDL_JoystickOpen(0); // Open the first available joystick
    }

    bool enableColorTracking = false;
    std::cout << "Enter 1 for Sphero control, 2 for color tracking: ";
    int mode;
    std::cin >> mode;
    if (mode == 2) {
        enableColorTracking = true;
    }

    // Networking and video threads now need to consider the user's choice
    std::thread network_thread;
    if (!enableColorTracking) {
        network_thread = std::thread([&]() {
            NetworkHandler networkHandler;
            networkHandler.handle_controlling(joystick);
        });
    }

    std::thread video_thread([&]() {
        VideoHandler videoHandler;
        videoHandler.handle_video(enableColorTracking);
    });

    // Main loop now only handles window events
    bool runLoop = true;
    while (runLoop) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                runLoop = false;
            }
        }
        SDL_Delay(10);
    }

    // Cleanup
    if (joystick) {
        SDL_JoystickClose(joystick);
    }

    SDL_DestroyWindow(win);
    SDL_Quit();

    // Ensure threads are joined before exiting
    if (network_thread.joinable()) {
        network_thread.join();
    }
    video_thread.join();

    return 0;
}