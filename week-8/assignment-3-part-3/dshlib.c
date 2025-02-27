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
    char *cmd_buff = malloc(SH_CMD_MAX);
    if (!cmd_buff) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    int rc = 0;

    while (1) {
        command_list_t* clist = (command_list_t*) malloc(sizeof(command_list_t));
        if (!clist) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }
        memset(clist, 0, sizeof(command_list_t));
        clist->num = 0;

        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        if (strlen(cmd_buff) == 0) {
            printf(CMD_WARN_NO_CMD);
            free(clist);
            continue;
        }

        rc = build_cmd_list(cmd_buff, clist);
        if (rc != OK) {
            if (rc == ERR_CMD_OR_ARGS_TOO_BIG) {
                printf(CMD_ERR_PIPE_LIMIT, 8);
            }
            free(clist);
            continue;
        }

        rc = execute_pipeline(clist);
        if (rc == OK_EXIT) {
            free(clist);
            break;
        }

        free(clist);
    }

    free(cmd_buff);
    return OK;
}


/*
 * function: build_cmd_list
 * purpose: parses a command line into separate commands split by pipe symbols
 * parameters:
 *    cmd_line: the raw command line input
 *    clist: pointer to a command_list_t to populate
 * returns: ok on success, error code on failure
 */
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    int commandCount = 0;
    char tokenBuffer[SH_CMD_MAX]; // local token buffer
    cmd_buff_t command_t;
    alloc_cmd_buff(&command_t);

    stripLTWhiteSpace(cmd_line);

    // Get and process the first token.
    int tokenRC = getTruncToken(cmd_line, tokenBuffer, PIPE_STRING);

    if (tokenRC == -1) {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }

    int buildRC = build_cmd_buff(tokenBuffer, &command_t);

    if (buildRC < 0) {
        if (buildRC == ERR_CMD_ARGS_BAD) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
        }
        return buildRC;
    }

    clist->commands[clist->num] = command_t;
    clist->num++;
    commandCount++;

    // Loop while there is still remaining input in cmd_line.
    while (strlen(cmd_line) != 0) {

        stripLTWhiteSpace(cmd_line);
        if (strlen(cmd_line) == 0) {
            break;
        }

        char* tokenBuffer = malloc(SH_CMD_MAX);
        memset(tokenBuffer, 0, SH_CMD_MAX);

        tokenRC = getTruncToken(cmd_line, tokenBuffer, PIPE_STRING);

        if (strlen(tokenBuffer) == 0) {
            free(tokenBuffer);
            break;
        }

        commandCount++;

        if (commandCount > CMD_MAX) {
            free(tokenBuffer);
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        alloc_cmd_buff(&command_t);
        buildRC = build_cmd_buff(tokenBuffer, &command_t);

        if (buildRC < 0) {
            if (buildRC == ERR_CMD_ARGS_BAD) {
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            }
            free(tokenBuffer);
            return buildRC;
        }


        clist->commands[clist->num] = command_t;
        clear_cmd_buff(&command_t);

        free(tokenBuffer);
        clist->num++;
        if (buildRC == ERR_CMD_OR_ARGS_TOO_BIG) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

    }


    return OK;
}

/*
 * function: match_command
 * purpose: determines if the input is a built-in command
 * parameters:
 *    input: string for command name
 * returns: a Built_In_Cmds enum indicating the built-in type or bi_not_bi
 */
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


/*
 * function: exec_built_in_cmd
 * purpose: runs the specified built-in command (cd, exit, etc.)
 * parameters:
 *    cmd: a pointer to the cmd_buff_t containing arguments
 * returns: a Built_In_Cmds enum result indicating execution or exit
 */
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


/*
 * function: exec_cmd
 * purpose: handles built-in vs external commands; external commands are executed via execvp
 * parameters:
 *    cmd: a pointer to the cmd_buff_t with argv data
 * returns: ok_exit for exit, ok on success, otherwise error code
 */
int exec_cmd(cmd_buff_t* cmd) {
    Built_In_Cmds type = match_command(cmd->argv[0]);

    if (type == BI_NOT_BI) {
        // REMOVED second fork
        handle_redirections(cmd);
        // ADDED: directly exec here
        execvp(cmd->argv[0], cmd->argv);
        int exec_errno = errno;
        exit(exec_errno);
    } else {
        Built_In_Cmds rc = exec_built_in_cmd(cmd);
        if (rc == BI_CMD_EXIT) {
            return OK_EXIT;
        } else {
            return OK;
        }
    }
    return OK;
}


/*
 * function: alloc_cmd_buff
 * purpose: allocates memory for the cmd_buff_t's _cmd_buffer and argv
 * parameters:
 *    cmd_buff: pointer to cmd_buff_t
 * returns: ok on success or err_memory on failure
 */
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


/*
 * function: free_cmd_buff
 * purpose: frees the _cmd_buffer inside the cmd_buff_t
 * parameters:
 *    cmd_buff: pointer to cmd_buff_t
 * returns: ok
 */
int free_cmd_buff(cmd_buff_t* cmd_buff) {

    // free command buffer
    if (cmd_buff->_cmd_buffer != NULL) {
        free(cmd_buff->_cmd_buffer);
    }

    return OK;
}


/*
 * function: clear_cmd_buff
 * purpose: zeroes out the cmd_buff_t structure
 * parameters:
 *    cmd_buff: pointer to cmd_buff_t
 * returns: ok
 */
