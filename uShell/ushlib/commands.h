/*
 * commands.h
 *
 *  Created on: 11-Oct-2013
 *      Author: sriraam
 */

#include "../parse.h"


/*
struct USH_CONTEXT{
	char *pwd;
	char **env;
};

typedef struct USH_CONTEXT UshContext;
*/


int executeIfBuildInCommand(Cmd c);
int applyShellContext(Cmd c);
