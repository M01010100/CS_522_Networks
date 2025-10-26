<!-- filepath: c:\Users\matth\Classes\CS_522_Networks\Midterm\README.md -->
# Terminal-Based Chat Relay System

A bidirectional chat application using TCP sockets in C. This allows real-time text communication between a server and multiple clients in a chat room style - messages from any client are broadcast to all other connected clients.

## Features

- **Chat Room Broadcast**: Messages from any client are broadcast to all other connected clients
- **Multiple Concurrent Clients**: Server uses `select()` to handle up to 10 simultaneous client connections
- **User Identification**: Clients provide a username/identifier when connecting
- **Message Encryption**: XOR-based encryption for secure message transmission
- **Timestamps**: Each received message is displayed with a timestamp showing when it was received
- **Join/Leave Notifications**: All users are notified when someone joins or leaves the chat
- **Simple Terminal Interface**: Type messages directly in the terminal
- **Connection Management**: Graceful handling of disconnections and quit commands
- **Cross-platform Scripts**: PowerShell and batch file support for easy compilation and execution

## Files

- `server.c` - Chat server implementation
- `client.c` - Chat client implementation
- `start_server.ps1` / `start_server.bat` - Server startup scripts
- `start_client.ps1` / `start_client.bat` - Client startup scripts

## Quick Start

### Option 1: Using PowerShell Scripts 

#### Start the Server
```powershell
cd Midterm
.\start_server.ps1
```

#### Start a Client (in a new terminal)
```powershell
cd Midterm
.\start_client.ps1 localhost
```

### Option 2: Using Batch Files (WSL)

#### Start the Server
```cmd
cd Midterm
start_server.bat
```

#### Start a Client (in a new terminal)
```cmd
cd Midterm
start_client.bat localhost
```

### Option 3: Manual Compilation (Linux/WSL)

#### Compile
```bash
cd Midterm
gcc -o server server.c -Wall
gcc -o client client.c -Wall
```

#### Run Server
```bash
./server
```

#### Run Client (in a new terminal)
```bash
./client localhost
```

## Usage

### Server
1. Run the server script - it will start listening on port 3490
2. Wait for client connections
3. When clients connect and provide usernames, they enter a chat room
4. The server displays all chat activity with usernames and timestamps
5. The server monitors and logs all messages but does not send messages itself
6. Users are notified when others join or leave the chat
7. Press Ctrl+C to shut down the server

### Client
1. Run the client script with the server hostname/IP
   - Examples: `localhost`, `127.0.0.1`, `192.168.1.100`
2. You'll see a welcome message when connected
3. Enter your name/identifier when prompted
4. After authentication, you enter a chat room with other connected users
5. Messages you type are broadcast to all other connected clients
6. You'll see messages from other users with their usernames and timestamps
7. You'll be notified when other users join or leave
8. Type `quit` to disconnect from the server

## Chat Commands

- **quit** - End the chat session and disconnect

## Security Features

### User Identification
- Each client must provide a username/identifier upon connection
- Username is encrypted during transmission
- Server displays the username with each message for context
- Helps identify participants in multi-client scenarios

### Message Encryption
- All messages are encrypted using XOR cipher before transmission
- Encryption key is shared between client and server (defined in source code)
- Key: `NetworksCS522Key`
- Both client and server automatically encrypt outgoing messages and decrypt incoming messages
- Provides basic confidentiality for chat messages over the network

### Timestamps
- Each received message is automatically timestamped
- Timestamp format: `[YYYY-MM-DD HH:MM:SS]`
- Example: `[2025-10-25 14:30:45] Server: Hello!`
- Timestamps show when messages were received, not when they were sent
- Helps track conversation flow and message timing

## Network Details

- **Port**: 3490 (defined in both server.c and client.c)
- **Protocol**: TCP (SOCK_STREAM)
- **Address Family**: IPv4 and IPv6 compatible
- **Buffer Size**: 1024 bytes

## Connection Examples

### Same Machine
```powershell
.\start_client.ps1 localhost
```

