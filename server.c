#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
int sockfd; // Server socket
/* Signal handler for closing the server  */
void signalhandler(int signum) {
	printf("\nCaught signal %d\nTerminating...\n",signum);
	// Close server socket
	close(sockfd);
	exit(signum);
}

/* Display error messages */
void error(const char *msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[]) {
	int newsockfd, portno;
	socklen_t clilen;
	char buffer[2048],*command[50],* word;
	struct sockaddr_in serv_addr, cli_addr;
	int n,status1,status2;
	pid_t pid,cpid,endID,endID2;
	char str[INET_ADDRSTRLEN];

	if (argc < 2) {
		fprintf(stderr, "No port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	signal(SIGINT, signalhandler); // Caughts signals
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");

	/* Create a connection queue and wait for clients.  */
	printf("listening for connections\n");
	listen(sockfd, 5);
	while(1) {
		clilen = sizeof(cli_addr);
		// Create new socket for client
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
			error("ERROR on accept");

		if((pid=fork())==-1){
			error("fork");
			exit(1);
		}
		if(pid!=0) { /* Server-Parent Process */
			// Check if child1 is still running
			endID = waitpid(pid, &status1, WNOHANG | WUNTRACED);
			if(endID == -1){ /* Error calling waitpid */
				error("waitpid");
				exit(1);
			} else if(endID == 0) { /* Child1 is still running */

			} else if (endID == pid) { /* Child1 ended */
				// Client closed connection
			}
		} else { /* Child1 process */
			// Change the signal handler to default
			signal(SIGINT, SIG_DFL);
			if (inet_ntop(AF_INET, &cli_addr.sin_addr, str, INET_ADDRSTRLEN) == NULL) {
				fprintf(stderr, "Could not convert byte to address\n");
				exit(1);
			}
			printf("accepted conncetion\n");
			fprintf(stdout, "The client address is :%s\n", str);
			while(1){
				bzero(buffer, 2048);
				n = read(newsockfd, buffer, 2047);
				if (n < 0) error("ERROR reading from socket");
					// If client sents void close client connection
					if(!strcmp(buffer,"")){
						printf("Client %s closed connection\n", str);
						break;
					}
					if((cpid=fork())==-1){
						error("fork");
						exit(1);
					}
					if (cpid!=0) { /* Child1 Process  */
						// Wait for child2 process to terminate
						while(1){
							endID2 = waitpid(cpid, &status2, WNOHANG | WUNTRACED);
							if(endID2 == -1) { /* Error calling waitpid */
								error("waitpid");
								exit(1);
							}else if(endID2 == 0) { /* Child2 still running */
								// Wait for child2 to end
							}else if(endID2 == cpid) { /* Child2 ended */
								// End while loop that waits for child2 to end
								break;
							}
						}
					}else{ /* Child2 Process */
						printf("Executing command: %s\n", buffer);
						// Separates command from parameters
						word = strtok(buffer," ");
						int i = 0;
						while(word != NULL){
							command[i] = word;
							i++;
							word = strtok(NULL," ");
						}
						command[i+1] = NULL;
						dup2(newsockfd, STDOUT_FILENO); // Redirect stdout to socket
						execvp(command[0],command); // Execute command
						fprintf(stdout, "execvp: %s\n", strerror(errno));
						exit(EXIT_SUCCESS);
					}
				}
				// Close client socket
				close(newsockfd);
				exit(EXIT_SUCCESS);
			}
		}
		// Close server socket
		close(sockfd);
		exit(EXIT_SUCCESS);
} /* End main */

