add_executable(sensorreader sensor_processing.cpp)
target_include_directories(sensorreader PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")