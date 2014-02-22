/*
 * players.h
 *
 *  Created on: 04-Nov-2013
 *      Author: sriraam
 */

#ifndef PLAYERS_H_
#define PLAYERS_H_

struct PLAYER{
	char hostname[512];
	int outputPort;
	int id;
};

typedef struct PLAYER Player;

#endif /* PLAYERS_H_ */
