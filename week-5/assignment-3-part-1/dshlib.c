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
    char* commandBuffer = malloc(SH_CMD_MAX);
    memset(commandBuffer, 0, SH_CMD_MAX);
    command_t* command = (command_t*) malloc(sizeof(command_t));

    stripLTWhiteSpace(cmd_line);

    int tokenRC = getTruncToken(cmd_line, commandBuffer, PIPE_STRING);

    if (tokenRC == -1) {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }


    int buildRC = buildCommand(commandBuffer, command);
    free(commandBuffer);
    commandCount++;

    if (buildRC == ERR_CMD_OR_ARGS_TOO_BIG) {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    } else {

        // save first command, then move on to others
        memcpy(&clist->commands[clist->num], command, sizeof(command_t));
        clist->num++;
        free(command);

        while (strlen(commandBuffer) != 0) {
            char* commandBuffer = malloc(SH_CMD_MAX);
            memset(commandBuffer, 0, SH_CMD_MAX);
            tokenRC = getTruncToken(cmd_line, commandBuffer, PIPE_STRING);

            if (strlen(commandBuffer) == 0) {
                break;
            }

            commandCount++;

            if (commandCount > CMD_MAX) {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            
            command_t* newCommand = (command_t*) malloc(sizeof(command_t));
            buildRC = buildCommand(commandBuffer, newCommand);
            free(commandBuffer);

            if (buildRC == ERR_CMD_OR_ARGS_TOO_BIG) {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            
            memcpy(&clist->commands[clist->num], newCommand, sizeof(command_t));
            clist->num++;

            free(newCommand);
        }
    }

    return OK;
}


/*  - Takes in command_t pointer (already allocated)
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


// Given an input string and a pointer to a buffer as well as a delimiter
// this function will copy the characters from the input string to the tokenBuffer
// until the delimiter is reached. 
// The inputString is modified in place so the chracters that were copied shifts the pointer over in 
// memory to resume reading after the last occurrence of the delimiter. 
int getTruncToken(char* inputString, char* tokenBuffer, char* delimiter) {
    size_t delimiterIndex = strcspn(inputString, delimiter);
    size_t cpyIndex = 0;
    
    if (delimiterIndex == strlen(inputString)) {
        while (inputString[cpyIndex] != '\0') {  
            tokenBuffer[cpyIndex] = inputString[cpyIndex];  
            cpyIndex++;
        }
        tokenBuffer[cpyIndex] = '\0'; 
        inputString[0] = '\0';  
    
        inputString[0] = '\0';  // Empty the inputString for the next token
        return 0;
    }

    // Copy the first command into tokenBuffer
    while (cpyIndex < delimiterIndex) {
        tokenBuffer[cpyIndex] = inputString[cpyIndex];
        cpyIndex++;
    }
    tokenBuffer[cpyIndex] = '\0';

    // Shift cmdLine forward past the delimiter
    size_t newInputStringIndex = 0;
    delimiterIndex++;
    
    while (inputString[delimiterIndex] != '\0') {
        inputString[newInputStringIndex] = inputString[delimiterIndex];
        newInputStringIndex++;
        delimiterIndex++;
    }
    inputString[newInputStringIndex] = '\0'; 

    return 0; 
}


// implements extra credit dragon function
// NOTE: the compression I used here was by
// using a python script to generate a static
// sequence of tokens, each with a pattern followed by how many times
// that pattern repeats. Newlines are denoted 'n'.
// this gets expanded and printed to stdout
void printDragon() {
    char* compressedDragon[] = {" 72", "@1", "%4", " 23", "n", " 69", "%6", " 25", "n", " 68", "%6", " 26", "n", " 65", "%1", " 1", "%7", " 11", "@1", " 14", "n", " 64", "%10", " 8", "%7", " 11", "n", " 39", "%7", " 2", "%4", "@1", " 9", "%12", "@1", " 4", "%6", " 2", "@1", "%4", " 8", "n", " 34", "%22", " 6", "%28", " 10", "n", " 32", "%26", " 3", "%12", " 1", "%15", " 11", "n", " 31", "%29", " 1", "%19", " 5", "%3", " 12", "n", " 29", "%28", "@1", " 1", "@1", "%18", " 8", "%2", " 12", "n", " 28", "%33", " 1", "%22", " 16", "n", " 28", "%58", " 14", "n", " 28", "%50", "@1", "%6", "@1", " 14", "n", " 6", "%8", "@1", " 11", "%16", " 8", "%26", " 6", "%2", " 16", "n", " 4", "%13", " 9", "%2", "@1", "%12", " 11", "%11", " 1", "%12", " 6", "@1", "%1", " 16", "n", " 2", "%10", " 3", "%3", " 8", "%14", " 12", "%24", " 24", "n", " 1", "%9", " 7", "%1", " 9", "%13", " 13", "%12", "@1", "%11", " 23", "n", "%9", "@1", " 16", "%1", " 1", "%13", " 12", "@1", "%25", " 21", "n", "%8", "@1", " 17", "%2", "@1", "%12", " 12", "@1", "%28", " 18", "n", "%7", "@1", " 19", "%15", " 11", "%33", " 14", "n", "%10", " 18", "%15", " 10", "%35", " 6", "%4", " 2", "n", "%9", "@1", " 19", "@1", "%14", " 9", "%12", "@1", " 1", "%4", " 1", "%17", " 3", "%8", "n", "%10", " 18", "%17", " 8", "%13", " 6", "%18", " 1", "%9", "n", "%9", "@1", "%2", "@1", " 16", "%16", "@1", " 7", "%14", " 5", "%24", " 2", "%2", "n", " 1", "%10", " 18", "%1", " 1", "%14", "@1", " 8", "%14", " 3", "%26", " 1", "%2", "n", " 2", "%12", " 2", "@1", " 11", "%18", " 8", "%40", " 2", "%3", " 1", "n", " 3", "%13", " 1", "%2", " 2", "%1", " 2", "%1", "@1", " 1", "%18", " 10", "%37", " 4", "%3", " 1", "n", " 4", "%18", " 1", "%22", " 11", "@1", "%31", " 4", "%7", " 1", "n", " 5", "%39", " 14", "%28", " 8", "%3", " 3", "n", " 6", "@1", "%35", " 18", "%25", " 15", "n", " 8", "%32", " 22", "%19", " 2", "%7", " 10", "n", " 11", "%26", " 27", "%15", " 2", "@1", "%9", " 9", "n", " 14", "%20", " 11", "@1", "%1", "@1", "%1", " 18", "@1", "%18", " 3", "%3", " 8", "n", " 18", "%15", " 8", "%10", " 20", "%15", " 4", "%1", " 9", "n", " 16", "%36", " 22", "%14", " 12", "n", " 16", "%26", " 2", "%4", " 1", "%3", " 22", "%10", " 2", "%3", "@1", " 10", "n", " 21", "%19", " 1", "%6", " 1", "%2", " 26", "%13", "@1", " 10", "n", " 81", "%7", "@1", " 8", "n"};
    for (int tokenIndex = 0; tokenIndex < 362; tokenIndex++) {
        char* token = compressedDragon[tokenIndex];

        if (strcmp(token, "n") == 0) {
            printf("\n");
            continue;
        }

        char* pattern = malloc(2*sizeof(char));
        char* repeat = malloc(4*sizeof(char));

        int patternIter = 0;
        int repeatIter = 0;
        int patternLength = 0;
        int repeatLength = 0;
        while (*(token)) {
            char c = *(token);
            if (!isdigit(c)) {
                pattern[patternIter] = c;
                patternLength = patternIter;
                patternIter++;
            } else {
                repeat[repeatIter] = c;
                repeatLength = repeatIter;
                repeatIter++;
            }
            token++;
        }
        pattern[patternLength+1] = '\0';
        repeat[repeatLength+1] = '\0';

        token -= (patternIter + repeatIter);

        int repeatAsInt = atoi(repeat);

        for (int i = 0; i < repeatAsInt; i++) {
            printf("%s", pattern);
        }

        free(pattern);
        free(repeat);
    }
}


