project(tests)

add_executable(test_communicationhandler.cpp)
add_executable(test_motorcontroller.cpp)
add_executable(test_sensorreader.cpp)

#target link catch2
target_link_libraries(test_communicationhandler.cpp PRIVATE Catch2::Catch2)
target_link_libraries(test_motorcontroller.cpp PRIVATE Catch2::Catch2)
target_link_libraries(test_sensorreader.cpp PRIVATE Catch2::Catch2)
