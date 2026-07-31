# TCP Chat (C++)

A simple TCP chat application using Berkeley sockets. The server handles multiple clients, and clients exchange messages through the server.

---

## Tech Stack

- **C++23+** — programming language
- **Berkeley sockets** — TCP/IP API
- **epoll** — I/O multiplexing (Linux)
- **std::thread** — multithreading
- **Boost.Asio** — asynchronous networking (Proactor pattern)
- **Git** — version control
- **CMake** — build system

---

## Project Evolution 🚀

The project evolved step-by-step from basic concepts to a high-concurrency architecture:
1. **v1.0 (Multithreaded Server)**: A classic `One Thread Per Client` model. Each client was handled in a dedicated thread. While simple to implement, this approach fails to scale to thousands of clients due to thread context-switching overhead.
2. **v2.0 (Asynchronous Epoll Server)**: A complete transition to non-blocking I/O (`O_NONBLOCK`) and the Linux `epoll` system call. Now, a single server thread efficiently manages hundreds of concurrent clients, reacting only to actual socket events.
3. **v3.0 (Boost.Asio Server)**: A modern Proactor-pattern implementation using Boost.Asio. Fully asynchronous with shared_ptr-based session management, message queuing, and thread-safe broadcast.

---

## Build & Run

CMake is used for building. Make sure your compiler supports C++23.

```bash
# 1. Generate build files and compile
cmake -B build
cmake --build build

# 2. Start server (choose mode)
./build/server --mode threads   # Multithreaded
./build/server --mode epoll     # Epoll (Linux)
./build/server --mode asio      # Boost.Asio

# 3. Start client
./build/client
```

---


tcp-chat/
├── CMakeLists.txt
├── README.md
├── stress_test.sh      # Load test script
├── include/            # Header files
│   ├── chat_room.hpp
│   ├── chat_server.hpp
│   ├── chat_session.hpp
│   ├── protocol.hpp
│   └── utils.hpp
└── src/                # Source files
    ├── main.cpp
    ├── server.cpp          # epoll
    ├── thread_server.cpp   # multithreaded
    ├── asio_server.cpp     # Boost.Asio
    ├── chat_server.cpp
    ├── chat_session.cpp
    ├── chat_room.cpp
    ├── client.cpp
    ├── protocol.cpp
    └── utils.cpp

---

## Status

✅ **Multithreaded server** — done  
✅ **Epoll server** — done  
✅ **Boost.Asio server** — done

All three server implementations are complete and working.

---
