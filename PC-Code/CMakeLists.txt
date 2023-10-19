
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
)



find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)


target_link_libraries(Sphero_Robot PRIVATE
        Boost::boost Boost::system
        nlohmann_json nlohmann_json::nlohmann_json
        opencv_core opencv_highgui opencv_imgproc opencv_imgcodecs
        SDL2::SDL2 SDL2::SDL2main
)

add_subdirectory(src)
add_subdirectory(tests)
