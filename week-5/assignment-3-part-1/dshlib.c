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

void printDragon() {
    int i = 0;
    int rc = 0;
    char allLinesExpanded[38][101];  // Fixed size for each line
    char* dragon = strdup(COMPRESSED_DRAGON);
    char* lineExapanded = malloc(DRAGON_LINE_SIZE);
    char* lineToken = malloc(DRAGON_LINE_SIZE);

    // while we still have lines to extract
    while (strlen(lineToken) != 0 || i == 0) {
        // get the compressed new line
        rc = getTruncToken(dragon, lineToken, DOLLAR_STRING);
        
        char* compressedToken = malloc(100);  // Sufficient space for the token
        int j = 0;

        // Declare lineIndex outside the inner loop so it is accessible later
        int lineIndex = 0;  // Make this available for the entire while loop

        // while we still have tokens to get from the line
        while (strlen(compressedToken) != 0 || j == 0) {
            // get the compressed character token
            rc = getTruncToken(lineToken, compressedToken, DASH_STRING);

            char* characterToAdd = malloc(3);
            int numberOfRepeats = 0;
            int k = 0;
            while (*(compressedToken + k)) {
                char c = *(compressedToken + k);
                if (!isdigit(c)) {
                    *(characterToAdd + k) = c;
                } else {
                    numberOfRepeats = atoi((compressedToken + k));
                }
                k++;
            }
            *(characterToAdd + k) = '\0';

            // now we have our character string and how many times it repeats, so we add it to the line
            for (int h = 0; h < numberOfRepeats; h++) {
                for (size_t g = 0; g < strlen(characterToAdd); g++) {
                    lineExapanded[lineIndex] = characterToAdd[g];
                    lineIndex++;
                }
            }
            lineExapanded[lineIndex] = '\0';  // Null-terminate the line

            free(characterToAdd);
            j++;
        }

        // Copy the null-terminated line with lineIndex being properly scoped now
        memcpy(allLinesExpanded[i], lineExapanded, lineIndex + 1);  // Copy the line

        i++;
    }

    // Print the result
    for (int i = 0; i < 38; i++) {
        printf("%s\n", allLinesExpanded[i]);
    }

    // Free allocated memory
    free(dragon);
    free(lineExapanded);
    free(lineToken);
}


