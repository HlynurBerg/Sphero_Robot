// include the clients.hpp file from the client folder in the comms library
#include "comms/client/clients.hpp"

int client(int argc, char* argv[]) {
    // Start the networking and video threads
    std::thread network_thread(handle_network);
    std::thread video_thread(handle_video);
    // Join the networking and video threads before exiting
    network_thread.join();
    video_thread.join();

    return 0;
}
