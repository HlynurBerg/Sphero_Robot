add_executable(sensorreader sensorreader.cpp)
target_include_directories(sensorreader PRIVATE "${CMAKE_SOURCE_DIR}/include")


add_executable(color_tracker color_tracker.cpp)
find_package(OpenCV REQUIRED)
target_link_libraries(color_tracker ${OpenCV_LIBS})
