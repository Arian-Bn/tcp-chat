# TCP Chat (C++)

A simple TCP chat application using Berkeley sockets. The server handles multiple clients, and clients exchange messages through the server.

---

## Tech Stack

- **C++23+** — programming language
- **Berkeley sockets** — TCP/IP API
- **epoll** — I/O multiplexing (Linux)
- **std::thread** — multithreading
- **Git** — version control
- **CMake** — build system

---

## Project Evolution 🚀

The project evolved step-by-step from basic concepts to a high-concurrency architecture:
1. **v1.0 (Multithreaded Server)**: A classic `One Thread Per Client` model. Each client was handled in a dedicated thread. While simple to implement, this approach fails to scale to thousands of clients due to thread context-switching overhead.
2. **v2.0 (Asynchronous Epoll Server)**: A complete transition to non-blocking I/O (`O_NONBLOCK`) and the Linux `epoll` system call. Now, a single server thread efficiently manages hundreds of concurrent clients, reacting only to actual socket events.

---

## Build & Run

Для автоматической сборки проекта используется **CMake**. Убедитесь, что ваш компилятор поддерживает стандарт C++23.

```bash
# 1. Генерация файлов сборки и компиляция
cmake -B build
cmake --build build

# 2. Запуск сервера (в первом терминале)
./build/server

# 3. Запуск клиента (во втором терминале)
./build/client
```

---


tcp-chat/
├── CMakeLists.txt
├── server.cpp      
├── client.cpp
├── protocol.cpp
├── protocol.hpp
├── stress_test.sh      # Load test script
└── README.md

---

## Status

✅ **Multithreaded server** — done  
✅ **Epoll server** — done  
⏳ **Boost.Asio server** — in progress 
