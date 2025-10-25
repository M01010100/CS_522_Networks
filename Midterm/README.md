# Terminal-Based Chat Relay System

A simple bidirectional chat application using TCP sockets in C. This allows real-time text communication between a server and multiple clients.

## Features

- **Bidirectional Communication**: Both server and client can send and receive messages simultaneously
- **Multiple Clients**: Server uses fork() to handle multiple client connections concurrently
- **Simple Terminal Interface**: Type messages directly in the terminal
- **Connection Management**: Graceful handling of disconnections and quit commands
- **Cross-platform Scripts**: PowerShell and batch file support for easy compilation and execution

## Files

- `server.c` - Chat server implementation
- `client.c` - Chat client implementation
- `start_server.ps1` / `start_server.bat` - Server startup scripts
- `start_client.ps1` / `start_client.bat` - Client startup scripts

## Requirements

### Windows
- **WSL (Windows Subsystem for Linux)** - Recommended
  - Install with: `wsl --install` in PowerShell (admin)
- OR **MinGW/Cygwin** with GCC compiler

### Linux/macOS
- GCC compiler (`gcc`)
- Standard Unix socket libraries (usually pre-installed)

## Quick Start

### Option 1: Using PowerShell Scripts (Recommended)

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
3. Once a client connects, you can type messages in the server terminal
4. Messages are prefixed with "Client: " when received from the client
5. Type `quit` to end the chat session with the current client

### Client
1. Run the client script with the server hostname/IP
   - Examples: `localhost`, `127.0.0.1`, `192.168.1.100`
2. You'll see a welcome message when connected
3. Type messages to send to the server
4. Messages from the server are prefixed with "Server: "
5. Type `quit` to disconnect from the server

## Chat Commands

- **quit** - End the chat session and disconnect

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
3. When a client connects, forks a child process to handle that client
4. Uses `select()` to monitor both stdin and the socket simultaneously
5. Relays messages bidirectionally until either side types "quit" or disconnects

### Client Operation
1. Connects to the specified server hostname/IP on port 3490
2. Receives welcome message from server
3. Uses `select()` to monitor both stdin and the socket simultaneously
4. Sends user input to server and displays received messages
5. Maintains connection until "quit" is typed or server disconnects

## Technical Notes

### select() System Call
Both server and client use `select()` with file descriptor sets to enable non-blocking I/O:
- Monitors stdin for user input
- Monitors socket for incoming network data
- Allows simultaneous send/receive without threads

### Process Model
- Server uses `fork()` to create a child process for each client
- Parent continues accepting new connections
- Child handles bidirectional communication with one client
- `SIGCHLD` handler prevents zombie processes

## Troubleshooting

### "WSL not found" Error
Install WSL: `wsl --install` in PowerShell (admin), then restart your computer.

### "Connection refused" Error
- Ensure the server is running before starting the client
- Check that port 3490 is not blocked by firewall
- Verify the hostname/IP address is correct

### "Address already in use" Error
- Another instance of the server is already running
- Wait a few seconds for the OS to release the port
- Or kill the existing server process

### Compilation Errors
- Ensure GCC is installed: `gcc --version`
- Check that all required header files are available
- On Windows, verify WSL is properly configured

## Extending the Application

Possible enhancements:
- Add username/authentication
- Implement chat rooms (multiple clients in one session)
- Add message encryption
- Implement file transfer
- Add message history/logging
- Create a GUI interface
- Add timestamp to messages

## Port Configuration

To change the port, modify the `PORT` definition in both files:
```c
#define PORT "3490"  // Change to your desired port
```

## License

Educational use - CS 522 Networks Course
