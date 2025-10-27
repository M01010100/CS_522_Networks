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

This chat application follows a **client-server broadcast architecture** with the following key design decisions:

#### **Single-Process Broadcast Server**
- Uses `select()` to handle multiple clients in a single process
- Maintains an array of all connected clients (up to 10 concurrent connections)
- Broadcasts messages from one client to all other connected clients
- More scalable than fork-based approach (no process overhead per client)
- All clients share the same process memory space

#### **Non-Blocking I/O with select()**
- Server uses `select()` to monitor listener socket + all client sockets simultaneously
- Client uses `select()` to monitor stdin (keyboard) and socket (network) simultaneously
- Event-driven architecture: only processes data when available
- No busy-waiting or polling needed

### Server Implementation (`server.c`)

#### **Client Management Structure**
```c
typedef struct {
    int fd;                      // Client socket file descriptor
    char username[64];           // Client's chosen username
    struct sockaddr_storage addr; // Client's network address
} client_t;

client_t clients[MAX_CLIENTS];   // Array of connected clients
int client_count = 0;            // Current number of clients
```
**Design Rationale:**
- Fixed-size array for simplicity (production would use dynamic list)
- Stores username for message attribution
- Tracks socket FD for sending/receiving
- No mutex needed (single-threaded)

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

#### **Connection Handling with select()**
```c
FD_SET(listener, &master);  // Add listener to master set
fdmax = listener;

while(1) {
    read_fds = master;  // Copy master to working set
    select(fdmax+1, &read_fds, NULL, NULL, NULL);
    
    for(i = 0; i <= fdmax; i++) {
        if (FD_ISSET(i, &read_fds)) {
            if (i == listener) {
                // New connection - accept and add to master set
                newfd = accept(listener, ...);
                FD_SET(newfd, &master);
            } else {
                // Data from existing client - receive and broadcast
                recv(i, buf, sizeof(buf), 0);
                broadcast_message(buf, i, username);
            }
        }
    }
}
```
**Key Design Decisions:**
- **Master set**: Persistent list of all file descriptors to monitor
- **Working set**: Copied each iteration (select modifies it)
- **Single loop**: Handles both new connections and existing client data
- **No forking**: All clients handled in same process
- **Scalability**: Can handle multiple clients with minimal overhead

**Advantages over fork() approach:**
- ✅ Lower memory usage (no separate process per client)
- ✅ Easier state sharing (all clients in same array)
- ✅ No zombie processes to manage
- ✅ Simpler inter-client communication (just loop through array)
- ✅ Better for broadcast scenarios

#### **Broadcast Mechanism**
```c
void broadcast_message(const char *message, int sender_fd, const char *sender_name) {
    char buf[MAXDATASIZE];
    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));
    
    // Format: [timestamp] Username: message
    snprintf(buf, sizeof(buf), "%s %s: %s", timestamp, sender_name, message);
    
    int msg_len = strlen(buf);
    xor_encrypt_decrypt(buf, msg_len, ENCRYPTION_KEY);
    
    // Send to all clients except sender
    for (int i = 0; i < client_count; i++) {
        if (clients[i].fd != sender_fd && clients[i].fd != -1) {
            send(clients[i].fd, buf, msg_len, 0);
        }
    }
}
```
**Implementation Notes:**
- Server adds timestamp (clients display as-is)
- Includes sender's username for attribution
- Encrypts once, sends to multiple clients
- Skips sender (no echo-back)
- Skips invalid file descriptors (-1)

#### **Client Lifecycle Management**
```c
// Add client
int add_client(int fd, struct sockaddr_storage *addr) {
    clients[client_count].fd = fd;
    clients[client_count].username[0] = '\0';  // Empty until received
    client_count++;
}

// Remove client (shift array down)
void remove_client(int index) {
    for (int i = index; i < client_count - 1; i++) {
        clients[i] = clients[i + 1];
    }
    client_count--;
}
```
**Design Considerations:**
- Simple array-based storage (no linked list complexity)
- Username initially empty, set after first message
- Remove shifts array to maintain contiguity
- O(n) removal acceptable for small MAX_CLIENTS (10)

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