### Different Machine (same network)
```powershell
.\start_client.ps1 192.168.1.100
```

### Using Hostname
```powershell
.\start_client.ps1 my-server-name
```

## How It Works

### Server Operation
1. Creates a socket and binds to port 3490
2. Listens for incoming connections
3. Uses `select()` to monitor all client connections simultaneously (single-process, non-blocking)
4. Maintains a list of up to 10 connected clients with their usernames
5. When a message arrives from any client, broadcasts it to all other connected clients
6. Handles client disconnections and notifies remaining users

### Client Operation
1. Connects to the specified server hostname/IP on port 3490
2. Receives welcome message from server
3. Sends username to server for identification
4. Uses `select()` to monitor both stdin and the socket simultaneously
5. Sends user input to server, which broadcasts to all other clients
6. Displays received messages from other users with timestamps
7. Maintains connection until "quit" is typed or server disconnects

## Technical Notes

### select() System Call
Both server and client use `select()` with file descriptor sets to enable non-blocking I/O:
- **Server**: Monitors listener socket + all connected client sockets
- **Client**: Monitors stdin for user input + socket for incoming messages
- Allows simultaneous handling of multiple connections/events without threads

### Server Architecture
- **Single-process design**: Uses `select()` instead of `fork()` for scalability
- Maintains array of up to 10 active client connections
- Tracks username for each connected client
- Broadcasts messages from one client to all others
- No inter-process communication needed (all in one process)

## Troubleshooting


### "Connection refused" Error
- Ensure the server is running before starting the client
- Check that port 3490 is not blocked by firewall
- Verify the hostname/IP address is correct

### "Address already in use" Error
- Another instance of the server is already running
- Wait a few seconds for the OS to release the port
- Or kill the existing server process


## Port Configuration

To change the port, modify the `PORT` definition in both files:
```c
#define PORT "3490"  // Change to your desired port
```

## Code Review & Implementation Details

### Architecture Overview

This chat application follows a client-server architecture with the following key design decisions:

#### **Multi-Process Model (Server)**
- Uses `fork()` to spawn a child process for each client connection
- Parent process continues accepting new connections
- Child process handles bidirectional communication with one specific client
- Avoids threading complexity while supporting multiple concurrent clients

#### **Non-Blocking I/O with select()**
- Both client and server use `select()` system call for event-driven I/O
- Monitors multiple file descriptors simultaneously without blocking
- Eliminates need for multithreading on the client side

### Server Implementation (`server.c`)

#### **Socket Setup and Binding**
```c
// Creates IPv4/IPv6 compatible socket
getaddrinfo(NULL, PORT, &hints, &servinfo);
bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
listen(sockfd, BACKLOG);
```
- Uses `getaddrinfo()` for protocol-independent socket creation
- `AI_PASSIVE` flag allows binding to any available interface
- `BACKLOG` of 10 allows queue of pending connections

#### **Connection Handling**
```c
while(1) {
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (!fork()) {  // Child process
        close(sockfd);  // Child doesn't need listener
        // Handle client communication
        exit(0);
    }
    close(new_fd);  // Parent doesn't need this connection
}
```
**Key Points:**
- `accept()` blocks until a client connects
- `fork()` creates child process; returns 0 in child, PID in parent
- Child closes listener socket (only parent accepts new connections)
- Parent closes client socket (only child communicates with client)
- Child exits when client disconnects, preventing zombie processes via `SIGCHLD` handler

#### **Signal Handling**
```c
void sigchld_handler(int s) {
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}
```
**Purpose:**
- Reaps zombie child processes automatically
- `WNOHANG` prevents blocking if no children have exited
- Preserves `errno` to avoid side effects in signal handler

