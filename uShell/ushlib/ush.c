/*
 * ush.c

 *
 *  Created on: 03-Oct-2013
 *      Author: sriraam
 */

#include <stdio.h>
#include "../parse.h"
#include "ush.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include "ushio.h"
#include <errno.h>
#include "signal_handler.h"
#include <stdlib.h>


static const char* RUNCOM = ".ushrc";
static int abortPipeline = 0;
int isAtStartOrAtMiddleOfPipe(Cmd c){
	if(c->in != Tnil && c->out == Tnil){
		//last
		//printf("last %d\n",(c->in != Tnil));
		return 0;
	}
	if(isBuiltInCommand(c->args[0])){
		if(c->in == Tnil && (c->out == Tnil || (c->out != Tpipe && c->out != TpipeErr))){
			//no pipe
			return 0;
		}
	}
	if(c->in == Tnil && c->out == Tnil){
		//no pipe
		return 0;
	}
	return 1;
}
int isNiceForShell(Cmd c){
	int i=0;
	int isNumber = 1;
	//printf("nargs is %d\n",c->nargs);
	if(strcmp(c->args[0],"nice")==0 && c->nargs > 1){
		if(isANumber(c->args[1])){
			if (c->nargs == 2) {
				//printf("nice #num\n");
				// nice #num
				return 1;
			}
/*
			if(isBuiltInCommand(c->args[2])){
				//nice #num cmd
				return 1;
			}
*/
		}/*else{
			//printf("2nd not a number\n");

			if(isBuiltInCommand(c->args[1])){
				//printf("builtin\n");
				//nice cmd
				return 1;
			}

			//printf("not a builtinn\n");
		}*/
		return 0;
	}
	return 1;
}

void handleError(){
		switch(errno){
		case EACCES:
			fprintf(stderr,"permission denied\n");
			break;
		case EISDIR:
			fprintf(stderr,"permission denied\n");
			break;
		case ENOENT:
			fprintf(stderr,"command not found\n");
			break;
		}
}
int executeInternal(char *command,char **arguments,Cmd c){
	int parent =0,execReturn = 0,status = 0;

	if(!isAtStartOrAtMiddleOfPipe(c) && isNiceForShell(c)){
		//printf("check if built in\n");
		backupIO(c);
		setIO(c);
		if(executeIfBuildInSameShellCommand(c) == 0){
			//printf("same shell\n");
			restoreIO(c);
			closeIO(c);
			return 0;
		}
		restoreIO(c);
		closeIO(c);
	//	printf("not a built in\n");
	}
	//printf("forking for %s\n",c->args[0]);
	parent = fork();
	if(!parent){
		setIO(c);
		if(executeIfBuildInSameShellCommand(c) == 0){
			closeIO(c);
			exit(0);
		}
		errno=0;
		execReturn = execvp(command,arguments);
		//printf("error \n");
		handleError();
		closeIO(c);
		writeAbortFile();
		exit(-1);
		//printf("setting abort\n");
		//return 0;

	}
	else{
		wait(&status);
		kill(parent,SIGSTOP);
		if(ifChildAborted()){
			abortPipeline = 1;
		}
		closeIO(c);
	}
	return 0;
}




void initShell() {
	int iNumber;
	char *fullRC = NULL;
	char *home = getenv("HOME");//"/home/sriraam";
	fullRC = malloc(strlen(home)+strlen(RUNCOM)+2);
	strcat(fullRC,home);
	strcat(fullRC,"/");
	strcat(fullRC,RUNCOM);
	//printf("%s runcom\n",fullRC);
	iNumber = open(fullRC, O_RDONLY);
	if(iNumber != -1){
	parseFile(iNumber);
	}
	close(iNumber);
	handleSignals();
}


int setOutputPipe(int pipedInput,Cmd c,char **currentOFile){
	if(c->out == Tpipe || c->out == TpipeErr){
		if(pipedInput){
			c->outfile = TMPIFILE;
			*currentOFile=TMPIFILE;
		} else{
			c->outfile = TMPOFILE;
			*currentOFile=TMPOFILE;
			//printf("ofi %s\n",c->outfile);
		}
		pipedInput = 1;
	}else{
		pipedInput = 0;
	}
	return pipedInput;
}


int execute(Pipe pipe) {
	Cmd c;
	int pipedInput = 0;
	static char *currentOFile;
	currentOFile = TMPIFILE;
	abortPipeline = 0;
	if (pipe == NULL || pipe->head == NULL) {
		return 1;
	}
		for (c = pipe->head; c != NULL; c = c->next) {
			if ( !strcmp(c->args[0], "end") )
			      exit(0);
			if (pipedInput) {
				c->infile = currentOFile;
			}
			pipedInput = setOutputPipe(pipedInput, c, &currentOFile);
			//printf("executng %s\n",c->args[0]);
			executeInternal(c->args[0], c->args, c);
			//printf("done %d\n",abortPipeline);
			if(abortPipeline){
				//printf("aborting...\n");
				return 0;
			}
		}
	//printf("%x\n",pipe->next);
	execute(pipe->next);
	return 0;
}

