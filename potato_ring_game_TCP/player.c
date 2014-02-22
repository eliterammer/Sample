/******************************************************************************
 *
 *  File Name........: speak.c
 *
 *  Description......:
 *	Create a process that talks to the listen.c program.  After 
 *  connecting, each line of input is sent to listen.
 *
 *  This program takes two arguments.  The first is the host name on which
 *  the listen process is running. (Note: listen must be started first.)
 *  The second is the port number on which listen is accepting connections.
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

#include "infrastructure.h"
#include "players.h"
#include <time.h>
#include <unistd.h>
#define LEN	64

static Player left;
static Player right;
static int playedId=100;
void checkInputs(int argc, char* argv[]) {
	/* read host and port number from command line */
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <host-name> <port-number>\n", argv[0]);
		exit(1);
	}
}

void getPlayerIdFromMaster(int s){
	int SIZE_OF_BUFFER=512;
	char buf[SIZE_OF_BUFFER];
	int p =waitForConnection(s,sizeof(struct sockaddr_in),buf);
	getMessageFromSocket(p,buf,&SIZE_OF_BUFFER);
	playedId = atoi(buf);
	close(p);
}
void sendIntroduction(int s,int ownPort){
	char buf[512];
	sprintf(buf,"%d",ownPort);
	sendMessageToSocket(s,buf);
	sendMessageToSocket(s,"EOM");
}
void getNeighbour(int s,Player *neighbour){
	int SIZE_OF_BUFFER=512;
	char buf[SIZE_OF_BUFFER];
	char *tmp;

	int p =waitForConnection(s,sizeof(struct sockaddr_in),buf);

	getMessageFromSocket(p,buf,&SIZE_OF_BUFFER);
	if(!strcmp(buf,"EOG")){
		close(p);
		close(s);
		exit(1);
	}
	//printf("neighbour is %s\n",buf);
	tmp = strtok(buf," ");
	playedId = atoi(tmp);

	//printf("player id is %d\n",playedId);
	tmp = strtok(NULL," ");
	strcpy(neighbour->hostname,tmp);
	//printf("hostname is %s\n",neighbour->hostname);
	tmp = strtok(NULL," ");
	neighbour->outputPort = atoi(tmp);
	//printf("neigh outport is %d\n",neighbour->outputPort);
	tmp = strtok(NULL," ");
	neighbour->id=atoi(tmp);
	//printf("neigh id is %d\n",neighbour->id);

	//printf("neighbour is %s %d %d\n",neighbour->hostname,neighbour->outputPort,neighbour->id);
	close(p);
}

int openListenerSocket(int ownPort){
	int s;
	struct hostent *hp;
	struct sockaddr_in sin;
	hp = getHostFromSystem();
	s = openSocket(&sin,ownPort,hp);
	return s;
}
void playTheGame(int s,int myPort,int ringMaster){
	int SIZE_OF_BUFFER = 512;
	char *buf;
	char *space;
	int hops=0;
	char *currentGame;
	int randomPlayer;

	buf = malloc(SIZE_OF_BUFFER);
	int p =waitForConnection(s,sizeof(struct sockaddr_in),buf);
	buf = getMessageFromSocket(p,buf,&SIZE_OF_BUFFER);
	//printf("Potato is %s\n",buf);
	if(!strcmp(buf,"EOG")){
		// server signals end of game
		//printf("End of game\n");
		close(p);
		return;
	}
	space = strstr(buf," ");
	currentGame = malloc(SIZE_OF_BUFFER+100); //100 for ,IdEOM
	if(currentGame == NULL){
		perror("malloc failed");
		exit(-1);
	}
	if(space == NULL){
		hops = atoi(buf);
	}else{
		buf[space-buf]='\0';
		strcpy(currentGame,buf);
		hops = atoi(space+1);
	}
	hops--;
	//printf("hops left %d\n",hops);
	if(hops == 0){
		printf("I'm it\n");
		if(strlen(currentGame)!=0){
			sprintf(currentGame,"%s,%dEOM",currentGame,playedId);
		}else{
			sprintf(currentGame,"%dEOM",playedId);
		}
		sendMessageToSocket(ringMaster,currentGame);
		close(p);
		free(buf);
		free(currentGame);
		return;
	}else{
		if(strlen(currentGame)!=0){
		/*	if(hops < 2200){
				printf("%d %d\n",strlen(currentGame),SIZE_OF_BUFFER+100);
			}
		*/	sprintf(currentGame,"%s,%d %dEOM",currentGame,playedId,hops);
		//	printf("actual len %d\n",strlen(currentGame));
		}else{
			sprintf(currentGame,"%d %dEOM",playedId,hops);
		}
	}
	randomPlayer = rand()%2;
	printf("Sending potato to %d\n",randomPlayer == 0 ?left.id:right.id);
	close(p);
	//printf("get sock after closing\n");
	p=getSocketOfPlayer(randomPlayer == 0 ?left:right);
	//printf("sending\n");
	sendMessageToSocket(p,currentGame);
	//printf("sent\n");
	close(p);
	free(buf);
	free(currentGame);
	playTheGame(s,myPort,ringMaster);
}
int main (int argc, char *argv[])
{
  int s, port,own;
  char host[LEN];
  struct hostent *hp;
  struct sockaddr_in sin;
  int ownPort;
  char buf[128];
  int bufSize = 128;
  checkInputs(argc, argv);
  /* read host and port number from command line */
  srand(time(NULL));
  
  /* fill in hostent struct */
  hp = gethostbyname(argv[1]); 
  if ( hp == NULL ) {
    fprintf(stderr, "%s: host not found (%s)\n", argv[0], host);
    exit(1);
  }
  port = atoi(argv[2]);
  s= connecToSocket(&sin,port,hp);
  setCleanTermination(sendMessageToSocket,s,"errEOM");
  srand(time(NULL));
  ownPort = rand()%100+65400;
  own = openListenerSocket(ownPort);
  //printf("listener socket opened\n");
  sendIntroduction(s,ownPort);
  getPlayerIdFromMaster(own);
  printf("Connected as player %d\n",playedId);

 // close(s);
  //printf("intro sent\n");
  getNeighbour(own,&left);
  getNeighbour(own,&right);
  //printf("neighbours got\n");
  //s= connecToSocket(&sin,port,hp);
  playTheGame(own,ownPort,s);
  //printf("done\n");
  close(own);
  close(s);
  return 0;
}

/*........................ end of speak.c ...................................*/
