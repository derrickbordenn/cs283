#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
	char *token;
	char *saveptr;
	int num_cmds = 0;

	//set everything in the clist memory block to 0
	memset(clist, 0, sizeof(command_list_t));


	token = strtok_r(cmd_line, PIPE_STRING, &saveptr);
	while (token != NULL){

		//trim leading spaces
		while (*token == SPACE_CHAR) {
			token++;
		}

		//trim trailing spaces
		char *end = token + strlen(token) - 1;
		while (end > token && *end == SPACE_CHAR) {
			*end-- = '\0';
		}
		if (num_cmds >= CMD_MAX) {
			return ERR_TOO_MANY_COMMANDS;
		}

		command_t *cmd = &clist->commands[num_cmds];
		char *argptr;

		//get the executable name (first command)
		argptr = strtok(token, " ");

		if (argptr == NULL || strlen(argptr) >= EXE_MAX) {
			return ERR_CMD_OR_ARGS_TOO_BIG;
		}
		strcpy(cmd->exe, argptr);
		
		//get the arguments (rest of commands)
		argptr = strtok(NULL, "");
		if (argptr != NULL && strlen(argptr) >= ARG_MAX) {
			return ERR_CMD_OR_ARGS_TOO_BIG;
		}
		if (argptr) {
			//print the arguments of the executable
			snprintf(cmd->args, ARG_MAX, "[%s]", argptr);
		} else {
			cmd->args[0] = '\0';
		}

		num_cmds++;
		token = strtok_r(NULL, PIPE_STRING, &saveptr);
	}
	clist->num = num_cmds;
	
	if (num_cmds == 0) {
		printf(CMD_WARN_NO_CMD);
		return WARN_NO_CMDS;
	}
	
	//print and format strings according to test file
	printf(CMD_OK_HEADER, num_cmds);
	for (int i = 0; i < num_cmds; i++) {
		if (strlen(clist->commands[i].args) > 0) {
			printf("<%d> %s %s\n", i + 1, clist->commands[i].exe, clist->commands[i].args);
		} else {
			printf("<%d> %s\n", i + 1, clist->commands[i].exe);
		}
	}

	return OK;
}