int clear_cmd_buff(cmd_buff_t* cmd_buff) {
    // write the entire struct back to null terminations
    memset(cmd_buff, '\0', sizeof(*cmd_buff));
    return OK;
}


/*
 * function: build_cmd_buff
 * purpose: tokenizes a line into argv entries and stores them in cmd_buff
 * parameters:
 *    cmd_line: the raw command line
 *    cmd_buff: pointer to a cmd_buff_t to fill
 * returns: ok on success, or an error code if arguments exceed capacity
 */
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


/*
 * function: stripLTWhiteSpace
 * purpose: removes leading/trailing whitespace from the given string in-place
 * parameters:
 *    string: a c-string to modify
 * returns: none
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


/*
 * function: getTruncToken
 * purpose: extracts a token from inputString up to the delimiter, then modifies inputString
 * parameters:
 *    inputString: the original line, advanced past the delimiter on return
 *    tokenBuffer: the token read from inputString
 *    delimiter: the delimiter (usually a pipe character)
 * returns: 0 on success, -1 if there's an error
 */
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


/*
 * function: execute_pipeline
 * purpose: if there's only one command and it's built-in, run in parent; else fork for each stage
 * parameters:
 *    clist: a pointer to command_list_t holding the pipeline commands
 * returns: ok_exit for an exit request, or the final child's error code, or 0 on success
 */
int execute_pipeline(command_list_t *clist) {
    // ADDED: handle built-ins in parent if there's only one command
    if (clist->num == 1) {
        cmd_buff_t *cmd = &clist->commands[0];
        Built_In_Cmds type = match_command(cmd->argv[0]);
        if (type != BI_NOT_BI) {
            Built_In_Cmds rc = exec_built_in_cmd(cmd);
            if (rc == BI_CMD_EXIT) {
                return OK_EXIT;
            }
            return OK;
        }
    }

    int pipes[clist->num - 1][2];
    pid_t pids[clist->num];

    for (int i = 0; i < clist->num -1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Error making pipe.");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("Error making child processes.");
            exit(EXIT_FAILURE);
        }
        if (pids[i] == 0) {
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            int rc = exec_cmd(&clist->commands[i]);
            if (rc == OK_EXIT) {
                exit(EXIT_SC);
            } else if (rc == OK) {
                exit(EXIT_SUCCESS);
            } else {
                exit(EXIT_FAILURE);
            }
        }
    }

    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    int pipelineStatus = EXIT_SUCCESS;
    int childStatus;
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &childStatus, 0);
        if (WIFEXITED(childStatus) && WEXITSTATUS(childStatus) == EXIT_SC) {
            pipelineStatus = OK_EXIT;
        }

        if (WIFEXITED(childStatus) && WEXITSTATUS(childStatus)) {
            switch (WEXITSTATUS(childStatus)) {
                case ENOENT:
                    printf("Command not found in PATH\n");
                    errno = WEXITSTATUS(childStatus);
                    return errno;
                case EACCES:
                    printf("Permission denied to execute command\n");
                    errno = WEXITSTATUS(childStatus);
                    return errno;
                default:
                    printf("Error executing external command\n");
                    errno = WEXITSTATUS(childStatus);
                    return errno;
            }
        }
    }

    return pipelineStatus;
}

/*
 * function: handle_redirections
 * purpose: scans cmd->argv for redirection operators and opens/closes file descriptors
 * parameters:
 *    cmd: pointer to a cmd_buff_t to adjust
 * returns: none (exits on error)
 */
void handle_redirections(cmd_buff_t *cmd) {
    int new_argc = 0;
    char *new_argv[CMD_ARGV_MAX];

    for (int i = 0; i < cmd->argc; i++) {
        if (strcmp(cmd->argv[i], REDIR_STDIN) == 0) {
            // if we want to redirect stdin, open file for reading
            if (i + 1 < cmd->argc) {
                int fd = open(cmd->argv[i+1], O_RDONLY);
                if (fd < 0) {
                    perror("open for input");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
                i++;
                continue;
            } else {
                fprintf(stderr, "syntax error: expected filename after %s\n", REDIR_STDIN);
                exit(1);
            }
        } else if (strcmp(cmd->argv[i], REDIR_STDOUT) == 0) {
            // if we want to redirect stdout, open file for writing
            if (i + 1 < cmd->argc) {
                int fd = open(cmd->argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open for output");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                i++; 
                continue;
            } else {
                fprintf(stderr, "syntax error: expected filename after %s\n", REDIR_STDOUT);
                exit(1);
            }
        } else if (strcmp(cmd->argv[i], STDOUT_APPEND) == 0) {
            // if we want to redirect stdout appending, open file for writing with no trunc
            if (i + 1 < cmd->argc) {
                int fd = open(cmd->argv[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (fd < 0) {
                    perror("open for output append");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                i++; 
                continue;
            } else {
                fprintf(stderr, "syntax error: expected filename after %s\n", STDOUT_APPEND);
                exit(1);
            }
        } else {

            new_argv[new_argc++] = cmd->argv[i];
        }
    }
    new_argv[new_argc] = NULL;
    
    // update the new argument now that we got rid of and parsed the redirection
    cmd->argc = new_argc;
    for (int i = 0; i < new_argc; i++) {
        cmd->argv[i] = new_argv[i];
    }
    cmd->argv[new_argc] = NULL;
}