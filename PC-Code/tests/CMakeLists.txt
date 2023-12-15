project(tests)

find_package(OpenCV REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

# using relative path to include the header files is a bad practice but it works for now
add_executable(client_tester client_tester.cpp
        ../src/communications/client.cpp

)

add_test(NAME client_tester COMMAND client_tester)
target_link_libraries(client_tester PRIVATE
        Boost::system Catch2::Catch2WithMain
        Boost::system
        Catch2::Catch2WithMain
        ${OpenCV_LIBS}
        SDL2::SDL2
        SDL2::SDL2main
)

target_include_directories(client_tester PRIVATE "${CMAKE_SOURCE_DIR}/PC-Code/include"
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(client_tester PRIVATE
        SDL2::SDL2 SDL2::SDL2main

)

add_executable(motorcontroller_tests motorcontroller_tests.cpp
        ../src/control/motorcontroller.cpp
)

add_test(NAME motorcontroller_tests COMMAND motorcontroller_tests)
target_link_libraries(motorcontroller_tests PRIVATE
        Catch2::Catch2WithMain
        Boost::system
        Catch2::Catch2WithMain
        ${OpenCV_LIBS}
        SDL2::SDL2
        SDL2::SDL2main
)
target_include_directories(motorcontroller_tests PRIVATE "${CMAKE_SOURCE_DIR}/PC-Code/include")

# using relative path to include the header files is a bad practice but it works for now
add_executable(colortracking_tests colortracking_tests.cpp
        ../src/sensors/sensor_processing.cpp
)
add_test(NAME colortracking_tests COMMAND colortracking_tests)
target_link_libraries(colortracking_tests PRIVATE
        Catch2::Catch2WithMain
        Boost::system
        ${OpenCV_LIBS}
)
target_include_directories(colortracking_tests PRIVATE "${CMAKE_SOURCE_DIR}/PC-Code/include")

# using relative path to include the header files is a bad practice but it works for now
add_executable(JSON_tester JSON_tester.cpp
        ../src/sensors/sensor_processing.cpp
)

add_test(NAME JSON_tester COMMAND JSON_tester)
target_link_libraries(JSON_tester PRIVATE
        Catch2::Catch2WithMain
        Boost::system
        SDL2::SDL2
        SDL2::SDL2main
        ${OpenCV_LIBS}
)

target_include_directories(JSON_tester PRIVATE "${CMAKE_SOURCE_DIR}/PC-Code/include")