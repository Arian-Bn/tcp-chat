cmake_minimum_required (VERSION 3.20)
cmake_policy(SET CMP0167 OLD)

project (tcp-chat CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

add_executable(client 
    src/client.cpp 
    src/protocol.cpp
)
add_executable(server 
    src/main.cpp 
    src/server.cpp 
    src/asio_server.cpp
    src/thread_server.cpp
    src/chat_server.cpp 
    src/chat_session.cpp 
    src/chat_room.cpp 
    src/protocol.cpp
    src/utils.cpp
)

target_include_directories(server PRIVATE include)
target_include_directories(client PRIVATE include)

target_link_libraries(server PRIVATE Boost::boost Threads::Threads)
target_link_libraries(client PRIVATE Boost::boost Threads::Threads)
