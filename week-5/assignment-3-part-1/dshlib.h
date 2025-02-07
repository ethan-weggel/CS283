#ifndef __DSHLIB_H__
#define __DSHLIB_H__

// Constants for command structure sizes
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
#define DRAGON_LINE_SIZE 101
// Longest command that can be read from the shell
#define SH_CMD_MAX EXE_MAX + ARG_MAX

typedef struct command
{
    char exe[EXE_MAX];
    char args[ARG_MAX];
} command_t;

typedef struct command_list
{
    int num;
    command_t commands[CMD_MAX];
} command_list_t;

// Special character #defines
#define SPACE_CHAR ' '
#define SPACE_STRING " "
#define PIPE_CHAR '|'
#define PIPE_STRING "|"
#define COMPRESSED_DRAGON " 72-@1-%%4-$- 69-%%6-$- 68-%%6-$- 65-%%1- 1-%%7- 11-@1-$- 64-%%10- 8-%%7-$- 39-%%7- 2-%%4-@1- 9-%%12-@1- 4-%%6- 2-@1-%%4-$- 34-%%22- 6-%%28-$- 32-%%26- 3-%%12- 1-%%15-$- 31-%%29- 1-%%19- 5-%%3-$- 29-%%28-@1- 1-@1-%%18- 8-%%2-$- 28-%%33- 1-%%22-$- 28-%%58-$- 28-%%50-@1-%%6-@1-$- 6-%%8-@1- 11-%%16- 8-%%26- 6-%%2-$- 4-%%13- 9-%%2-@1-%%12- 11-%%11- 1-%%12- 6-@1-%%1-$- 2-%%10- 3-%%3- 8-%%14- 12-%%24-$- 1-%%9- 7-%%1- 9-%%13- 13-%%12-@1-%%11-$-%%9-@1- 16-%%1- 1-%%13- 12-@1-%%25-$-%%8-@1- 17-%%2-@1-%%12- 12-@1-%%28-$-%%7-@1- 19-%%15- 11-%%33-$-%%10- 18-%%15- 10-%%35- 6-%%4-$-%%9-@1- 19-@1-%%14- 9-%%12-@1- 1-%%4- 1-%%17- 3-$-%%10- 18-%%17- 8-%%13- 6-%%18- 1-$-%%9-@1-%%2-@1- 16-%%16-@1- 7-%%14- 5-%%24- 2-$- 1-%%10- 18-%%1- 1-%%14-@1- 8-%%14- 3-%%26- 1-$- 2-%%12- 2-@1- 11-%%18- 8-%%40- 2-%%3-$- 3-%%13- 1-%%2- 2-%%1- 2-%%1-@1- 1-%%18- 10-%%37- 4-%%3-$- 4-%%18- 1-%%22- 11-@1-%%31- 4-%%7-$- 5-%%39- 14-%%28- 8-%%3-$- 6-@1-%%35- 18-%%25-$- 8-%%32- 22-%%19- 2-%%7-$- 11-%%26- 27-%%15- 2-@1-%%9-$- 14-%%20- 11-@1-%%1-@1-%%1- 18-@1-%%18- 3-%%3-$- 18-%%15- 8-%%10- 20-%%15- 4-%%1-$- 16-%%36- 22-%%14-$- 16-%%26- 2-%%4- 1-%%3- 22-%%10- 2-%%3-@1-$- 21-%%19- 1-%%6- 1-%%2- 26-%%13-@1-$- 81-%%7-@1-$-"
#define DASH_STRING "-"
#define DOLLAR_STRING "$"
#define PERCENT_STRING "%%"
#define AT_STRING "@"

#define SH_PROMPT "dsh> "
#define EXIT_CMD "exit"

// Standard Return Codes
#define OK 0
#define WARN_NO_CMDS -1
#define ERR_TOO_MANY_COMMANDS -2
#define ERR_CMD_OR_ARGS_TOO_BIG -3

// starter code
#define M_NOT_IMPL "The requested operation is not implemented yet!\n"
#define EXIT_NOT_IMPL 3
#define NOT_IMPLEMENTED_YET 0

// prototypes
int build_cmd_list(char *cmd_line, command_list_t *clist);
int buildCommand(char* commandString, command_t* commandStruct);
void stripLTWhiteSpace(char* string);
int getTruncToken(char* inputString, char* tokenBuffer, char* delimiter);
void printDragon();

// output constants
#define CMD_OK_HEADER "PARSED COMMAND LINE - TOTAL COMMANDS %d\n"
#define CMD_WARN_NO_CMD "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT "error: piping limited to %d commands\n"

#endif