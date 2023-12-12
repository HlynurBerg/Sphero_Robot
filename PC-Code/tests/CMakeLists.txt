project(tests)

add_subdirectory(communications)

find_package(OpenCV REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

add_executable(client_tester client_tester.cpp)
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