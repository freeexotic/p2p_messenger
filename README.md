# P2P Messenger

Decentralized peer-to-peer messenger written in C++. Allows creating chats and exchanging messages directly between clients without a central server.

## Features

- **Decentralized architecture**: direct connection between clients
- **Two operation modes**: create new chat or join existing one
- **Real-time message exchange**
- **Automatic participant discovery**: when connecting, clients exchange lists of other participants
- **Multithreading**: separate threads for listening and processing connections
- **Input data validation**: IP addresses, ports and usernames verification
- **Graceful shutdown**: proper termination with closing all connections

## Requirements

- **Compiler**: supporting C++17 (GCC, Clang)
- **Operating System**: Linux/Unix (uses POSIX sockets)
- **Libraries**: C++ standard library, system libraries for socket operations

## Building

### Build with qmake (recommended)
```bash
qmake p2p_chat.pro
make
```

### Build with g++
```bash
g++ -std=c++17 -pthread -o p2p_messenger main.cpp client.cpp clientbase.cpp
```

## Usage

### Running the application
```bash
./p2p_messenger
```

### Connection setup

When starting, the program will ask for:

1. **IP address** (format: 127.0.0.1)
   - Your local IP address for listening to incoming connections
   - IPv4 format validation supported

2. **Port** (range: 1-65535)
   - Port for listening to incoming connections
   - Make sure the port is not occupied by other applications

3. **Username**
   - Unique name (no spaces, up to 50 characters)
   - Will be displayed in messages

4. **Operation mode**:
   - `1` - **Create chat**: become first participant, others can connect to you
   - `2` - **Join chat**: connect to existing chat

### Joining existing chat

If mode `2` is selected, the program will ask for:
- IP address of chat participant
- Port of chat participant

After successful connection, you will automatically receive the list of all chat participants.

### Chat commands

After successful connection, the following commands are available:

- **`/msg <message>`** - send message to all participants
- **`/list`** - show list of connected participants
- **`/quit`** - exit chat
- **Any other text** - sent as message to all participants

### Example session

```
Enter your IP in format: 127.0.0.1
192.168.1.100
Enter your port in format: 1234
8080
Enter your username (no spaces, up to 50 characters)
Alice
[1] - create chat
[2] - join chat
1

Available commands:
/msg <message> - send message to everyone
/list - show connected clients
/quit - exit program

Enter command or message: Hello everyone!
Message sent.
Enter command: /list

=== Connected clients ===
IP: 192.168.1.101, Port: 8081, Username: Bob
IP: 192.168.1.102, Port: 8082, Username: Charlie
========================

Enter command: /quit
Exiting program...
Closing connections...
Shutdown completed.
```

## Architecture

### Main components

#### `main.cpp`
- Application entry point
- User input handling with validation
- Client creation and startup

#### `client.hpp/cpp`
- Main class for P2P connections
- Network connections management
- Message exchange protocol handling
- Multithreaded incoming connections processing

#### `clientbase.hpp/cpp`
- Connected clients database management
- Chat participants information storage
- Client information packets creation

#### `ClientNode`
Data structure for storing client information:
```cpp
struct ClientNode {
    std::string username_;    // Username
    std::string ip_;         // IP address
    std::uint16_t port_;     // Port
    int socket_to_send_;     // Socket for sending data
};
```

### Message exchange protocol

The application uses text protocol with prefixes for message typing:

- **`01010101011`** - Connection request with information exchange
- **`01010101111`** - Connection confirmation and name sending
- **`01010101001`** - All chat participants list transmission
- **`01010111011`** - Simple connection without additional negotiation
- **`MESSAGE`** - Text message between participants

### Multithreading

- **Main thread**: user input processing and message sending
- **Listening thread**: accepting new connections (detached thread)
- **Processing threads**: separate thread created for each connected client (detached)

## Technical features

### Network interaction
- Uses TCP sockets for reliable delivery
- Non-blocking sockets for listening
- Automatic reconnection on failures
- Connection break handling

### Error handling
- All user inputs validation
- Graceful network errors handling
- Automatic retry attempts for sending
- Proper resource cleanup on termination

### Security
- Incoming data validation
- Username length limitations
- IP addresses and ports correctness verification

## Known limitations

1. **No encryption**: messages transmitted in plain text
2. **No authentication**: anyone can connect to chat
3. **IPv4 only**: only IPv4 supported
4. **Local network**: works best within single network
5. **Limited scalability**: optimal for small groups (up to 10-20 participants)

## Possible improvements

- [ ] Add encryption (TLS/SSL)
- [ ] Authentication system
- [ ] IPv6 support
- [ ] File transfers
- [ ] Message history
- [ ] Group chats with moderation
- [ ] GUI interface
- [ ] Configuration files
- [ ] Logging

## License

This project is created for educational purposes. Feel free to use and modify the code.

## Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Make changes and commit (`git commit -m 'Add amazing feature'`)
4. Push branch (`git push origin feature/amazing-feature`)
5. Create Pull Request

## Support

When encountering problems:
1. Check if port is not occupied: `netstat -ln | grep :8080`
2. Make sure firewall doesn't block connections
3. Verify IP addresses correctness in local network
4. When building, ensure C++17 support is available

## Network interaction example

```
Client A (192.168.1.100:8080) - Chat creator
    ↑
    ├── Client B (192.168.1.101:8081) - Connects to A
    ├── Client C (192.168.1.102:8082) - Connects to A, gets info about B
    └── Client D (192.168.1.103:8083) - Connects to A, gets info about B,C

Result: all clients connected to each other (full mesh topology)
```

---

*Developed using modern C++17 and POSIX sockets*