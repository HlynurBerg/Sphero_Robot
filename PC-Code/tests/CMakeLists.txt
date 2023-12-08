project(tests)


add_executable(client_tester client_tester.cpp)
add_test(NAME client_tester COMMAND client_tester)
target_link_libraries(client_tester PRIVATE Boost::system Catch2::Catch2WithMain)
target_include_directories(client_tester PRIVATE "${CMAKE_SOURCE_DIR}/PC-Code/include")

