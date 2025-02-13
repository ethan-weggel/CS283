#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
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

    // initialize variabels
    char* cmd_buff = malloc(SH_CMD_MAX);
    int rc = 0;
    cmd_buff_t cmd;

    // run our loop
    while (1) {

        // get input
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        // remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // make sure there is something to parse, else continue
        if (strlen(cmd_buff) == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        // allocate cmd struct and check all went well
        rc = alloc_cmd_buff(&cmd);
        if (rc != OK) {
            return rc;
        }

        // populate struct based on input and check all went well
        rc = build_cmd_buff(cmd_buff, &cmd);
        if (rc < 0) {
            if (rc == ERR_CMD_ARGS_BAD) {
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
                continue;
            } 
            return rc;
        }

        // execute command based on struct and check all went well
        rc = exec_cmd(&cmd);
        if (rc == OK_EXIT) {
            free_cmd_buff(&cmd);
            return OK;
        } else if (rc < 0) {
            // show error message for non exit commands

            free_cmd_buff(&cmd);
            return rc;
        }

        // clear the buff after each command to restart
        clear_cmd_buff(&cmd);
    }

    return OK;
}

Built_In_Cmds match_command(const char* input) {
    // run through different string matches
    if (strcmp(input, "exit") == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "dragon") == 0) {
        return BI_CMD_DRAGON;
    } else if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    } else if (strcmp(input, "rc") == 0) {
        return BI_RC;
    } else {
        return BI_NOT_BI;
    }
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t* cmd) {
    Built_In_Cmds type = match_command(cmd->argv[0]);

    // if we are using exit, exit, otherwise call existing dragon function or cd using syscall
    if (type == BI_CMD_EXIT) {
        return BI_CMD_EXIT;
    } else if (type == BI_CMD_DRAGON) {
        print_dragon();
        return BI_EXECUTED;
    } else if (type == BI_CMD_CD) {
        if (cmd->argc == 1) {
            return BI_EXECUTED;
        }
        chdir(cmd->argv[1]);
        return BI_EXECUTED;
    } else if (type == BI_RC) {
        int savedErrno = errno;
        printf("%d\n", savedErrno);
        return BI_EXECUTED;
    }
    return BI_EXECUTED;
}

int exec_cmd(cmd_buff_t* cmd) {
    Built_In_Cmds type = match_command(cmd->argv[0]);

    // if we dont have a built-in, fork and execute using syscalls
    if (type == BI_NOT_BI) {
        int pRes, cRes;

        pRes = fork();
        if (pRes < 0) {
            return pRes;
        }

        if (pRes == 0) {

            int rc = execvp(cmd->argv[0], cmd->argv);
            if (rc < 0) {
                // Capture and handle execvp error
                int exec_errno = errno; 
                switch (exec_errno) {
                    case ENOENT:
                        printf("Command not found in PATH\n");
                        break;
                    case EACCES:
                        printf("Permission denied to execute command\n");
                        break;
                    default:
                        printf("Error executing external command\n");
                        break;
                }
                exit(errno);
            }
        } else {
            wait(&cRes);
            errno = WEXITSTATUS(cRes);
            return errno;
        }

    } else {
        // otherwise we use the built-in call
        Built_In_Cmds rc = exec_built_in_cmd(cmd);
        if (rc == BI_CMD_EXIT) {
            return OK_EXIT;
        } else {
            return OK;
        }
    }
    return OK;
}

int alloc_cmd_buff(cmd_buff_t* cmd_buff) {
    // allocate buffer for copy
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer == NULL) {
        return ERR_MEMORY;
    }

    // allocate memory for string pointers
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }

    return OK;
}


int free_cmd_buff(cmd_buff_t* cmd_buff) {

    // free command buffer
    if (cmd_buff->_cmd_buffer != NULL) {
        free(cmd_buff->_cmd_buffer);
    }

    return OK;
}


int clear_cmd_buff(cmd_buff_t* cmd_buff) {
    // write the entire struct back to null terminations
    memset(cmd_buff, '\0', sizeof(*cmd_buff));
    return OK;
}

int build_cmd_buff(char* cmd_line, cmd_buff_t* cmd_buff) {
    int argc = 0;
    stripLTWhiteSpace(cmd_line);

    // populate the _cmd_buffer
    strcpy(cmd_buff->_cmd_buffer, cmd_line);

    char* charPtr = cmd_buff->_cmd_buffer; 
    int length = 0;
    int quoteMode = 0;
    
    while (*charPtr != '\0') {
        if (*charPtr == '"' && !quoteMode) {
            *charPtr = '\0';
            quoteMode = 1;
        } else if (*charPtr == '"' && quoteMode){
            quoteMode = 0;
            *charPtr = '\0';
        } else if (*charPtr == ' ' && !quoteMode) {
            *charPtr = '\0';
        }
        charPtr++;
        length++;
    }

    for (int i = 0; i < length; i++) {
        if (argc > CMD_MAX) {
            return ERR_CMD_ARGS_BAD;
        }

        if (i == 0 || (cmd_buff->_cmd_buffer[i-1] == '\0' && cmd_buff->_cmd_buffer[i] != '\0')) {
            cmd_buff->argv[argc] = &cmd_buff->_cmd_buffer[i];
            argc++;
        }
    }
    
    if (length > ARG_MAX) {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }

    if (strlen(cmd_buff->argv[0]) > 64) {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }

    // set number of args and free memory
    cmd_buff->argc = argc;
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