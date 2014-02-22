/*
 * commands.c
 *
 *  Created on: 11-Oct-2013
 *      Author: sriraam
 */
#include "commands.h"
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<errno.h>
#include<sys/resource.h>
#include<stdio.h>

static const char* PWD = "pwd";
static const char* WHERE = "where";
static const char* ECHO = "echo";
static const char* UNSETENV = "unsetenv";
static const char* SETENV = "setenv";
static const char* LOGOUT = "logout";
static const char* NICE = "nice";
static const char* CD = "cd";

extern char **environ;

int cd(Cmd c){
	if(c->nargs == 1){
		chdir(getenv("HOME"));
	}else{
		chdir(c->args[1]);
	}
	return 0;
}

int pwd(){
	char *returnValue=get_current_dir_name();
	printf("%s\n",returnValue);
	return 0;
}

int isBuiltInCommand(const char* command){
	if (strcmp(command, LOGOUT)==0) {
		return 1;
	}
	if (strcmp(command, SETENV)==0) {
		return 1;
	}
	if (strcmp(command, UNSETENV)==0) {
		return 1;
	}
	if (strcmp(command, ECHO)==0) {
		return 1;
	}
	if (strcmp(command, WHERE)==0) {
		return 1;
	}
	if (strcmp(command, CD)==0) {
		return 1;
	}
	if (strcmp(command, PWD)==0) {
		return 1;
	}
	if (strcmp(command, NICE)==0) {
		return 1;
	}
	return 0;
}

int executeSetEnv(Cmd c){
	char **tmp;
	int i =0;
	switch(c->nargs){
		case 3:
			//printf("%s %s\n",c->args[1],c->args[2]);
			setenv(c->args[1],c->args[2],1);
			break;
		case 2:
			setenv(c->args[1],"",1);
			break;
		case 1:
			//printf("case 1\n");
			tmp = environ;
			while(tmp[i]){
				printf("%s\n",tmp[i]);
				i++;
			}
	}
	return 0;
}

int executeUnsetEnv(Cmd c){
	unsetenv(c->args[1]);
	return 0;
}

int executeEcho(Cmd c){
	int i=1;
	for(;i<c->nargs;i++){
		if(i == c->nargs-1){
			printf("%s",c->args[i]);
		}else{
			printf("%s ",c->args[i]);
		}
	}
	printf("\n");
	return 0;
}

char ** getPaths(char *path,int *count){
	int i =0;
	char **paths;
	for(;i<strlen(path);i++){
		if(path[i]==':'){
			(*count)++;
		}
	}
	(*count)++;//current dir
	paths = (char **)malloc(sizeof(char *)*(*count)+1);
	i=0;
	paths[i++]=get_current_dir_name();
	paths[i++]=strtok(path,":");
	while(i<=*count){
		paths[i]=strtok(NULL,":");
		i++;
	}
	return paths;
}

void printWhereBuiltIns(char *command){
	if (strcmp(command, LOGOUT)==0) {
		printf("built-in %s\n", LOGOUT);
	}
	if (strcmp(command, SETENV)==0) {
		printf("built-in %s\n", SETENV);
	}
	if (strcmp(command, UNSETENV)==0) {
		printf("built-in %s\n", UNSETENV);
	}
	if (strcmp(command, ECHO)==0) {
		printf("built-in %s\n", ECHO);
	}
	if (strcmp(command, WHERE)==0) {
		printf("built-in %s\n", WHERE);
	}
	if (strcmp(command, CD)==0) {
		printf("built-in %s\n", CD);
	}
	if (strcmp(command, PWD)==0) {
		printf("built-in %s\n", PWD);
	}
	if (strcmp(command, NICE)==0) {
		printf("built-in %s\n", NICE);
	}

}
int executeWhere(Cmd c){
	int i =0;
	char *path = getenv("PATH");
	int noOfPaths=0;
	char **paths = getPaths(path,&noOfPaths);
	int count =0;
	struct stat    statbuf;
	char *tmpChar;

	printWhereBuiltIns(c->args[1]);
	while(i<noOfPaths){
		count =strlen(paths[i])+strlen(c->args[1])+2;
		tmpChar = paths[i];
		paths[i]=malloc(count*sizeof(char));
		strcat(paths[i],tmpChar);
		strcat(paths[i],"/");
		strcat(paths[i],c->args[1]);
		if(paths[i],stat(paths[i],&statbuf)==0){
	//		if(statbuf.st_mode & 0111){
				printf("%s\n",paths[i]);
	//		}
		}
		i++;
	}
	return 0;
}

