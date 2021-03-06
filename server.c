#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <strings.h>
#include <sys/unistd.h>


/*------------------------------------------------------------------------
 * main - Iterative UDP server for TIME service
 *------------------------------------------------------------------------
 */
/*
PDU : Protocol Data Unit
*/
 
struct PDUFiles { // Struct to store File Data
	char type;
	char http_req[10];
	int source_port;
	int dest_port;
	int seq_num;
	int payload;
	char peerName[10];
	char fileName[10];
	struct sockaddr_in data;
};

struct PDUStatus { // Struct for status messages
	char type;
	int ack_num;
	char data[100];
} error, ack;
 
struct PDUFiles fileList[5];
int counter = 0;

int addToList(struct PDUFiles pdu){
	/* Adds Files to the File List as they are registered by a peer.
	Takes a File Data Unit as input, Returns 0 for success and -1 on failure */

	printf("| %-10s   / HTTP/1.1   %10s |\r\n", pdu.http_req, pdu.fileName);
	printf("| %-10i       |        %10i |\n", pdu.source_port, pdu.dest_port);
    printf("| %-10i       |        %10i |\n", pdu.seq_num, pdu.payload);


	ack.ack_num = pdu.seq_num + pdu.payload;
	error.ack_num = ack.ack_num;

	for(int i = 0; i <= counter; i++){
		if(strcmp(fileList[i].peerName, pdu.peerName) == 0 && strcmp(fileList[i].fileName, pdu.fileName) == 0){
			printf("Error: Names are the same\n");
			return -1;
		}
	}
	fileList[counter] = pdu;
	counter++;
	return 0;
}

int removeFromList(struct PDUFiles pdu){

		printf("| %-10s   / HTTP/1.1   %10s |\r\n", pdu.http_req, pdu.fileName);
	printf("| %-10i       |        %10i |\n", pdu.source_port, pdu.dest_port);
    printf("| %-10i       |        %10i |\n", pdu.seq_num, pdu.payload);

	ack.ack_num = pdu.seq_num + pdu.payload;
	error.ack_num = ack.ack_num;
	/*Removes Files from the File List as they are unregistered by the peers
	Takes a File Data Unit as input, returns 0 for success and -1 for failure*/
	for(int i = 0; i < counter; i++){


		if(strcmp(fileList[i].peerName, pdu.peerName) == 0 && strcmp(fileList[i].fileName, pdu.fileName) == 0){
			printf("PDU to remove: \nPeer Name: %s\nFile Name: %s\n", fileList[i].peerName, fileList[i].fileName);
			for(i; i <= counter; i++){
				fileList[i] = fileList[i+1];
			}
			counter--;
			return 0;
		}
	}
	return -1;
}

int searchList(struct PDUFiles *pdu){
	/*Searches for the filename that is input by a peer in the file list.
	Prints if it finds a match and exits with code 0 on succes, -1 on failure*/
	printf("Searching for PDU:\nFile Name: %s\n", pdu->fileName);
	for(int i = counter-1; i >= 0; i--){ // Loops till it finds file or exits w/ -1
		if(strcmp(fileList[i].fileName, pdu->fileName) == 0){
			printf("Match: %s = %s\n", fileList[i].fileName, pdu->fileName);
			strcpy(pdu->peerName, fileList[i].peerName);
			strcpy(pdu->fileName, fileList[i].fileName);
			pdu->data = fileList[i].data;
			return 0;
		}
	}
	return -1;
}

void List(char* list){
	for(int i = 0; i < counter; i++){ // loop through File
		int j = 0;
		for(j = 0; j < i; j++){ // loop through unique names
			if(strcmp(fileList[i].fileName, fileList[j].fileName) == 0){
				break;
			}
		}
		if(i == j){
			strcat(list, fileList[i].fileName);
			strcat(list, "\n");
		}
	}
	printf("List:\n%s\n", list);
}
	
