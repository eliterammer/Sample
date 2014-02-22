/*
 * infrastructure.c
 *
 *  Created on: 04-Nov-2013
 *      Author: sriraam
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include "infrastructure.h"
#include "players.h"
#include <string.h>
#include <stdlib.h>

int connecToSocket(struct sockaddr_in* sin, int port, struct hostent* hp) {
	/* use address family INET and STREAMing sockets (TCP) */
	int rc;
	int s;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror("socket:");
		exit(s);
	}
	//printf("socket opened\n");
	/* set up the address and port */
	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);
	memcpy(&sin->sin_addr, hp->h_addr_list[0], hp->h_length);
	//printf("connecting to %s %d\n",hp->h_name,port);
	rc = connect(s, (struct sockaddr *)sin, sizeof(*sin));
	if ( rc < 0 ) {
	  perror("connect:");
	  exit(rc);
	}
	return s;
}

static void (*errFunction)(int,char *)=NULL;
static int s_internal;
static char* msg_internal;

void setCleanTermination(void (*func)(int,char *),int s,char *msg){
errFunction=func;
s_internal=s;
msg_internal=msg;
}
int openSocket(struct sockaddr_in* sin, int port, struct hostent* hp) {
	/* use address family INET and STREAMing sockets (TCP) */
	int rc;
	int s;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror("socket:");
		if(errFunction != NULL)
		errFunction(s_internal,msg_internal);
		exit(s);
	}
	//printf("socket opened\n");
	/* set up the address and port */
	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);
	memcpy(&sin->sin_addr, hp->h_addr_list[0], hp->h_length);
	/* bind socket s to address sin */
	//printf("binding to %d\n",port);
	rc = bind(s, (struct sockaddr*) sin, sizeof(*sin));
	if (rc < 0) {
		printf("cannot bind\n");
		close(s);
		if(errFunction != NULL)
		errFunction(s_internal,msg_internal);
		exit(rc);
	}
	rc = listen(s, 5);
	if (rc < 0) {
		perror("listen:");
		if(errFunction != NULL)
		errFunction(s_internal,msg_internal);
		exit(rc);
	}
	return s;
}
int waitForConnection(int s, int len,char *buf) {
	struct sockaddr_in incoming;
	struct hostent* ihp;
	int p;
	p = accept(s, (struct sockaddr*) &incoming, &len);
	if (p < 0) {
		perror("bind:");
		exit(-1);
	}
	ihp = gethostbyaddr((char *) &incoming.sin_addr, sizeof(struct in_addr),AF_INET);
	//printf(">> Connected to %s\n", ihp->h_name);
	strcpy(buf,ihp->h_name);
	return p;
}


struct hostent* getHostFromSystem() {
	char host[64];
	struct hostent* hp=NULL;
	/* fill in hostent struct for self */
	gethostname(host, sizeof host);
	//printf("host is %s\n", host);
	hp = gethostbyname(host);
	if (hp == NULL) {
		fprintf(stderr, "host not found (%s)\n", host);
		exit(1);
	}
	return hp;
}
void sendMessageToSocket(int s, char *buf) {
	int len;
	int flag = 1;
	//printf("sending %s\n",buf);
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
	len = send(s, buf, strlen(buf), 0);

	if (len != strlen(buf)) {
		perror("send");
		exit(1);
	}
}

int getSocketOfPlayer(Player to){
	struct sockaddr_in sin;
	struct hostent *hp;
	int s;
	hp = gethostbyname(to.hostname);
	if (hp == NULL) {
		fprintf(stderr, "host not found (%s)\n", to.hostname);
		exit(1);
	}
	//printf("opening\n");
	s = connecToSocket(&sin, to.outputPort, hp);
	return s;
}
char* getMessageFromSocket(int p, char *buf,int *size) {
	char minibuf[128];
	buf[0]='\0';
	int len = 0;
	int currentLength=0;
	int bufLen = 0;
	while(1){
		//printf("waiting ..\n");
		len = recv(p, minibuf, 127, 0);
		//printf("gotsomething\n");
		if (len < 0) {
			perror("recv");
			exit(1);
		}else if(len == 0){
			return buf;
		}
		minibuf[len] = '\0';
		currentLength+=len;
		//printf("cl vs size %d vs %d\n",currentLength,*size);
		if(currentLength >= *size){
			//printf("changing size from %d to %d\n",*size,currentLength);
			buf = realloc(buf, currentLength+1);
			if(buf == NULL){
				perror("realloc failed");
				exit(-1);
			}
			//printf("current buf is %s\n",buf);
			*size = currentLength+1;
		}
		strcat(buf, minibuf);
		bufLen = strlen(buf);
	//	printf("buflen vs capacity %d %d\n",bufLen,*size);
		if (bufLen > 2) {
			if(buf[bufLen-1] == 'M' && buf[bufLen-2] == 'O' && buf[bufLen-3] == 'E'){
				buf[bufLen-3]='\0';
				break;
			}
		}

	}
	return buf;
}