#### **Bidirectional Communication Loop**
```c
FD_ZERO(&readfds);
FD_SET(STDIN_FILENO, &readfds);  // Monitor keyboard input
FD_SET(new_fd, &readfds);         // Monitor socket data

select(new_fd + 1, &readfds, NULL, NULL, NULL);

if (FD_ISSET(STDIN_FILENO, &readfds)) {
    // User typed something - read and send
}
if (FD_ISSET(new_fd, &readfds)) {
    // Data received from client - read and display
}
```
**How select() Works:**
- `FD_ZERO` clears the file descriptor set
- `FD_SET` adds file descriptors to monitor
- `select()` blocks until one or more FDs have activity
- `FD_ISSET` checks which FD triggered the wake-up
- First parameter must be `max_fd + 1` for proper monitoring

### Client Implementation (`client.c`)

#### **Connection Establishment**
```c
getaddrinfo(argv[1], PORT, &hints, &servinfo);
// Loop through results and connect to first available
for(p = servinfo; p != NULL; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) != -1)
        break;  // Success!
}
```
**Robustness Features:**
- Iterates through all addresses returned by `getaddrinfo()`
- Handles both IPv4 and IPv6 addresses automatically
- Falls back if first connection attempt fails
- Validates successful connection before proceeding

#### **Welcome Message Reception**
```c
numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0);
buf[numbytes] = '\0';
printf("%s\n", buf);
```
**Best Practices:**
- Null-terminates received data before printing
- Leaves room in buffer (`MAXDATASIZE-1`) for null terminator
- Checks `numbytes` to handle disconnection gracefully

### Encryption Implementation

#### **XOR Cipher Function**
```c
void xor_encrypt_decrypt(char *data, int len) {
    const char *key = "NetworksCS522Key";
    int key_len = strlen(key);
    
    for (int i = 0; i < len; i++) {
        data[i] ^= key[i % key_len];
    }
}
```
**How It Works:**
- **Symmetric**: Same function encrypts and decrypts (XOR property: `A XOR B XOR B = A`)
- **Byte-by-byte**: Each character XORed with corresponding key character
- **Key repetition**: Uses modulo to cycle through key (`i % key_len`)
- **In-place**: Modifies data buffer directly for efficiency

**Security Considerations:**
- ⚠️ XOR cipher is weak against known-plaintext attacks
- 🔒 Adequate for basic obfuscation and learning purposes
- 🚀 Upgrade to AES for production use
- 🔑 Key is hardcoded (not recommended for real applications)

#### **Encryption in Practice**
```c
// Before sending
xor_encrypt_decrypt(buf, strlen(buf));
send(sockfd, buf, strlen(buf), 0);

// After receiving
numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0);
xor_encrypt_decrypt(buf, numbytes);
buf[numbytes] = '\0';
```
### Timestamp Implementation

#### **Timestamp Generation**
```c
void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "[%Y-%m-%d %H:%M:%S]", t);
}
```
**Components:**
- `time(NULL)`: Gets current Unix timestamp (seconds since 1970)
- `localtime()`: Converts to local timezone broken-down time
- `strftime()`: Formats time into human-readable string
- ISO 8601-style format: `YYYY-MM-DD HH:MM:SS`

#### **Timestamp Usage**
```c
char timestamp[32];
get_timestamp(timestamp, sizeof(timestamp));
printf("%s Server: %s", timestamp, buf);
```
**Design Choices:**
- Fixed 32-byte buffer (sufficient for timestamp format)
- Prepended to messages for consistent appearance
- Shows **receive time**, not send time (client clock used)

### Buffer Management

#### **Buffer Size Strategy**
```c
#define MAXDATASIZE 1024
char buf[MAXDATASIZE];
```
**Considerations:**
- 1024 bytes balances memory usage with practical message length
- Must always leave room for null terminator
- Handles typical chat messages (tweet-length)
- Larger buffers would support file transfer

#### **Safe String Handling**
```c
fgets(buf, sizeof(buf), stdin);  // Prevents buffer overflow
buf[strcspn(buf, "\n")] = '\0';  // Removes newline safely
xor_encrypt_decrypt(buf, strlen(buf));  // Only encrypts actual content
```
**Security Practices:**
- `fgets()` with size limit prevents overflow
- `strcspn()` safely finds newline position
- Null-terminate before any string operations
- Use actual string length, not buffer size, for encryption

### Error Handling Patterns

