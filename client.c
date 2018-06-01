#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
// Display error messages
void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	char buffer[2048];
	// Check if enough arguments given
	if (argc < 3) {
		fprintf(stderr, "usage %s ip-address port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");
	while(1){
		printf("%s:>",argv[1]);
		bzero(buffer, 2048);
		fgets(buffer, 2047, stdin);
		buffer[strlen(buffer) - 1] = '\0';
		// If user types quit or hit enter close connection with server
		if(!strcmp(buffer, "quit\0") || buffer[0] == '\0'){
			printf("Exit...\n");
			break;
		}

		// Send command to server
		n = write(sockfd, buffer, strlen(buffer));
		if (n < 0)
			error("ERROR writing to socket");
		// Read data from server
		bzero(buffer, 2048);
		n = read(sockfd, buffer, 2047);
		if (n < 0){
			error("ERROR reading from socket");
		}else if(n == 0){ /* If server send 0 bytes */
			printf("Server closed connection\n");
			break;
		}
		printf("%s\n", buffer);
	}
	close(sockfd); // Close socket
	return 0;
} // End main
