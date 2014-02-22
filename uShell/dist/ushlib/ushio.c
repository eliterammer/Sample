/*
 * ushio.c


 *
 *  Created on: 11-Oct-2013
 *      Author: sriraam
 */
#include<stdlib.h>
#include<fcntl.h>
#include "../parse.h"
#include "ushio.h"
#include<stdio.h>
#include <unistd.h>
#include <sys/stat.h>

static int stdinBkp,stdoutBkp;

static const char *ABORT_FILE = ".tmpAbortFile";
void closeOp(Cmd c){
	if((c->out != Tnil)){
		//close(c->outfile);
		//close(1);
		//printf("closed o %s\n",c->outfile);
		if(strcmp(c->outfile,TMPIFILE) == 0 || strcmp(c->outfile,TMPOFILE) == 0){
			c->outfile = NULL;
		}
	}
}

void closeIp(Cmd c){
	if((c->in != Tnil)){
		//close(c->infile);
		//printf("closed i %s\n",c->infile);
		if(!c->infile)
		if(strcmp(c->infile,TMPIFILE) == 0 || strcmp(c->infile,TMPOFILE) == 0){
			c->infile = NULL;
		}
	}
}

void writeAbortFile(){
	open(ABORT_FILE,O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR);
	return;
}

int ifChildAborted(){

	struct stat    statbuf;
	if(stat(ABORT_FILE,&statbuf) ==0){
		remove(ABORT_FILE);
		return 1;
	}
	return 0;
}

void closeIO(Cmd c){
	closeOp(c);
	closeIp(c);
}
void backupIO(Cmd c){
	if((c->out != Tnil)){
		stdoutBkp = dup(1);
	}
	if((c->in != Tnil)){
		stdinBkp = dup(0);
	}
}

void restoreIO(Cmd c){
	if((c->out != Tnil)){
		fflush(stdout);
		 dup2(stdoutBkp,1);
	}
	if((c->in != Tnil)){
		dup2(stdinBkp,0);
	}
}
void setIO(Cmd c){
	if((c->out != Tnil)){
		//printf("opening for o %s\n",c->outfile);
		if(c->out == Tapp){
			//printf("1");
			dup2(open(c->outfile,O_WRONLY|O_APPEND|O_CREAT,S_IRUSR|S_IWUSR),1);
		}
		else if(c->out == TappErr){
			//printf("2");
			dup2(open(c->outfile,O_WRONLY|O_APPEND|O_CREAT,S_IRUSR|S_IWUSR),2);
		}
		else if(c->out == ToutErr){
			//printf("3");
			dup2(open(c->outfile,O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR),2);
		}
		else{
			//printf("writeonly\n");
			dup2(open(c->outfile,O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR),1);
		}


		if(c->out == TpipeErr){
			//printf("err pipe\n");
			dup2(open(c->outfile,O_WRONLY|O_APPEND|O_CREAT),2);
		}
		//printf("dup over\n");
	}

	if((c->in != Tnil)){
		//printf("opening for i %s\n",c->infile);
		dup2(open(c->infile,O_RDONLY),0);
	}
}