#### **Connection Errors**
```c
if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) <= 0) {
    if (numbytes == 0) {
        printf("Server hung up\n");
    } else {
        perror("recv");
    }
    break;
}
```
**Graceful Degradation:**
- `recv() == 0`: Clean disconnect (other side called `close()`)
- `recv() < 0`: Error occurred (network issue, socket closed)
- Distinguishes between normal and abnormal termination
- Uses `perror()` to print descriptive error messages

#### **System Call Validation**
```c
if (sockfd == -1) {
    perror("socket");
    exit(1);
}
```
**Best Practices:**
- Check return values of all system calls
- Use `perror()` to print system error messages
- Exit with non-zero status on fatal errors
- Validates state before proceeding to dependent operations

### Performance Considerations

#### **Why select() Instead of Threading?**
1. **Lower Overhead**: No context switching between threads
2. **Simpler Synchronization**: No mutexes or race conditions
3. **Resource Efficient**: Single thread monitors multiple I/O sources
4. **Suitable for I/O-Bound**: Chat apps spend most time waiting for network/keyboard

#### **Process vs Thread Model**
**Server uses processes (fork()):**
- ✅ Complete isolation between clients
- ✅ Crash in one client doesn't affect others
- ✅ Simple memory management (no shared state)
- ⚠️ Higher memory overhead per client
- ⚠️ More expensive context switches

**Client uses single process:**
- ✅ Lightweight (one connection per client)
- ✅ No inter-thread communication needed
- ✅ `select()` provides pseudo-concurrency

### Code Quality Observations

#### **Strengths**
- ✅ Clean separation of concerns (setup, loop, cleanup)
- ✅ Consistent error handling patterns
- ✅ Protocol-independent (IPv4/IPv6 compatible)
- ✅ Graceful shutdown on quit command
- ✅ Prevents zombie processes with signal handler

#### **Areas for Enhancement**
- ⚠️ Hardcoded encryption key (consider key exchange)
- ⚠️ No message framing (1024-byte limit per message)
- ⚠️ XOR cipher is weak (upgrade to AES/TLS)
- ⚠️ No authentication (anyone can connect)
- ⚠️ No logging of conversations
- ⚠️ Single-threaded server (blocking on stdin affects all clients)

### Memory Management

#### **Resource Cleanup**
```c
freeaddrinfo(servinfo);  // Free address list
close(sockfd);           // Release socket file descriptor
exit(0);                 // Cleanup on child exit
```
**Leak Prevention:**
- `getaddrinfo()` allocates memory that must be freed
- Socket file descriptors must be closed
- Child processes exit cleanly to release all resources
- OS reclaims memory on process termination

#### **File Descriptor Limits**
- Each client connection uses one file descriptor
- Default Linux limit: 1024 FDs per process
- Server can theoretically handle ~1000 concurrent clients
- Use `ulimit -n` to increase if needed

### Testing Considerations

#### **Test Scenarios**
1. **Single Client**: Basic send/receive functionality
2. **Multiple Clients**: Concurrent connections, no interference
3. **Rapid Disconnect**: Client quits immediately after connecting
4. **Long Messages**: Test 1024-byte boundary
5. **Special Characters**: Verify encryption handles all bytes
6. **Server Restart**: Client should detect disconnection
7. **Network Latency**: Works on slow connections?

#### **Debugging Tips**
```c
// Add verbose logging
printf("DEBUG: Received %d bytes\n", numbytes);

// Dump encrypted data
for (int i = 0; i < numbytes; i++)
    printf("%02x ", (unsigned char)buf[i]);
```

### Protocol Limitations

#### **Message Boundaries**
- No explicit message framing protocol
- Relies on line-buffered input (`fgets()`)
- Large messages could be fragmented by TCP
- Consider adding length prefix for production use

#### **Wire Format**
```
[Encrypted Payload]
```
- No header, just encrypted content
- No sequence numbers or checksums
- Sender doesn't know if message arrived intact
- Consider adding application-level acknowledgments

## License

Educational use - CS 522 Networks Course
