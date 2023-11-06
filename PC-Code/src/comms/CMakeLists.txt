
add_executable(tcp_client tcp_client.cpp)
target_include_directories(tcp_client PRIVATE "${CMAKE_SOURCE_DIR}/PC-Code/include")
find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(tcp_client ${Boost_LIBRARIES})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(tcp_client ${SDL2_LIBRARIES})
