/* ** server.c -- a stream socket server demo 
*/ 

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <arpa/inet.h> 
#include <sys/wait.h> 
#include <signal.h> 
#include <time.h> 

#define PORT "3490" // the port users will be connecting to 

#define BACKLOG 10	 // how many pending connections queue will hold 

// Encryption key - must match client
#define ENCRYPTION_KEY "NetworksCS522Key"

// Simple XOR encryption/decryption function
void xor_encrypt_decrypt(char *data, int length, const char *key) {
	int key_len = strlen(key);
	for (int i = 0; i < length; i++) {
		data[i] ^= key[i % key_len];
	}
}

// Get current timestamp as string
void get_timestamp(char *buffer, size_t size) {
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(buffer, size, "[%Y-%m-%d %H:%M:%S]", t);
}

void sigchld_handler(int s) 
{ 
	// waitpid() might overwrite errno, so we save and restore it: 
	int saved_errno = errno; 

	while(waitpid(-1, NULL, WNOHANG) > 0); 

	errno = saved_errno; 
} 


// get sockaddr, IPv4 or IPv6: 
void *get_in_addr(struct sockaddr *sa) 
{ 
	if (sa->sa_family == AF_INET) { 
		return &(((struct sockaddr_in*)sa)->sin_addr); 
	} 

	return &(((struct sockaddr_in6*)sa)->sin6_addr); 
} 

int main(void) 
{ 
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd 
	struct addrinfo hints, *servinfo, *p; 
	struct sockaddr_storage their_addr; // connector's address information 
	socklen_t sin_size; 
	struct sigaction sa; 
	int yes=1; 
	char s[INET6_ADDRSTRLEN]; 
	int rv; 

	memset(&hints, 0, sizeof hints); 
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM; 
	hints.ai_flags = AI_PASSIVE; // use my IP 

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) { 
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
		return 1; 
	} 

	// loop through all the results and bind to the first we can 
	for(p = servinfo; p != NULL; p = p->ai_next) { 
		if ((sockfd = socket(p->ai_family, p->ai_socktype, 
				p->ai_protocol)) == -1) { 
			perror("server: socket"); 
			continue; 
		} 

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, 
				sizeof(int)) == -1) { 
			perror("setsockopt"); 
			exit(1); 
		} 

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) { 
			close(sockfd); 
			perror("server: bind"); 
			continue; 
		} 

		break; 
	} 

	freeaddrinfo(servinfo); // all done with this structure 

	if (p == NULL)  { 
		fprintf(stderr, "server: failed to bind\n"); 
		exit(1); 
	} 

	if (listen(sockfd, BACKLOG) == -1) { 
		perror("listen"); 
		exit(1); 
	} 

	sa.sa_handler = sigchld_handler; // reap all dead processes 
	sigemptyset(&sa.sa_mask); 
	sa.sa_flags = SA_RESTART; 
	if (sigaction(SIGCHLD, &sa, NULL) == -1) { 
		perror("sigaction"); 
		exit(1); 
	} 

	printf("server: waiting for connections...\n"); 

	while(1) {  // main accept() loop 
		sin_size = sizeof their_addr; 
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); 
		if (new_fd == -1) { 
			perror("accept"); 
			continue; 
		} 

		inet_ntop(their_addr.ss_family, 
			get_in_addr((struct sockaddr *)&their_addr), 
			s, sizeof s); 
		printf("server: got connection from %s\n", s); 

		if (!fork()) { // this is the child process 
			close(sockfd); // child doesn't need the listener 
			
			// Send welcome message
			char welcome[] = "=== Connected to Chat Server ===\nType your messages and press Enter. Type 'quit' to exit.\n";
			if (send(new_fd, welcome, strlen(welcome), 0) == -1) {
				perror("send");
				close(new_fd);
				exit(1);
			}
			
			// Receive username from client
			char buf[1024];
			char username[64];
			ssize_t numbytes;
			
			numbytes = recv(new_fd, username, sizeof(username) - 1, 0);
			if (numbytes <= 0) {
				printf("Failed to receive username from client\n");
				close(new_fd);
				exit(1);
			}
			username[numbytes] = '\0';
			
			// Decrypt the username
			xor_encrypt_decrypt(username, numbytes, ENCRYPTION_KEY);
			
			// Send acknowledgment with username
			char ack_msg[128];
			snprintf(ack_msg, sizeof(ack_msg), "Welcome, %s! You are now connected to the chat server.", username);
			int ack_len = strlen(ack_msg);
			xor_encrypt_decrypt(ack_msg, ack_len, ENCRYPTION_KEY);
			if (send(new_fd, ack_msg, ack_len, 0) == -1) {
				perror("send acknowledgment");
				close(new_fd);
				exit(1);
			}
			
			// Chat relay loop - bidirectional communication
			fd_set master_fds, read_fds;
			int fdmax;
			
			FD_ZERO(&master_fds);
			FD_SET(new_fd, &master_fds);
			fdmax = new_fd;
			
			printf("Chat session started with %s (username: %s)\n", s, username);
			printf("Monitoring messages from %s...\n", username);
			
			while(1) {
				read_fds = master_fds;
				if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
					perror("select");
					break;
				}
				
				// Check if client sent data
				if (FD_ISSET(new_fd, &read_fds)) {
					numbytes = recv(new_fd, buf, sizeof(buf) - 1, 0);
					if (numbytes <= 0) {
						if (numbytes == 0) {
							printf("%s disconnected\n", username);
						} else {
							perror("recv");
						}
						break;
					}
					buf[numbytes] = '\0';
					
					// Decrypt the received message
					xor_encrypt_decrypt(buf, numbytes, ENCRYPTION_KEY);
					
					// Check if client wants to quit
					if (strncmp(buf, "quit", 4) == 0) {
						printf("%s ended the chat\n", username);
						break;
					}
					
					// Get timestamp
					char timestamp[64];
					get_timestamp(timestamp, sizeof(timestamp));
					
					printf("%s %s: %s", timestamp, username, buf);
					fflush(stdout);
				}
			}
			
			close(new_fd); 
			exit(0); 
		} 
		close(new_fd);  // parent doesn't need this 
	} 

	return 0; 
}