add_executable(main main.cpp
        src/Controllers/MotorController.cpp
        src/Sensors/SensorReader.cpp
        src/Comms/CommunicationHandler.cpp
        Includes/Comms/CommunicationHandler.hpp
        Includes/Controllers/MotorController.hpp
        Includes/Sensors/SensorReader.hpp)