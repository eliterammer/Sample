/******************************************************************************
 *
 *  File Name........: listen.c
 *
 *  Description......:
 *	Creates a program that establishes a passive socket.  The socket 
 *  accepts connection from the speak.c program.  While the connection is
 *  open, listen will print the characters that come down the socket.
 *
 *  Listen takes a single argument, the port number of the socket.  Choose
 *  a number that isn't assigned.  Invoke the speak.c program with the 
 *  same port number.
 *
 *  Revision History.:
 *
 *  When	Who         What
 *  09/02/96    vin         Created
 *
 *****************************************************************************/

/*........................ Include Files ....................................*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
//
#include "players.h"
#include <time.h>
#include "infrastructure.h"

void terminatePlayer(Player whom){
	struct sockaddr_in sin;
	struct hostent *hp;
	int s;
	char buf[512];
//	printf("closing %d\n",whom.outputPort);
	hp = gethostbyname(whom.hostname);
	if (hp == NULL) {
		fprintf(stderr, "host not found (%s)\n", whom.hostname);
		exit(1);
	}
	//printf("opening\n");
	s = connecToSocket(&sin, whom.outputPort, hp);
	sprintf(buf, "EOGEOM");
	//printf("sending %s\n", buf);
	sendMessageToSocket(s, buf);
	close(s);
}


void terminatePlayers(int till, Player players[]) {
	int counter = 0;
	//printf("except is %d\n",except);
	for (counter = 0; counter < till; counter++) {
		terminatePlayer(players[counter]);
	}
}

//
void terminateAllOtherPlayers(int except,int size,Player players[]){
	int counter = 0;
	//printf("except is %d\n",except);
	for (counter = 0; counter < size; counter++) {
		if(counter != except){
			terminatePlayer(players[counter]);
		}
	}

}
void introduce(Player *to, Player whom,int playerId,int whomId) {
	struct sockaddr_in sin;
	struct hostent *hp;
	int s;
	char buf[512];
	char *EOM = "EOM";
	hp = gethostbyname(to->hostname);
	if (hp == NULL) {
		fprintf(stderr, "host not found (%s)\n", to->hostname);
		exit(1);
	}
	//printf("opening\n");
	s = connecToSocket(&sin, to->outputPort, hp);
	sprintf(buf, "%d %s %d %d%s",playerId, whom.hostname,whom.outputPort,whomId,EOM);
	//printf("sending %s\n", buf);
	sendMessageToSocket(s, buf);
	close(s);
}
void introduceNeighbours(Player players[], int size) {
	int counter = 0;
	int index = 0;
	for (counter = 0; counter < size; counter++) {
		index = (counter + size - 1) % size;
		//printf("introducing %d to %d as left\n", counter, index);
		// introduce index to counter
		introduce(&players[counter], players[index],counter,index);
	}

	for (counter = 0; counter < size; counter++) {
		index = (counter + 1) % size;
		//printf("introducing %d to %d as right\n", counter, index);
		// introduce index to counter
		introduce(&players[counter], players[index],counter,index);
	}
}

int getPortNumberFromInput(int argc, char* argv[]) {
	/* read port number from command line */
	if (argc < 4) {
		fprintf(stderr, "Usage: %s <port-number> <number-of-players> <hops>\n", argv[0]);
		exit(1);
	}
	return atoi(argv[1]);
}


int main(int argc, char *argv[]) {
	char *buf;
	int s, p,tmpSock, len, port;
	struct hostent *hp;
	struct sockaddr_in sin;

	///
	int SIZE_OF_RING = 3;
	int noOfClients = 0;
	int randomPlayer;
	int SIZE_OF_BUFFER = 512;
	int fds[SIZE_OF_RING];
	int hops=0;
	///

	/* read port number from command line */
	port = getPortNumberFromInput(argc, argv);

	SIZE_OF_RING = atoi(argv[2]);
	if(SIZE_OF_RING < 2){
		printf("No of players cannot be less than 2\n");
		return 0;
	}

	Player players[SIZE_OF_RING];
	buf = malloc(SIZE_OF_BUFFER);
	hops = atoi(argv[3]);
	if(hops < 0){
		printf("no of hops cannot be negative\n");
		close(s);
		return 0;
	}




	//printf("hops is %d\n",hops);
	/* fill in hostent struct for self */
	hp = getHostFromSystem();

	printf("Potato Master on %s\n",hp->h_name);
    printf("Players = %d\n",SIZE_OF_RING);
	printf("Hops = %d\n",hops);

	/* use address family INET and STREAMing sockets (TCP) */
	s = openSocket(&sin, port, hp);
	len = sizeof(sin);
	/* accept connections */
	while (noOfClients < SIZE_OF_RING) {
		p = waitForConnection(s, len, buf);
		strcpy(players[noOfClients].hostname, buf);
		/* read and print strings sent over the connection */
		buf = getMessageFromSocket(p, buf, &SIZE_OF_BUFFER);
		//printf("port num is %d\n", atoi(buf));
		if(!strcmp(buf,"err")){
			printf("More than one player tried to bind with the same port\nTearing down game\n");
			terminatePlayers(noOfClients,players);
			close(p);
			close(s);
			exit(1);
		}
		players[noOfClients].outputPort = atoi(buf);
		printf("player %d is on %s\n",noOfClients,players[noOfClients].hostname);
		tmpSock=getSocketOfPlayer(players[noOfClients]);
		sprintf(buf,"%dEOM",noOfClients);
		sendMessageToSocket(tmpSock,buf);
		close(tmpSock);
		fds[noOfClients++]=p;
	//	close(p);
	}

	if(hops == 0){
		//buf[0]='\0';
		//printf("Trace of potato:\n%s\n",buf);
		terminateAllOtherPlayers(SIZE_OF_RING,SIZE_OF_RING,players);
		close(s);
		return 0;

	}

	//printf("all players are ready\n");
	introduceNeighbours(players, SIZE_OF_RING);
	srand(time(NULL));
	randomPlayer = rand() % SIZE_OF_RING;
	printf("All players present, sending potato to player %d\n", randomPlayer);
	p = getSocketOfPlayer(players[randomPlayer]);
	sprintf(buf,"%dEOM",hops);
	//printf("start with %s\n",buf);
	sendMessageToSocket(p, buf);
	close(p);
	//printf("waiting for game to get over\n");
	fd_set fdSet;
	FD_ZERO(&fdSet);
	int i;
	int max=-1;
	for(i = 0;i<SIZE_OF_RING;i++){
		FD_SET(fds[i],&fdSet);
		if(max < fds[i]){
			max = fds[i];
		}
	}
	p = select(max+1,&fdSet,NULL,NULL,NULL);
	if(p > 0){
		for(i = 0;i<SIZE_OF_RING;i++){
			if(FD_ISSET(fds[i],&fdSet)){
				buf = getMessageFromSocket(fds[i],buf,&SIZE_OF_BUFFER);
				break;
			}
		}
	}

	printf("Trace of potato:\n%s\n",buf);
	terminateAllOtherPlayers(i,SIZE_OF_RING,players);
	//printf(">> Connection closed\n");
	close(s);
	return 0;
}

/*........................ end of listen.c ..................................*/
