set(CMAKE_CXX_STANDARD 17)

add_executable (udp_client client.cpp)
target_link_libraries (udp_client net)
if(WIN32)
  target_link_libraries(udp_client ws2_32)
endif()

add_executable (udp_server server.cpp)
target_link_libraries (udp_server net)
if(WIN32)
  target_link_libraries(udp_server ws2_32)
endif()