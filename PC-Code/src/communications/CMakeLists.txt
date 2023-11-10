add_executable(html5 html5.cpp)
target_include_directories(html5 PUBLIC "${PROJECT_SOURCE_DIR}/include")


add_executable(tcp_client tcp_client.cpp)
target_include_directories(tcp_client PUBLIC "${PROJECT_SOURCE_DIR}/include")