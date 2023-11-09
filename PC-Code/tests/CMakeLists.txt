project(tests)


add_executable(test_client test_client.cpp)
add_test(NAME test_client COMMAND test_tcp_client)
target_link_libraries(test_client PRIVATE Boost::system Catch2::Catch2WithMain)
target_include_directories(test_client PRIVATE "${CMAKE_SOURCE_DIR}/PC-Code/include")


