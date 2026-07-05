# TCP Chat (C++)

A simple TCP chat application using Berkeley sockets. The server handles multiple clients, and clients exchange messages through the server.

---

## Tech Stack

- **C++23** — programming language
- **Berkeley sockets** — TCP/IP API
- **std::thread** — multithreading
- **Git** — version control
- **Make / g++** — build tools

---

## Build & Run

### Server

```bash
g++ -std=c++23 server.cpp -o server
./server
```

### Client

```bash
g++ -std=c++23 client.cpp -o client
./client
```

### Build with CMake

```bash
cmake -B build
cmake --build build
./build/server
./build/client
```

---

```
tcp-chat/
├── CMakeLists.txt
├── server.cpp
├── client.cpp
└── README.md
```

---

## Status

✅ Echo server — done  
⏳ Multithreading + multiple clients — in progress
