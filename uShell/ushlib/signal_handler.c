/*
 * signal_handler.c
 *
 *  Created on: 16-Oct-2013
 *      Author: sriraam
 */
#include "signal_handler.h"
#include<stdio.h>

void doNothing(){
}
void handleSignals(){
	signal(SIGTERM,doNothing);
	signal(SIGQUIT,doNothing);
	signal(SIGHUP,doNothing);
	signal(SIGINT,doNothing);
	signal(SIGTSTP,doNothing);
}
