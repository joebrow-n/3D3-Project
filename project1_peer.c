/* A simple echo client using TCP */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/select.h>
#include <dirent.h>

#define SERVER_TCP_PORT 33000	/* well-known port */
#define BUFLEN		256	/* buffer length */

struct pdu1 {
	char type;
	char peerName[10];
	char contentName[10];
	struct sockaddr_in data;
};

struct pdu2 {
	char type;
	char data[100];
} error, ack;

void newAddress(struct pdu1 *pdu, int sd_tcp);
void sendFile(int sd);
void receiveFile(int sd, char* content);

int main(int argc, char **argv)
{
	int n, i, alen, ret_sel, sd_tcp, sd_tcp3;
	int sd, port = 33000, file;
	struct	hostent	*phe; // Store information about given host
	struct	sockaddr_in sin, server, reg_addr, client;
	char *host = "localhost", *pos, username[10], sbuf[BUFLEN];
	char command;
	char registered[10];
	struct pdu1 rpdu;
	fd_set rfds, afds;

	// Check number of commandline arguments
	switch (argc) {
		// If there is only one commandline argument (argument to run the program) then break from switch statement
		case 1:
			break;
		// If there are two commandline arguments then the second commandline argument is the IP address of the server
		case 2:
			host = argv[1];
		// If there are 3 commandline arguments then the second is commandline argument is the IP address of the server 
		// and the third is the listening port number of the server
		case 3:
			host = argv[1];
			port = atoi(argv[2]);
			break;
		// If none of the above conditions are satisfied, return an error
		default:
			fprintf(stderr, "usage: UDPtime [host [port]]\n");
			exit(1);
	}

	// set the sin sockaddr_in to zero
	memset(&sin, 0, sizeof(sin));
	// Standard state for sin_family
    sin.sin_family = AF_INET; 
	// convert the port number to a network byte order - defines the bit-order of network addresses as they pass through the network                                     
    sin.sin_port = htons(port);
        
    // Create hostent struct for host (contains information about host)
	if (phe = gethostbyname(host)){
			// copy the address into the sin struct
			memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
	}
	// Else return error
	else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		fprintf(stderr, "Can't get host entry \n");
                                                                                
    // Create a UDP socket - uses AF_INET for hosts connected by IPv4, SOCK_DGRAM for UDP connection, Protocol value 0 for IP
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	// If error occurred during creation, print error message
	if (sd < 0)
		fprintf(stderr, "Can't create UDP socket \n");

    // Create a TCP socket - uses AF_INET for hosts connected by IPv4, SOCK_STREAM for TCP connection, Protocol value 0 for IP
	sd_tcp = socket(AF_INET, SOCK_STREAM, 0);
	// If error occurred during creation, print error message
	if (sd_tcp < 0)
		fprintf(stderr, "Can't create TCP socket \n");
	
	// create new address for the TCP socket
	newAddress(&rpdu, sd_tcp); 
                                                                      
    /* Connect the UDP socket */
	if (connect(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "Can't connect to server");
	
	// Begin the creation of a peer
	printf("Enter Username: \n");
	scanf("%s", username);
	
	
	error.type = 'E';
	strcpy(error.data,"Error");
	
	ack.type = 'A';
	
	printf("'?' for help\n");
	while(1){
		// Initialize current set
		FD_ZERO(&afds); // current: afds, ready: rfds, server: sd
		FD_SET(sd_tcp, &afds);
		FD_SET(sd_tcp3, &afds);
		FD_SET(0, &afds);
			
		memcpy(&rfds, &afds, sizeof(rfds));

		if((ret_sel = select(FD_SETSIZE, &rfds, NULL, NULL, NULL)) < 0){
			printf("Select error: %d\n", ret_sel);
		}
		if(FD_ISSET(0, &rfds)){
			// stdin
			scanf(" %c", &command);

			struct pdu1 tpdu, spdu, dpdu, rpdu2;
			struct pdu2 bp, opdu;	
			
			switch(command){
			case '?':
				printf("Commands:\n R - Content Registration\n T - Content Deregistration\n L - List Local Content\n D - Content Download Request\n O - List all the online content\n Q - Quit\n");
				break;
			case 'R':
				rpdu.type = 'R';
				strcpy(rpdu.peerName, username);
				printf("File (to upload) name: \n");
				scanf(" %s", &rpdu.contentName);
				
				printf("Port: %u\n", rpdu.data.sin_port);
				
				// Send to Server
				write(sd, &rpdu, sizeof(rpdu));
				printf("PDU Sent...\n");
				
				// Reply from Server
				while ((i = read(sd, &bp, sizeof(bp))) > 0){
					if(bp.type == 'E'){
						printf("%s\n", bp.data);
						break;
					}else if(bp.type == 'A'){
						printf("Acknowledged, success!\n");
						strcpy(registered, rpdu.contentName);
						break;
					}
				}
				break;
			case 'T':
				tpdu.type = 'T';
				strcpy(tpdu.peerName, username);
				printf("File (to deregister) name: \n");
				scanf(" %s", &tpdu.contentName);
				
				write(sd, &tpdu, sizeof(tpdu));
				printf("PDU Sent...\n");
				
				// Reply from Server
				while ((i = read(sd, &bp, sizeof(bp))) > 0){
					if(bp.type == 'E'){
						printf("%s\n", bp.data);
						exit(1);
					}else if(bp.type == 'A'){
						printf("Acknowledged, success!\n");
						break;
					}
				}
				break;
			case 'L':
			{
				struct dirent **list;
				int n, i = 0;
				
				if((n = scandir(".", &list, NULL, alphasort)) < 0)
					perror("scandir error");
				else{
					while(i < n){
						if(strstr(list[i]->d_name, ".txt")){
							printf("%s\n", list[i]->d_name);
							free(list[i]);
						}
						i++;
					}
					free(list);
				}
				break;
			}
			case 'D':
				// S - Search for address of corresponding content server from Index Server
				spdu.type = 'S';
				strcpy(spdu.peerName, username);
				printf("File (to download) name: \n");
				scanf(" %s", &spdu.contentName);
				write(sd, &spdu, sizeof(spdu));
				printf("PDU Sent...\n");
				
				read(sd, &dpdu, sizeof(dpdu));
				if(dpdu.type == 'E'){
					printf("Error received, file does not exist\n");
					break;
				}else if(dpdu.type == 'S'){
					// Extract address from pdu and setup TCP connection
					printf("PDU received:\nType: %c, Peer Name: %s, Content Name: %s\n", dpdu.type, dpdu.peerName, dpdu.contentName);
					printf("Port: %u\n", dpdu.data.sin_port);
					
					// Connect to content server
					int sd_tcp2;
					if((sd_tcp2 = socket(AF_INET, SOCK_STREAM, 0)) < 0){
						fprintf(stderr, "Can't create TCP socket \n");
						break;
					}
					if(connect(sd_tcp2, (struct sockaddr*)&dpdu.data, sizeof(dpdu.data)) < 0){
						printf("Failed to connect to content server\n");
						break;
					}
					
					// Request file from content server
					receiveFile(sd_tcp2, dpdu.contentName); // stuck inside here
					printf("File received\n");
										
					// Register file to content server
					sd_tcp3 = socket(AF_INET, SOCK_STREAM, 0);
					if (sd_tcp3 < 0)
						fprintf(stderr, "Can't create TCP socket \n");
					newAddress(&rpdu2, sd_tcp3); 
					printf("Port: %d\n", rpdu2.data.sin_port);
					rpdu2.type = 'R';
					strcpy(rpdu2.peerName, username);
					strcpy(rpdu2.contentName, dpdu.contentName);
					write(sd, &rpdu2, sizeof(rpdu2));
					printf("PDU Sent...\n");
					
					// Reply from Server
					while ((i = read(sd, &bp, sizeof(bp))) > 0){
						if(bp.type == 'E'){
							printf("%s\n", bp.data);
							break;
						}else if(bp.type == 'A'){
							printf("Acknowledged, success!\n");
							strcpy(registered, rpdu2.contentName);
							break;
						}
					}
					break;
				}
				break;
			case 'O':
				opdu.type = 'O';
				printf("Requesting list of online content...\n");
				write(sd, &opdu, sizeof(opdu));
				read(sd, &bp, sizeof(bp));
				if(bp.type == 'E'){
					printf("%s Empty\n", bp.data);
					break;
				}
				if(bp.type == 'O'){
					printf("%s\n", bp.data);
					break;
				}
				break;
			case 'Q':
				tpdu.type = 'T';
				strcpy(tpdu.peerName, username);
				strcpy(tpdu.contentName, registered);
				write(sd, &tpdu, sizeof(tpdu));
				while ((i = read(sd, &bp, sizeof(bp))) > 0){
					if(bp.type == 'E'){
						printf("%s Could not deregister before quitting\n", bp.data);
						exit(1);
					}else if(bp.type == 'A'){
						printf("Acknowledged, file has been deregistered\n");
						break;
					}
				}
				close(sd);
				close(sd_tcp);
				close(sd_tcp3);
				exit(0);
			default:
				printf("Switch error\n");
				exit(0);
			}
		}

		if(FD_ISSET(sd_tcp, &rfds)){ // listening to tcp = 4
			// New connection
			int new_sd;
			int c_len = sizeof(client);
			if((new_sd = accept(sd_tcp, (struct sockaddr*)&client, &c_len)) > 0){
				// Send file
				sendFile(new_sd);
			}
		}
		
		if(FD_ISSET(sd_tcp3, &rfds)){ // listening to tcp = 5
			// New connection
			int new_sd;
			int c_len = sizeof(client);
			if((new_sd = accept(sd_tcp3, (struct sockaddr*)&client, &c_len)) > 0){
				// Send file
				sendFile(new_sd);
			}
		}
	}
	close(sd_tcp);
	close(sd_tcp3);
	exit(0);
}

void sendFile(int sd){
	int i, file, fsize = 0, counter = 100;
	struct stat size;
	char* pos;
	char content[10];
	struct pdu2 dpdu, cpdu;
	
	read(sd, &dpdu, sizeof(dpdu));
	printf("PDU Received: %c\n", dpdu.type);
	
	// Determine file size
	stat(dpdu.data, &size);
	fsize = size.st_size;
	printf("Total file size = %d\n", fsize);
	
	cpdu.type = 'C';
	file = open(dpdu.data, O_RDWR); // Open File
	if(file < 0){
		write(sd, &error, sizeof(error));
		exit(1);
	}
	while((i = read(file, cpdu.data, counter)) > 0){
		write(sd, &cpdu, counter+1);
		fsize -= i;
		printf("Bytes left: %d\n", fsize);
		if((fsize/counter) == 0)
			counter = fsize;
	}
	close(file);
	close(sd);
}

void receiveFile(int sd, char* content){
	int n, file, j = 0;
	struct pdu2 dpdu, rbuf;
	
	dpdu.type = 'D';
	strcpy(dpdu.data, content);
	write(sd, &dpdu, sizeof(dpdu));
	
	while((n = read(sd, &rbuf, 101)) > 0){
		printf("PDU received: %c\n", rbuf.type);
		printf("Bytes to write: %d\n", n-1);
		if(rbuf.type == 'E'){
			printf("%s\n", rbuf.data);
			break;
		}
		if(rbuf.type != 'C'){
			printf("PDU received not Content Data\n");
			break;
		}
		if(j == 0)
	  	  	file = creat(content, 0600); // Create new
	  	else if(j > 0)
	  	 	file = open(content, O_WRONLY | O_APPEND); // Append to
	  	if(file < 0){
	  		printf("Error creating file\n");
	  		exit(1);
	  	}
	  	write(file, rbuf.data, n-1);
	  	close(file);
		j++;
		if(n < 101)
			break;
	}
	close(sd);
}

void newAddress(struct pdu1 *pdu, int sd_tcp){
	int alen;
	struct sockaddr_in reg_addr;

	memset(&reg_addr, 0, sizeof(reg_addr));
	reg_addr.sin_family = AF_INET;
	reg_addr.sin_port = htons(33000); // Random Port #	
	reg_addr.sin_addr.s_addr = inet_addr("10.35.70.32"); // IP Address of localhost
	if(bind(sd_tcp, (struct sockaddr *)&reg_addr, sizeof(reg_addr)) == -1){
		fprintf(stderr, "Can't bind name to socket: %d\n", sd_tcp);
		exit(1);
	}
	
	alen = sizeof(struct sockaddr_in);
	getsockname(sd_tcp, (struct sockaddr *) &reg_addr, &alen);
	
	if(listen(sd_tcp, 5) == -1){
		fprintf(stderr, "Listen failed\n");
		exit(1);
	};
	
	pdu->data = reg_addr;
}


