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
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    int commandCount = 0;
    command_t* command = (command_t*) malloc(sizeof(command_t));

    stripLTWhiteSpace(cmd_line);

    char* cmd_line_copy = strdup(cmd_line);
    char* commandString = strtok(cmd_line_copy, PIPE_STRING);

    if (commandString == NULL) {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }

    char* commandCopy = malloc(strlen(commandString) + 1);
    memcpy(commandCopy, commandString, strlen(commandString) + 1);

    int rc = buildCommand(commandCopy, command);
    commandCount++;

    if (rc == ERR_CMD_OR_ARGS_TOO_BIG) {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    } else {

        // save first command, then move on to others
        memcpy(&clist->commands[clist->num], command, sizeof(command_t));
        clist->num++;
        free(command);

        while ((commandString = strtok(NULL, PIPE_STRING)) != NULL) {
            printf("got here\n");

            commandCount++;

            if (commandCount > CMD_MAX) {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            char* commandCopy = malloc(strlen(commandString) + 1);
            memcpy(commandCopy, commandString, strlen(commandString) + 1);
            
            command_t* newCommand = (command_t*) malloc(sizeof(command_t));
            printf("new command: %s\n", commandCopy);
            rc = buildCommand(commandCopy, newCommand);

            if (rc == ERR_CMD_OR_ARGS_TOO_BIG) {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            
            memcpy(&clist->commands[clist->num], command, sizeof(command_t));
            clist->num++;

            free(newCommand);
        }
    }

    return OK;
}


/*  - Takes in command_t pointer (already allocated
*   - Parses command and popoulates command_t
*   - Error checks as it goes
*   - Will return either 'OK' or 'ERR_CMD_OR_ARGS_TOO_BIG' 
*/
int buildCommand(char* commandString, command_t* commandStruct) {
    int argsLength = 0;
    char* normalizedArgs = malloc(ARG_MAX);
    int endIndex = 0;

    // get rid of leading and trailing whitespace
    // convert tabs to spaces to make conversion easier
    stripLTWhiteSpace(commandString);
    for (size_t i = 0; i < strlen(commandString); i++) {
        if (commandString[i] == '\t') {
            commandString[i] = SPACE_CHAR;
        }
    }


    // isolate exe arg
    char* token = strtok(commandString, SPACE_STRING);

    if (strlen(token) > EXE_MAX) {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }

    strcpy(commandStruct->exe, token);


    // isolate and 'normalize' args by removing whitespace and writing to new 
    // arg buffer separating by a single space between each arg
    while (token != NULL) {
        token = strtok(NULL, SPACE_STRING);

        // we are done so break
        if (token == NULL) {
            break;
        }

        stripLTWhiteSpace(token);

        argsLength += strlen(token);

        if (argsLength > ARG_MAX) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        int tokenIndex = 0;
        while (*(token + tokenIndex) != '\0') {
            *(normalizedArgs + endIndex) = *(token + tokenIndex);
            endIndex++;
            tokenIndex++;
        }

        *(normalizedArgs + endIndex) = SPACE_CHAR;
        endIndex++;
    }

    stripLTWhiteSpace(normalizedArgs);

    strcpy(commandStruct->args, normalizedArgs);

    return OK;
}


/*  - Takes in character pointer
*   - Removes all LEADING and TRAILING whitespace only
*   - Modifies in place
*/  
void stripLTWhiteSpace(char* string) {
    if (*string == '\0') {
        return;
    }

    // Find the first non-whitespace character
    char *start = string;
    while (*start && (isspace((unsigned char) *start) || *start == '\t')) {
        start++;
    }

    // Find the last non-whitespace character
    char *end = string + strlen(string) - 1;
    while (end > start && (isspace((unsigned char) *end) || *end == '\t')) {
        end--;
    }

    if (start != string) {
        char* overwritePtr = string;
        while (start <= end) {
            *overwritePtr = *start;
            overwritePtr++;
            start++;
        }
        *overwritePtr = '\0'; 
    } else {
        *(end + 1) = '\0'; 
    }
}

void getCommandToken(char* cmdLine, char* tokenBuffer) {
    int pipeIndex = strcspn(cmdLine, PIPE_STRING);
    
    if (pipeIndex == sizeof(cmdLine) / sizeof(char)) {
        tokenBuffer = NULL;
        return;
    }

    int cpyIndex = 0;
    while (cpyIndex < pipeIndex) {
        tokenBuffer[cpyIndex] = cmdLine[cpyIndex];
        cpyIndex++;
    }

    pipeIndex++;
    cpyIndex = 0;
    while (*cmdLine) {
        cmdLine[cpyIndex] = cmdLine[pipeIndex];
        pipeIndex++;
        cpyIndex++;
    }
    cmdLine[pipeIndex] = '\0';
    return;
}