int shiftArrayByN(int n, Cmd c) {
	int i=0;
	//printf("in shift %d\n",c->nargs);
	for (i = n; i < c->nargs; i++) {
		//printf("%s to %s\n",c->args[i - n],c->args[i]);
		c->args[i - n] = c->args[i];
	}
	c->nargs = c->nargs-n;
	for (i = c->nargs; i < c->nargs+n; i++) {
		c->args[i]=NULL;
	}

	//printf("after shift %d\n",c->nargs);
	return 0;
}
int isANumber(const char* word){
	int i =0;
	for(;i<strlen(word);i++){
		if(word[i] >=48 && word[i] <=57 )
			;// A Number . no problem,
		else if((word[i]==43 || word[i]==45) && i == 0)
			;// sign digit
		else{
			return 0;
		}
	}
	return 1;
}

int executeNice(Cmd c){
	int priority;
	int i =0;
	int execReturn=0;
	if(strcmp(c->args[0],"nice")==0 && c->nargs > 1 && isANumber(c->args[1])){
		if( c->nargs == 2){
			//nice #num
			setpriority(PRIO_PROCESS,getpid(),atoi(c->args[1]));
			return 0;
		}
		//printf("%s to int\n",c->args[1]);
		priority= atoi(c->args[1]);
		shiftArrayByN(2, c);
		//printf("setting priority %d\n",priority);
	//	prCmd(c);
		nice(priority);
		errno=0;
		if(executeIfBuildInSameShellCommand(c) == 0){
			//printf("same shell\n");
			closeIO(c);
			return 0;
		}
		execReturn = execvp(c->args[0],c->args);
		handleError();
	}else if(strcmp(c->args[0],"nice")==0 && c->nargs > 1){
		//printf("%s to int\n",c->args[1]);
		priority= 4;
		shiftArrayByN(1, c);
		//printf("setting priority %d\n",priority);
		//prCmd(c);
		//nice(priority);
		setpriority(PRIO_PROCESS,getpid(),priority);
		errno=0;
		if(executeIfBuildInSameShellCommand(c) == 0){
			//printf("same shell\n");
			closeIO(c);
			return 0;
		}
		execReturn = execvp(c->args[0],c->args);
		handleError();
	}
	if(strcmp(c->args[0],"nice")==0 && c->nargs == 1){
	//nice(4);
	setpriority(PRIO_PROCESS,getpid(),4);
	return 0;
	}
}


int executeIfBuildInSameShellCommand(Cmd c){
	//printf("checking for %s\n",c->args[0]);
	if (strcmp(c->args[0], LOGOUT) == 0) {
		exit(0);
	}
	if (strcmp(c->args[0], SETENV) == 0) {
		return executeSetEnv(c);
	}
	if (strcmp(c->args[0], UNSETENV) == 0) {
		return executeUnsetEnv(c);
	}
	if (strcmp(c->args[0], ECHO) == 0) {
		return executeEcho(c);
	}
	if (strcmp(c->args[0], WHERE) == 0) {
		return executeWhere(c);
	}
	if(strcmp(c->args[0],CD) == 0){
		return cd(c);
	}
	if (strcmp(c->args[0], PWD) == 0) {
		return pwd();
	}
	if (strcmp(c->args[0], NICE) == 0) {
		return executeNice(c);
	}
	return -1;
}


