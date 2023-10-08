cmake_minimum_required(VERSION 3.26)
project(Sphero_Robot)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} PC-Code/main.cpp)

find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)


target_link_libraries(${PROJECT_NAME} PRIVATE
        Boost::boost Boost::system
        nlohmann_json nlohmann_json::nlohmann_json
        opencv_core opencv_highgui opencv_imgproc opencv_imgcodecs
        SDL2::SDL2 SDL2::SDL2main
)



add_subdirectory(PC-Code)
