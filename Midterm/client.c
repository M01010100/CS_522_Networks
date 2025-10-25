/* ** client.c -- a stream socket client demo 
*/ 

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 

#include <arpa/inet.h> 

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 1024 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6: 
void *get_in_addr(struct sockaddr *sa) 
{ 
	if (sa->sa_family == AF_INET) { 
		return &(((struct sockaddr_in*)sa)->sin_addr); 
	} 

	return &(((struct sockaddr_in6*)sa)->sin6_addr); 
} 

int main(int argc, char *argv[]) 
{ 
	int sockfd, numbytes;  
	char buf[MAXDATASIZE]; 
	struct addrinfo hints, *servinfo, *p; 
	int rv; 
	char s[INET6_ADDRSTRLEN]; 

	if (argc != 2) { 
	    fprintf(stderr,"usage: client hostname\n"); 
	    exit(1); 
	} 

	memset(&hints, 0, sizeof hints); 
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM; 

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) { 
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
		return 1; 
	} 

	// loop through all the results and connect to the first we can 
	for(p = servinfo; p != NULL; p = p->ai_next) { 
		if ((sockfd = socket(p->ai_family, p->ai_socktype, 
				p->ai_protocol)) == -1) { 
			perror("client: socket"); 
			continue; 
		} 

		inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), 
			s, sizeof s); 
		printf("client: attempting connection to %s\n", s); 

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) { 
			perror("client: connect"); 
			close(sockfd); 
			continue; 
		} 

		break; 
	} 

	if (p == NULL) { 
		fprintf(stderr, "client: failed to connect\n"); 
		return 2; 
	} 

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), 
			s, sizeof s); 
	printf("client: connected to %s\n", s); 

	freeaddrinfo(servinfo); // all done with this structure 

	// Receive welcome message
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) { 
	    perror("recv"); 
	    exit(1); 
	} 

	buf[numbytes] = '\0'; 
	printf("%s", buf); 

	// Chat loop - bidirectional communication
	fd_set master_fds, read_fds;
	int fdmax;
	
	FD_ZERO(&master_fds);
	FD_SET(STDIN_FILENO, &master_fds);
	FD_SET(sockfd, &master_fds);
	fdmax = sockfd;
	
	printf("\n"); // Add newline after welcome message
	
	while(1) {
		read_fds = master_fds;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			break;
		}
		
		// Check if server sent data
		if (FD_ISSET(sockfd, &read_fds)) {
			numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0);
			if (numbytes <= 0) {
				if (numbytes == 0) {
					printf("\nServer disconnected\n");
				} else {
					perror("recv");
				}
				break;
			}
			buf[numbytes] = '\0';
			printf("Server: %s", buf);
			fflush(stdout);
		}
		
		// Check if user typed something
		if (FD_ISSET(STDIN_FILENO, &read_fds)) {
			if (fgets(buf, MAXDATASIZE, stdin) != NULL) {
				// Check if user wants to quit
				if (strncmp(buf, "quit", 4) == 0) {
					printf("Ending chat session\n");
					send(sockfd, buf, strlen(buf), 0);
					break;
				}
				
				// Send message to server
				if (send(sockfd, buf, strlen(buf), 0) == -1) {
					perror("send");
					break;
				}
			}
		}
	}

	close(sockfd); 

	return 0; 
}