#### **Username Authentication Flow**
```c
// 1. Receive welcome message
recv(sockfd, buf, MAXDATASIZE-1, 0);
printf("%s", buf);

// 2. Prompt for username
printf("Enter your name/identifier: ");
fgets(username, sizeof(username), stdin);
username[strcspn(username, "\n")] = '\0';  // Remove newline

// 3. Send encrypted username
xor_encrypt_decrypt(username, username_len, ENCRYPTION_KEY);
send(sockfd, username, username_len, 0);

// 4. Wait for acknowledgment
recv(sockfd, buf, MAXDATASIZE-1, 0);
xor_encrypt_decrypt(buf, numbytes, ENCRYPTION_KEY);
printf("%s\n", buf);  // "Welcome, Alice! You are now connected..."
```
**Security Note:**
- Username transmitted encrypted (not plaintext)
- Server validates and acknowledges
- Two-way handshake ensures connection established

#### **Client Message Loop**
```c
FD_SET(STDIN_FILENO, &master_fds);  // Monitor keyboard
FD_SET(sockfd, &master_fds);         // Monitor server

while(1) {
    select(fdmax + 1, &read_fds, NULL, NULL, NULL);
    
    if (FD_ISSET(sockfd, &read_fds)) {
        // Server sent data - receive, decrypt, display
        recv(sockfd, buf, MAXDATASIZE - 1, 0);
        xor_encrypt_decrypt(buf, numbytes, ENCRYPTION_KEY);
        printf("%s", buf);  // Already includes timestamp from server
    }
    
    if (FD_ISSET(STDIN_FILENO, &read_fds)) {
        // User typed - read, encrypt, send
        fgets(buf, MAXDATASIZE, stdin);
        xor_encrypt_decrypt(buf, msg_len, ENCRYPTION_KEY);
        send(sockfd, buf, msg_len, 0);
    }
}
```
**Key Points:**
- Client doesn't add timestamp (server does)
- Symmetric design: encrypt before send, decrypt after receive
- Quit command detected before encryption

### Encryption Implementation

