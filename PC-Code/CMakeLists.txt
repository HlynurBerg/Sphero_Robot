add_executable(main main.cpp
        src/controllers/motorcontroller.cpp
        src/sensors/sensorreader.cpp
        src/comms/communicationhandler.cpp
        include/comms/communicationhandler.hpp
        include/controllers/motorcontroller.hpp
        include/sensors/sensorreader.hpp
        tests/test_communicationhandler.cpp
        tests/test_motorcontroller.cpp
        tests/test_sensorreader.cpp)