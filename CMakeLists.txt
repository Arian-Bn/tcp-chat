cmake_minimum_required (VERSION 3.20)
cmake_policy(SET CMP0167 OLD)

project (tcp-chat CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

add_executable(server server.cpp protocol.cpp)
add_executable(client client.cpp protocol.cpp)
add_executable(asio_server asio_server.cpp ChatServer.cpp ChatSession.cpp ChatRoom.cpp)

target_link_libraries(server PRIVATE Boost::boost Threads::Threads)
target_link_libraries(client PRIVATE Boost::boost Threads::Threads)
target_link_libraries(asio_server PRIVATE Boost::boost Threads::Threads)
