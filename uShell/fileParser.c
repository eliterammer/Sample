#include "fileParser.h"
#include "parse.h"
#include<unistd.h>
#include <sys/wait.h>
#include<stdio.h>

void parseFile(int iNumber){
	int retVal=0;
	int stdinCpy = dup(0);
		dup2(iNumber, 0);
		while (retVal != 1) {
			Pipe p = parse();
		//	prPipe(p);
			retVal = execute(p);
		}
		dup2(stdinCpy,0);
		return;
}

