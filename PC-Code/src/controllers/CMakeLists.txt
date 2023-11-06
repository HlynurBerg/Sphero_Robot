add_executable(motorcontroller motorcontroller.cpp)
target_include_directories(motorcontroller PRIVATE "${CMAKE_SOURCE_DIR}/include")
find_package(SDL2 REQUIRED)
include_directories(motorcontroller PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(motorcontroller PRIVATE SDL2::SDL2 SDL2::SDL2main)

find_package(SDL2_ttf REQUIRED)
include_directories(${SDL2_TTF_INCLUDE_DIRS})
target_link_libraries(motorcontroller PRIVATE ${SDL2_TTF_LIBRARIES})
