
add_executable(communicationhandler communicationhandler.cpp
target_link_libraries(communicationhandler PRIVATE Boost::system)
target_include_directories(communicationhandler PRIVATE "${Boost_INCLUDE_DIRS}/include")

add_executable(motorcontroller motorcontroller.cpp)
target_link_libraries(motorcontroller PRIVATE Boost::system)
target_include_directories(motorcontroller PRIVATE "${Boost_INCLUDE_DIRS}/include")

add_executable(sensorreader sensorreader.cpp)
target_link_libraries(sensorreader PRIVATE Boost::system)
target_include_directories(sensorreader PRIVATE "${Boost_INCLUDE_DIRS}/include")

add_executable(tcp_client tcp_client.cpp))
target_link_libraries(tcp_client PRIVATE Boost::system)
target_include_directories(tcp_client PRIVATE "${Boost_INCLUDE_DIRS}/include")


add_executable(tcp_server tcp_server.cpp)
target_link_libraries(tcp_server PRIVATE Boost::system)
target_include_directories(tcp_server PRIVATE "${Boost_INCLUDE_DIRS}/include")