#### **XOR Cipher Function**
```c
void xor_encrypt_decrypt(char *data, int len, const char *key) {
    int key_len = strlen(key);
    
    for (int i = 0; i < len; i++) {
        data[i] ^= key[i % key_len];
    }
}
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
// Before sending (client or server)
xor_encrypt_decrypt(buf, strlen(buf), ENCRYPTION_KEY);
send(sockfd, buf, strlen(buf), 0);

// After receiving (client or server)
numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0);
xor_encrypt_decrypt(buf, numbytes, ENCRYPTION_KEY);
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
- ISO 8601-style format: `[YYYY-MM-DD HH:MM:SS]`

#### **Timestamp Application**
```c
// Server adds timestamp when broadcasting
char timestamp[64];
get_timestamp(timestamp, sizeof(timestamp));
snprintf(buf, sizeof(buf), "%s %s: %s", timestamp, sender_name, message);
```
**Design Choices:**
- **Server-side only**: Server adds timestamp, clients display as-is
- **Single timestamp**: Prevents duplicate timestamps
- **Broadcast time**: Shows when message was relayed, not sent
- Fixed 64-byte buffer (sufficient for timestamp format)

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
2. **Simpler Synchronization**: No mutexes or race conditions needed
3. **Resource Efficient**: Single process monitors multiple I/O sources
4. **Suitable for I/O-Bound**: Chat apps spend most time waiting for network/keyboard
5. **Deterministic**: Easy to debug and reason about behavior

#### **Single-Process vs Multi-Process Model**
**Current Server (select-based, single-process):**
- ✅ Very low memory overhead (one process total)
- ✅ Easy state sharing (all clients in same array)
- ✅ No inter-process communication needed
- ✅ No zombie processes to manage
- ✅ Perfect for broadcast scenarios
- ⚠️ One process failure kills all clients

**Alternative (fork-based, multi-process):**
- ✅ Complete isolation between clients
- ✅ Crash in one child doesn't affect others
- ⚠️ Higher memory overhead per client (~1-2MB each)
- ⚠️ Requires IPC for client-to-client messages
- ⚠️ Zombie process management needed

**Client (select-based, single-process):**
- ✅ Lightweight (one connection only)
- ✅ No inter-thread communication needed
- ✅ `select()` provides pseudo-concurrency for I/O

#### **Scalability Analysis**
**Current Limits:**
- Max 10 concurrent clients (`MAX_CLIENTS`)
- Max 1024 bytes per message (`MAXDATASIZE`)
- Max ~1000 file descriptors (OS limit)

**Bottlenecks:**
- Broadcast O(n) for each message
- Single-threaded (one CPU core)
- No message queueing

**Production Improvements:**
- Use `epoll()` (Linux) or `kqueue()` (BSD) for >1000 clients
- Thread pool for CPU-intensive operations
- Message queue for non-blocking sends
- Rate limiting per client

### Code Quality Observations

#### **Strengths**
- ✅ Clean separation of concerns (setup, loop, cleanup)
- ✅ Consistent error handling patterns
- ✅ Protocol-independent (IPv4/IPv6 compatible)
- ✅ Graceful shutdown on quit command
- ✅ No zombie processes (no fork, no SIGCHLD needed)
- ✅ Client management abstracted (add/remove/find functions)
- ✅ Broadcast mechanism centralized
- ✅ Username authentication on connection

#### **Areas for Enhancement**
- ⚠️ Hardcoded encryption key (should use Diffie-Hellman key exchange)
- ⚠️ No message framing (1024-byte limit, could fragment)
- ⚠️ XOR cipher is weak (upgrade to AES-256-CBC or TLS)
- ⚠️ No user authentication (anyone can connect with any username)
- ⚠️ No duplicate username prevention
- ⚠️ No message logging or history
- ⚠️ No rate limiting (flood attacks possible)
- ⚠️ Fixed array size (MAX_CLIENTS = 10)
- ⚠️ No message acknowledgments

### Memory Management

#### **Resource Cleanup**
```c
freeaddrinfo(ai);        // Free address list from getaddrinfo()
close(listener);         // Release listener socket
close(clients[i].fd);    // Release client sockets
```
**Leak Prevention:**
- `getaddrinfo()` allocates memory that must be freed
- Socket file descriptors must be closed on disconnect
- Master FD set managed automatically (stack allocated)
- No dynamic allocation in main loop (clients[] is static array)
- OS reclaims memory on process termination

#### **File Descriptor Limits**
- Each client connection uses one file descriptor
- Default Linux limit: 1024 FDs per process
- Server can theoretically handle ~1000 concurrent clients
- Use `ulimit -n` to increase if needed

### Testing Considerations

#### **Test Scenarios**
1. **Single Client**: Basic send/receive functionality
2. **Multiple Clients (2-10)**: Concurrent connections, message broadcasting
3. **Client Join/Leave**: Verify notifications broadcast correctly
4. **Rapid Disconnect**: Client quits immediately after connecting
5. **Long Messages**: Test 1024-byte boundary behavior
6. **Special Characters**: Verify encryption handles all bytes (including newlines)
7. **Server Restart**: Clients should detect disconnection
8. **Network Latency**: Works on slow/high-latency connections?
9. **Username Collision**: Two clients with same username
10. **Max Clients**: 11th client should be rejected with "Server is full" message
7. **Network Latency**: Works on slow connections?

#### **Debugging Tips**
```c
// Add verbose logging to server
printf("DEBUG: Received %d bytes from client %d (%s)\n", 
       nbytes, i, clients[find_client(i)].username);

// Dump encrypted data (hex)
for (int j = 0; j < nbytes; j++)
    printf("%02x ", (unsigned char)buf[j]);
printf("\n");

// Track client count
printf("DEBUG: Current client count: %d\n", client_count);
```

**Useful Commands:**
```bash
# Monitor network traffic
sudo tcpdump -i lo port 3490 -A

# Check open connections
netstat -an | grep 3490

# Test from command line
telnet localhost 3490
```

### Protocol Limitations

#### **Message Boundaries**
- No explicit message framing protocol
- Relies on line-buffered input (`fgets()`)
- TCP may fragment large messages across multiple `recv()` calls
- Messages limited to 1024 bytes
- Consider adding length prefix for production: `[4-byte length][payload]`

#### **Wire Format**
```
Client -> Server:
  [Encrypted Username]           (on first message only)
  [Encrypted Message]            (subsequent messages)

Server -> Client:
  [Encrypted Timestamp + Username + Message]

Example encrypted packet:
  0x4F 0x2A 0x1B ... (encrypted bytes)
```
- No header, just encrypted content
- No sequence numbers or checksums (relies on TCP)
- Sender doesn't know if message was successfully broadcast
- Consider adding application-level acknowledgments

#### **Broadcasting Behavior**
- Messages sent from Client A go to ALL other clients (B, C, D...)
- Sender does NOT receive their own message (no echo)
- Server shows message on console but doesn't send it back
- Join/leave notifications sent to ALL clients except the one joining/leaving

## License

Educational use - CS 522 Networks Course