int
main(int argc, char *argv[])
{
	struct  sockaddr_in fromAddr;	/* the from address of a client	*/
	//char	buf[100];			/* "input" buffer; any size > 0	*/
	char *pts, *filename;
	time_t now;			/* current time			*/
	int	addrLen;			/* from-address length		*/
	struct sockaddr_in servAddr; /* an Internet endpoint address         */
    int servSock, type;        /* socket descriptor and socket type    */
	int port=33000;
	int i, file;
	struct stat size;
	
                                                                                

	switch(argc){
		case 1:
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %servSock [port]\n", argv[0]);
			exit(1);
	}

        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = INADDR_ANY;
        servAddr.sin_port = htons(port);
                                                                                                 
    /* Create socket */
        servSock = socket(AF_INET, SOCK_DGRAM, 0);
        if (servSock < 0)
		fprintf(stderr, "can't create socket\n");
                                                                                
    /* Bind the socket */
        if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
		fprintf(stderr, "can't bind to %d port\n",port);
        listen(servSock, 5);	
		addrLen = sizeof(fromAddr);

	
	//error.type = 'E';
	ack.type = 'A';
	
	// ERROR Message
	char errmsg[100] = "Error!";
	error.type = 'N';
	strcpy(error.data, errmsg);
	
	while (1) {
		struct PDUFiles spdu, spdu2, buf;
		struct PDUStatus opdu;
		
		// Receive pdu
		printf("Waiting for PDU...\n");
		
		if (recvfrom(servSock, &buf, sizeof(buf), 0, (struct sockaddr *)&fromAddr, &addrLen) < 0){
			fprintf(stderr, "recvfrom error\n");
			exit(1);
		}

		
		switch(buf.type){
		case 'R':
			// Register Files to the File List
			if(addToList(buf) < 0){
				fprintf(stderr, "Error: Name already exists\n");
				sendto(servSock, &error, sizeof(error), 0, (struct sockaddr *)&fromAddr, sizeof(fromAddr));
				break;
			}
			sendto(servSock, &ack, sizeof(ack), 0, (struct sockaddr *)&fromAddr, sizeof(fromAddr));
			break;
		case 'S':
			printf("Got type S\n");
			spdu.type = 'S';
			if(searchList(&buf) < 0){
				fprintf(stderr, "Error: File name not found\n");
				sendto(servSock, &error, sizeof(error), 0, (struct sockaddr *)&fromAddr, sizeof(fromAddr));
				break;
			}
			// buf found in List
			spdu = buf;
			printf("Sending File's address...\n");
			sendto(servSock, &spdu, sizeof(spdu), 0, (struct sockaddr *)&fromAddr, sizeof(fromAddr));
			break;
		case 'L':
			printf("Got type L\n");
			spdu.type = 'L';
			if(searchList(&buf) < 0){
				fprintf(stderr, "Error: File name not found\n");
				sendto(servSock, &error, sizeof(error), 0, (struct sockaddr *)&fromAddr, sizeof(fromAddr));
				break;
			}
			// buf found in List
			spdu = buf;
			printf("Sending File'servSock address...\n");
			sendto(servSock, &spdu, sizeof(spdu), 0, (struct sockaddr *)&fromAddr, sizeof(fromAddr));
			break;
		case 'T':
			// Deregistering Files from File List
			
			if(removeFromList(buf) < 0){
				fprintf(stderr, "Error: Name does not exist\n");
				sendto(servSock, &error, sizeof(error), 0, (struct sockaddr *)&fromAddr, sizeof(fromAddr));
				break;
			}
			sendto(servSock, &ack, sizeof(ack), 0, (struct sockaddr *)&fromAddr, sizeof(fromAddr));
			break;
		case 'O':
			// Listing all files available online
			printf("| %-10s   / HTTP/1.1   %10s |\r\n", "GET", "LIST");
			printf("| %-10i       |        %10i |\n", buf.source_port, buf.dest_port);
			printf("| %-10i       |        %10i |\n", buf.seq_num, buf.payload);

			opdu.ack_num = buf.seq_num + buf.payload;
			error.ack_num = ack.ack_num;
			
			if(counter == 0){
				printf("List is empty\n");
				sendto(servSock, &error, sizeof(error), 0, (struct sockaddr *)&fromAddr, sizeof(fromAddr));
				break;
			}
			opdu.type = 'A';
			char list[100];
			list[0] = '\0';
			List(&list);
			strcpy(opdu.data, list);

			if(sendto(servSock, &opdu, sizeof(opdu), 0, (struct sockaddr *)&fromAddr, sizeof(fromAddr)) < 0){
				printf("O: send to error\n");
				exit(1);
			}
			printf("PDU sent\n");
			break;
		default:
			fprintf(stderr, "Error: No match\n");
			sendto(servSock, &error, sizeof(error), 0, (struct sockaddr *)&fromAddr, sizeof(fromAddr));
			exit(1);
		}
		printf("\n");
	}
}