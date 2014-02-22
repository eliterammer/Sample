/*
 * infrastructure.h
 *
 *  Created on: 04-Nov-2013
 *      Author: sriraam
 */

#ifndef INFRASTRUCTURE_H_
#define INFRASTRUCTURE_H_
#include <stdlib.h>
#include "players.h"

int connecToSocket(struct sockaddr_in* sin, int port, struct hostent* hp);
int openSocket( struct sockaddr_in* sin, int port, struct hostent* hp);
struct hostent* getHostFromSystem();
char* getMessageFromSocket(int p, char *buf,int *size);
void sendMessageToSocket(int s, char *buf);
int waitForConnection(int s, int len,char *buf);
int getSocketOfPlayer(Player to);

#endif /* INFRASTRUCTURE_H_ */
