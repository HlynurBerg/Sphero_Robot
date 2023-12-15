set(Boost_NO_WARN_NEW_VERSIONS 1)


OPTION(NETWORKING_BUILD_TESTS "Build the unit tests" ON)

IF (MSVC)
        add_compile_definitions("_WIN32_WINNT=0x0601")
        add_compile_options(/bigobj)
ENDIF()

set(HEADERS
        include/communications/client.hpp
        include/communications/websocket.hpp
        include/control/motorcontroller.hpp
        include/sensors/colortracker.hpp
        include/sensors/sensor_processing.hpp
        include/communications/client.hpp
        include/communications/websocket.hpp
        include/control/motorcontroller.hpp
        include/sensors/colortracker.hpp
        include/sensors/sensor_processing.hpp
        include/communications/tcp_client.hpp
        )

add_executable(main main.cpp
        src/control/motorcontroller.cpp
        src/communications/client.cpp
        src/sensors/sensor_processing.cpp
)

target_include_directories(main PRIVATE "${CMAKE_SOURCE_DIR}/PC-Code/include")



find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(Threads REQUIRED)


target_link_libraries(main PRIVATE
        SDL2::SDL2 SDL2::SDL2main
        ${Boost_LIBRARIES}
        opencv_core
        opencv_imgproc
        opencv_imgcodecs
        opencv_highgui
)

add_subdirectory(src)

IF(NETWORKING_BUILD_TESTS)

        include(FetchContent)

        FetchContent_Declare(
                Catch2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG v3.4.0
        )
        FetchContent_MakeAvailable(Catch2)

        enable_testing()
        add_subdirectory(tests)
ENDIF()