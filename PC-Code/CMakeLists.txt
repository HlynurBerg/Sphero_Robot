set(Boost_NO_WARN_NEW_VERSIONS 1)

OPTION(NETWORKING_BUILD_TESTS "Build the unit tests" ON)

IF (MSVC)
        add_compile_definitions("_WIN32_WINNT=0x0601")
ENDIF()

set(HEADERS
        include/comms/communicationhandler.hpp
        include/controllers/motorcontroller.hpp
        include/sensors/sensorreader.hpp
        include/comms/client/tcp_client.hpp
        )

add_executable(Sphero_Robot
        main.cpp
        src/controllers/motorcontroller.cpp
        src/sensors/sensorreader.cpp
        include/sensors/sensorreader.hpp
        tests/test_communicationhandler.cpp
        tests/test_motorcontroller.cpp
        tests/test_sensorreader.cpp
        include/comms/server/socket_handler.hpp
        include/comms/server/tcp_server.hpp
        include/comms/client/tcp_client.hpp
        tests/test_tcp_client.cpp
)



find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(Threads REQUIRED)


target_link_libraries(Sphero_Robot PRIVATE
        SDL2::SDL2 SDL2::SDL2main

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