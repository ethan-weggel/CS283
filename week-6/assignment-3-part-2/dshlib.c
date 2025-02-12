#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
int exec_local_cmd_loop() {

    char *cmd_buff = malloc(SH_CMD_MAX);
    int rc = 0;
    cmd_buff_t cmd;

    while (1) {

        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        // remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        if (strlen(cmd_buff) == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        rc = alloc_cmd_buff(&cmd);
        if (rc != OK) {
            return rc;
        }

        rc = build_cmd_buff(cmd_buff, &cmd);
        if (rc < 0) {
            return rc;
        }

        rc = exec_cmd(&cmd);
        if (rc < 0) {
            if (rc == OK_EXIT) {
                free_cmd_buff(&cmd);
                return OK;
            } else {
                return rc;
            }
        }

        clear_cmd_buff(&cmd);
    }

    // TODO IMPLEMENT parsing input to cmd_buff_t *cmd_buff

    // TODO IMPLEMENT if built-in command, execute builtin logic for exit, cd (extra credit: dragon)
    // the cd command should chdir to the provided directory; if no directory is provided, do nothing

    // TODO IMPLEMENT if not built-in command, fork/exec as an external command
    // for example, if the user input is "ls -l", you would fork/exec the command "ls" with the arg "-l"

    return OK;
}

Built_In_Cmds match_command(const char* input) {
    if (strcmp(input, "exit") == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "dragon") == 0) {
        return BI_CMD_DRAGON;
    } else if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    } else {
        return BI_NOT_BI;
    }
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t* cmd) {
    Built_In_Cmds type = match_command(cmd->argv[0]);

    if (type == BI_CMD_EXIT) {
        return BI_RC;
    } else if (type == BI_CMD_DRAGON) {
        print_dragon();
        return BI_EXECUTED;
    } else if (type == BI_CMD_CD) {

        if (cmd->argc == 1) {
            return BI_EXECUTED;
        }

        int rc = chdir(cmd->argv[1]);

        if (rc == 0) {
            return BI_EXECUTED;
        } else {
            return BI_RC;
        }

    }
    return BI_RC;
}

int exec_cmd(cmd_buff_t* cmd) {
    Built_In_Cmds type = match_command(cmd->argv[0]);

    if (type == BI_NOT_BI) {
        int pRes, cRes;

        pRes = fork();
        if (pRes < 0) {
            perror("Error starting fork child process");
            return ERR_EXEC_CMD;
        }

        if (pRes == 0) {

            int rc = execvp(cmd->argv[0], cmd->argv);
            if (rc < 0) {
                perror("Error executing external command");
                return ERR_EXEC_CMD;
            }
        } else {
            wait(&cRes);
            return WEXITSTATUS(cRes);
        }

    } else {
        Built_In_Cmds rc = exec_built_in_cmd(cmd);
        if (rc == BI_RC) {
            return OK_EXIT;
        } else {
            return OK;
        }
    }
    return OK;
}

int alloc_cmd_buff(cmd_buff_t* cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer == NULL) {
        return ERR_MEMORY;
    }

    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }

    return OK;
}


int free_cmd_buff(cmd_buff_t* cmd_buff) {
    for (int i = 0; i < cmd_buff->argc; i++) {
        if (cmd_buff->argv[i] != NULL) {
            free(cmd_buff->argv[i]);
        }
    }

    if (cmd_buff->_cmd_buffer != NULL) {
        free(cmd_buff->_cmd_buffer);
    }

    return OK;
}


int clear_cmd_buff(cmd_buff_t* cmd_buff) {
    memset(cmd_buff, '\0', sizeof(*cmd_buff));
    return OK;
}

int build_cmd_buff(char* cmd_line, cmd_buff_t* cmd_buff) {
    int argc = 0;
    int argsLength = 0;
    char* token = cmd_line;
    char* arg = malloc(ARG_MAX);

    strcpy(cmd_buff->_cmd_buffer, cmd_line);

    stripLTWhiteSpace(cmd_line);
    
    // start tokenizing manually (since we need to handle quotes too)
    while (*token != '\0') {
        while (*token == ' ' || *token == '\t') {
            token++;
        }

        // when we hit a quote, start copying everything until ending quote
        if (*token == '"') {
            token++;
            int tempIndex = 0;

            while (*token != '"' && *token != '\0') {
                arg[tempIndex++] = *token;
                token++;
                argsLength++;
            }
            arg[tempIndex] = '\0';

            // now we add the quote arg to the argv list
            cmd_buff->argv[argc] = malloc(strlen(arg) + 1);
            strcpy(cmd_buff->argv[argc], arg);
            argc++;

            if (*token == '"') {
                token++;
            }
        }
        else {
            // otherwise we handle the token regularly
            int temp_index = 0;
            while (*token != '\0' && !(*token == ' ' || *token == '\t')) {
                arg[temp_index++] = *token;
                token++;
                argsLength++;
            }
            arg[temp_index] = '\0'; 

            if (temp_index > 0) {
                cmd_buff->argv[argc] = malloc(strlen(arg) + 1);
                strcpy(cmd_buff->argv[argc], arg);
                argc++;
            }
        }

        while (*token == ' ' || *token == '\t') {
            token++;
        }
    }

    if (argsLength > ARG_MAX) {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }

    cmd_buff->argc = argc;
    free(arg);
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