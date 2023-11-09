add_executable(motorcontroller motorcontroller.cpp)
target_include_directories(motorcontroller PRIVATE "${CMAKE_SOURCE_DIR}/include")

add_executable(userinput userinput.cpp)
target_include_directories(userinput PRIVATE "${CMAKE_SOURCE_DIR}/include")