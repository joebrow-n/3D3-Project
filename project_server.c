/* time_server.c - main */

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
 
struct pdu1 contentList[5];
int counter = 0;

int addToList(struct pdu1 pdu){
	printf("PDU to add:\nType: %c\nPeer Name: %s\nContent Name: %s\n", pdu.type, pdu.peerName, pdu.contentName);
	printf("Port: %u\n", pdu.data.sin_port);
	for(int i = 0; i <= counter; i++){
		if(strcmp(contentList[i].peerName, pdu.peerName) == 0 && strcmp(contentList[i].contentName, pdu.contentName) == 0){
			printf("Error: Names are the same\n");
			return -1;
		}
	}
	contentList[counter] = pdu;
	counter++;
	return 0;
}

int removeFromList(struct pdu1 pdu){
	for(int i = 0; i < counter; i++){
		if(strcmp(contentList[i].peerName, pdu.peerName) == 0 && strcmp(contentList[i].contentName, pdu.contentName) == 0){
			printf("PDU to remove: \nPeer Name: %s\nContent Name: %s\n", contentList[i].peerName, contentList[i].contentName);
			for(i; i <= counter; i++){
				contentList[i] = contentList[i+1];
			}
			counter--;
			return 0;
		}
	}
	return -1;
}

int searchList(struct pdu1 *pdu){
	printf("Searching for PDU:\nContent Name: %s\n", pdu->contentName);
	for(int i = counter-1; i >= 0; i--){
		if(strcmp(contentList[i].contentName, pdu->contentName) == 0){
			printf("Match: %s = %s\n", contentList[i].contentName, pdu->contentName);
			strcpy(pdu->peerName, contentList[i].peerName);
			strcpy(pdu->contentName, contentList[i].contentName);
			pdu->data = contentList[i].data;
			return 0;
		}
	}
	return -1;
}

void List(char* list){
	for(int i = 0; i < counter; i++){ // loop through content
		int j = 0;
		for(j = 0; j < i; j++){ // loop through unique names
			if(strcmp(contentList[i].contentName, contentList[j].contentName) == 0){
				break;
			}
		}
		if(i == j){
			strcat(list, contentList[i].contentName);
			strcat(list, "\n");
		}
	}
	printf("List:\n%s\n", list);
}
	
int
main(int argc, char *argv[])
{
	struct  sockaddr_in fsin;	/* the from address of a client	*/
	//char	buf[100];		/* "input" buffer; any size > 0	*/
	char    *pts, *filename;
	int	sock;			/* server socket		*/
	time_t	now;			/* current time			*/
	int	alen;			/* from-address length		*/
	struct  sockaddr_in sin; /* an Internet endpoint address         */
        int     s, type;        /* socket descriptor and socket type    */
	int 	port=3000;
	int i, file;
	struct stat size;
	
                                                                                

	switch(argc){
		case 1:
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);
                                                                                                 
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "can't create socket\n");
                                                                                
    /* Bind the socket */
        if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "can't bind to %d port\n",port);
        listen(s, 5);	
	alen = sizeof(fsin);

	
	error.type = 'E';
	ack.type = 'A';
	
	// ERROR Message
	char errmsg[100] = "Error!";
	error.type = 'E';
	strcpy(error.data, errmsg);
	
	while (1) {
		struct pdu1 spdu, spdu2, buf;
		struct pdu2 opdu;
		
		// Recive pdu
		printf("Receiving PDU...\n");
		if (recvfrom(s, &buf, sizeof(buf), 0,	(struct sockaddr *)&fsin, &alen) < 0){
			fprintf(stderr, "recvfrom error\n");
			exit(1);
		}
		
		switch(buf.type){
		case 'R':
			printf("Got type R\n");
			if(addToList(buf) < 0){
				fprintf(stderr, "Error: Name already exists\n");
				sendto(s, &error, sizeof(error), 0, (struct sockaddr *)&fsin, sizeof(fsin));
				break;
			}
			sendto(s, &ack, sizeof(ack), 0, (struct sockaddr *)&fsin, sizeof(fsin));
			break;
		case 'S':
			printf("Got type S\n");
			spdu.type = 'S';
			if(searchList(&buf) < 0){
				fprintf(stderr, "Error: Content name not found\n");
				sendto(s, &error, sizeof(error), 0, (struct sockaddr *)&fsin, sizeof(fsin));
				break;
			}
			// buf found in List
			spdu = buf;
			printf("Sending content's address...\n");
			sendto(s, &spdu, sizeof(spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin));
			break;
		case 'T':
			printf("Got type T\n");
			if(removeFromList(buf) < 0){
				fprintf(stderr, "Error: Name does not exist\n");
				sendto(s, &error, sizeof(error), 0, (struct sockaddr *)&fsin, sizeof(fsin));
				break;
			}
			sendto(s, &ack, sizeof(ack), 0, (struct sockaddr *)&fsin, sizeof(fsin));
			break;
		case 'O':
			printf("Got type O\n");
			if(counter == 0){
				printf("List is empty\n");
				sendto(s, &error, sizeof(error), 0, (struct sockaddr *)&fsin, sizeof(fsin));
				break;
			}
			opdu.type = 'O';
			char list[100];
			list[0] = '\0';
			List(&list);
			strcpy(opdu.data, list);
			printf("%s\n", opdu.data);
			if(sendto(s, &opdu, sizeof(opdu), 0, (struct sockaddr *)&fsin, sizeof(fsin)) < 0){
				printf("O: send to error\n");
				exit(1);
			}
			printf("PDU sent\n");
			break;
		default:
			fprintf(stderr, "Error: No match\n");
			sendto(s, &error, sizeof(error), 0, (struct sockaddr *)&fsin, sizeof(fsin));
			exit(1);
		}
		printf("\n");
	}
}
