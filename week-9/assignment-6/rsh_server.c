
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>

//INCLUDES for extra credit
#include <signal.h>
#include <pthread.h>
//-------------------------

#include "dshlib.h"
#include "rshlib.h"

static int IS_THREADED = 0;
volatile sig_atomic_t SERVER_STOP = 0;
static int LISTEN_SOCKET;

/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 * 
 *      This function basically runs the server by: 
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server. 
 * 
 *      This function is fully implemented for you and should not require
 *      any changes for basic functionality.  
 * 
 *      IF YOU IMPLEMENT THE MULTI-THREADED SERVER FOR EXTRA CREDIT YOU NEED
 *      TO DO SOMETHING WITH THE is_threaded ARGUMENT HOWEVER.  
 */
int start_server(char *ifaces, int port, int is_threaded) {
    IS_THREADED = is_threaded;
    int svr_socket;
    int rc;

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0){
        int err_code = svr_socket;  //server socket will carry error code
        return err_code;
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);

    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function. 
 * 
 *      This function simply returns the value of close() when closing
 *      the socket.  
 */
int stop_server(int svr_socket) {
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.   
 * 
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.  Specifically: 
 * 
 *      1. Create the server socket using the socket() function. 
 *      2. Calling bind to "bind" the server to the interface and port
 *      3. Calling listen to get the server ready to listen for connections.
 * 
 *      after creating the socket and prior to calling bind you might want to 
 *      include the following code:
 * 
 *      int enable=1;
 *      setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
 * 
 *      when doing development you often run into issues where you hold onto
 *      the port and then need to wait for linux to detect this issue and free
 *      the port up.  The code above tells linux to force allowing this process
 *      to use the specified port making your life a lot easier.
 * 
 *  Returns:
 * 
 *      server_socket:  Sockets are just file descriptors, if this function is
 *                      successful, it returns the server socket descriptor, 
 *                      which is just an integer.
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code is returned if the socket(),
 *                               bind(), or listen() call fails. 
 * 
 */
int boot_server(char *ifaces, int port) {
    
    // set up new socket and check for error
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0) {
        return ERR_RDSH_COMMUNICATION;
    }

    // force-binding port via linux syscall
    int enable = 1;
    setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    // bind socket to server address w/ port number by populating addr. struct
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    // converting string ip to usable form
    if (inet_pton(AF_INET, ifaces, &address.sin_addr) <= 0) {
        return ERR_RDSH_COMMUNICATION;
    }

    int bindRC = bind(socketFd, (const struct sockaddr*) &address, sizeof(struct sockaddr_in));
    if (bindRC < 0) {
        return ERR_RDSH_COMMUNICATION;
    }

    // now we set up listen syscall so that socket will listen for connection
    // backlog size -> queue of 20 clients
    int listenRC = listen(socketFd, 20);
    if (listenRC < 0) {
        return ERR_RDSH_COMMUNICATION;
    }

    return socketFd;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *   
 *  This function handles managing client connections.  It does this using
 *  the following logic
 * 
 *      1.  Starts a while(1) loop:
 *  
 *          a. Calls accept() to wait for a client connection. Recall that 
 *             the accept() function returns another socket specifically
 *             bound to a client connection. 
 *          b. Calls exec_client_requests() to handle executing commands
 *             sent by the client. It will use the socket returned from
 *             accept().
 *          c. Loops back to the top (step 2) to accept connecting another
 *             client.  
 * 
 *          note that the exec_client_requests() return code should be
 *          negative if the client requested the server to stop by sending
 *          the `stop-server` command.  If this is the case step 2b breaks
 *          out of the while(1) loop. 
 * 
 *      2.  After we exit the loop, we need to cleanup.  Dont forget to 
 *          free the buffer you allocated in step #1.  Then call stop_server()
 *          to close the server socket. 
 * 
 *  Returns:
 * 
 *      OK_EXIT:  When the client sends the `stop-server` command this function
 *                should return OK_EXIT. 
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code terminates the loop and is
 *                returned from this function in the case of the accept() 
 *                function failing. 
 * 
 *      OTHERS:   See exec_client_requests() for return codes.  Note that positive
 *                values will keep the loop running to accept additional client
 *                connections, and negative values terminate the server. 
 * 
 */
int process_cli_requests(int svr_socket) {
    int connectedSocketFd;
    int rc;

    // setting global server socket for threads to look at
    LISTEN_SOCKET = svr_socket;

    // setting flags in order to create a non-blocking accept process
    int flags = fcntl(LISTEN_SOCKET, F_GETFL, 0);
    fcntl(LISTEN_SOCKET, F_SETFL, flags | O_NONBLOCK);

    while (1) {
        // try to connect and handle errors
        connectedSocketFd = accept(svr_socket, NULL, NULL);
        if (connectedSocketFd >= 0) {
            printf("Server connected to new client.\n");
        }

        if (connectedSocketFd < 0) {
            // if we don't have a client, do a brief pause, could do other work here
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(50000);
                continue;
            } else {
                // we stop LISTEN_SOCKET to exit server which intentionally throws an error
                // its handled here
                if (SERVER_STOP == 1) {
                    return OK;
                }

                // else, return general error because something unintentional went wrong
                return ERR_RDSH_COMMUNICATION;
            }
        }

        if (IS_THREADED) {
            pthread_t threadId;
            pthread_create(&threadId, NULL, handle_threaded_client, (void*)(long) connectedSocketFd);

        } else {
            // handle request, check for exit
            rc = exec_client_requests(connectedSocketFd);
            if (rc < 0) {
                break;
            }
        }
    }

    stop_server(svr_socket);
    return OK;
}

// runs the commands from a connected client
// assume arg is a fd for a socket
// can set SERVER_STOP to terminate program
// also will close LISTEN_SOCKET (svr_socket)
// in order to get
void* handle_threaded_client(void* arg) {
    int rc;
    int socket = (int)(long) arg;

    rc = exec_client_requests(socket);

    if (rc == OK_EXIT) {
        close(LISTEN_SOCKET);
        SERVER_STOP = 1;
        // send_message_string(socket, "exit");
        close(socket);
        return NULL;
    }

    close(socket);
    return NULL;
}

/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *   
 *  This function handles accepting remote client commands. The function will
 *  loop and continue to accept and execute client commands.  There are 2 ways
 *  that this ongoing loop accepting client commands ends:
 * 
 *      1.  When the client executes the `exit` command, this function returns
 *          to process_cli_requests() so that we can accept another client
 *          connection. 
 *      2.  When the client executes the `stop-server` command this function
 *          returns to process_cli_requests() with a return code of OK_EXIT
 *          indicating that the server should stop. 
 * 
 *  Note that this function largely follows the implementation of the
 *  exec_local_cmd_loop() function that you implemented in the last 
 *  shell program deliverable. The main difference is that the command will
 *  arrive over the recv() socket call rather than reading a string from the
 *  keyboard. 
 * 
 *  This function also must send the EOF character after a command is
 *  successfully executed to let the client know that the output from the
 *  command it sent is finished.  Use the send_message_eof() to accomplish 
 *  this. 
 * 
 *  Of final note, this function must allocate a buffer for storage to 
 *  store the data received by the client. For example:
 *     io_buff = malloc(RDSH_COMM_BUFF_SZ);
 *  And since it is allocating storage, it must also properly clean it up
 *  prior to exiting.
 * 
 *  Returns:
 * 
 *      OK:       The client sent the `exit` command.  Get ready to connect
 *                another client. 
 *      OK_EXIT:  The client sent `stop-server` command to terminate the server
 * 
 *      ERR_RDSH_COMMUNICATION:  A catch all for any socket() related send
 *                or receive errors. 
 */
int exec_client_requests(int cli_socket) {
    char* buff;
    int recv_size;
    int is_last_chunk;
    char eof_char = '\0';
    int rc;

    buff = malloc(RDSH_COMM_BUFF_SZ);

    // while we haven't chosen to exit, keep looping
    while (1) {

        if (SERVER_STOP) {
            // send_message_string(cli_socket, "exit");
            return OK_EXIT;
        }

        command_list_t* clist = (command_list_t*) malloc(sizeof(command_list_t));
        if (!clist) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }
        memset(clist, 0, sizeof(command_list_t));
        clist->num = 0;

        // while we haven't reached the end of our stream, keep looping and collecting chunks
        while ((recv_size = recv(cli_socket, buff, RDSH_COMM_BUFF_SZ, 0)) > 0) {

            // if we get an error, return error code for communication fault
            if (recv_size < 0) {
                return ERR_RDSH_COMMUNICATION;
            }

            // if we received zero bytes [even though this can happen while waiting], exit
            if (recv_size == 0) {
                return OK;
            }

            is_last_chunk = ((char) buff[recv_size-1] == eof_char) ? 1 : 0;

            if (is_last_chunk) {
                buff[recv_size-1] = '\0';
                break;
            }
        }

        if (strlen(buff) == 0) {
            // Drain any remaining null bytes, or simply break out.
            // For example, break out of the loop to close the connection:
            break;
        }

        // now we have a null-terminated string inside buff;
        printf(RCMD_MSG_SVR_EXEC_REQ, buff);

        if (strcmp(buff, "exit") == 0) {

            rc = send_message_string(cli_socket, buff);
            if (rc < 0) {
                return ERR_RDSH_COMMUNICATION;
            }

            printf(RCMD_MSG_CLIENT_EXITED);

            free(buff);
            return OK;  
        }

        if (strcmp(buff, "stop-server") == 0) {

            rc = send_message_string(cli_socket, "exit");
            if (rc < 0) {
                return ERR_RDSH_COMMUNICATION;
            }

            printf(RCMD_MSG_SVR_STOP_REQ);

            free(buff);
            return OK_EXIT;
        }

        rc = build_cmd_list(buff, clist);
        if (rc != OK) {
            if (rc == ERR_CMD_OR_ARGS_TOO_BIG) {
                printf(CMD_ERR_PIPE_LIMIT, 8);
            }
            free(clist);
            continue;
        }

        // we save original descriptors so we can change it back after children are
        // done writing to socket
        int saved_stdin = dup(STDIN_FILENO);
        int saved_stdout = dup(STDOUT_FILENO);
        int saved_stderr = dup(STDERR_FILENO);

        rc = rsh_execute_pipeline(cli_socket, clist);
        if (rc < 0) {
            break;
        }

        // set the old descriptors back
        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);
        dup2(saved_stderr, STDERR_FILENO);

        // close the descriptors so we don't get leaks
        close(saved_stdin);
        close(saved_stdout);
        close(saved_stderr);

        // printf(RCMD_MSG_SVR_RC_CMD, rc);
        free(clist);

        continue;
    }

    return OK;
}

/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client

 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent. 
 * 
 *  Returns:
 * 
 *      OK:  The EOF character was sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the EOF character. 
 */
int send_message_eof(int cli_socket) {
    ssize_t bytesSent;

    // send EOF character [one byte], return OK if we are successful
    bytesSent = send(cli_socket, &RDSH_EOF_CHAR, 1, MSG_NOSIGNAL);
    if (bytesSent == 1) {

        return OK;
    }

    // if we got here, something bad happened 
    return ERR_RDSH_COMMUNICATION;
}

/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 *   
 *  Sends a message to the client.  Note this command executes both a send()
 *  to send the message and a send_message_eof() to send the EOF character to
 *  the client to indicate command execution terminated. 
 * 
 *  Returns:
 * 
 *      OK:  The message in buff followed by the EOF character was 
 *           sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the message followed by the EOF character. 
 */
int send_message_string(int cli_socket, char *buff) {
    size_t bytesSent;

    // send null-terminated string, if we get that number of bytes sent, we are successful
    bytesSent = send(cli_socket, buff, strlen(buff), 0);
    if (bytesSent == strlen(buff)) {
        int rc = send_message_eof(cli_socket);
        if (rc < 0) {
            return ERR_RDSH_COMMUNICATION;
        }
        return OK;
    }

    return ERR_RDSH_COMMUNICATION;
}


/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell. 
 *   
 *  This function executes the command pipeline.  It should basically be a
 *  replica of the execute_pipeline() function from the last deliverable. 
 *  The only thing different is that you will be using the cli_sock as the
 *  main file descriptor on the first executable in the pipeline for STDIN,
 *  and the cli_sock for the file descriptor for STDOUT, and STDERR for the
 *  last executable in the pipeline.  See picture below:  
 * 
 *      
 *┌───────────┐                                                    ┌───────────┐
 *│ cli_sock  │                                                    │ cli_sock  │
 *└─────┬─────┘                                                    └────▲──▲───┘
 *      │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐  │  │    
 *      │   │   Process 1  │     │   Process 2  │     │   Process N  │  │  │    
 *      │   │              │     │              │     │              │  │  │    
 *      └───▶stdin   stdout├─┬──▶│stdin   stdout├─┬──▶│stdin   stdout├──┘  │    
 *          │              │ │   │              │ │   │              │     │    
 *          │        stderr├─┘   │        stderr├─┘   │        stderr├─────┘    
 *          └──────────────┘     └──────────────┘     └──────────────┘   
 *                                                      WEXITSTATUS()
 *                                                      of this last
 *                                                      process to get
 *                                                      the return code
 *                                                      for this function       
 * 
 *  Returns:
 * 
 *      EXIT_CODE:  This function returns the exit code of the last command
 *                  executed in the pipeline.  If only one command is executed
 *                  that value is returned.  Remember, use the WEXITSTATUS()
 *                  macro that we discussed during our fork/exec lecture to
 *                  get this value. 
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {

    // changing the file descriptors for parent process so ALL
    // children inherit this behavior; we set it back to default
    // when function exits
    dup2(cli_sock, STDIN_FILENO);
    dup2(cli_sock, STDOUT_FILENO);
    dup2(cli_sock, STDERR_FILENO);

    // ADDED: handle built-ins in parent if there's only one command
    if (clist->num == 1) {
        cmd_buff_t *cmd = &clist->commands[0];
        Built_In_Cmds type = match_command(cmd->argv[0]);
        if (type != BI_NOT_BI) {
            Built_In_Cmds rc = rsh_built_in_cmd(cmd, cli_sock);
            send_message_eof(cli_sock);


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
            // if (i == 0) {
            //     dup2(cli_sock, STDIN_FILENO);
            // }

            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // if (i == clist->num - 1) {
            //     dup2(cli_sock, STDOUT_FILENO);
            //     dup2(cli_sock, STDERR_FILENO);
            // }

            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            close(cli_sock);

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
                    send_message_string(cli_sock, "Command not found in PATH\n");
                    errno = WEXITSTATUS(childStatus);
                    // send_message_eof(cli_sock);
                    return errno;
                case EACCES:
                    send_message_string(cli_sock, "Permission denied to execute command\n");
                    errno = WEXITSTATUS(childStatus);
                    // send_message_eof(cli_sock);
                    return errno;
                default:
                    send_message_string(cli_sock, "Error executing external command\n");
                    errno = WEXITSTATUS(childStatus);
                    // send_message_eof(cli_sock);
                    return errno;
            }
        }
    }

    send_message_eof(cli_sock);

    return pipelineStatus;
}

Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd, int cli_socket) {

    Built_In_Cmds type = match_command(cmd->argv[0]);

    // if we are using exit, exit, otherwise call existing dragon function or cd using syscall
    if (type == BI_CMD_EXIT) {
        send_message_eof(cli_socket);
        return BI_CMD_EXIT;
    } else if (type == BI_CMD_DRAGON) {
        print_dragon();
        send_message_eof(cli_socket);
        return BI_EXECUTED;
    } else if (type == BI_CMD_CD) {
        if (cmd->argc == 1) {
            return BI_EXECUTED;
        }
        chdir(cmd->argv[1]);
        send_message_eof(cli_socket);
        return BI_EXECUTED;
    } else if (type == BI_CMD_RC) {
        int savedErrno = errno;
        printf("%d\n", savedErrno);
        send_message_eof(cli_socket);
        return BI_EXECUTED;
    }
    send_message_eof(cli_socket);
    return BI_EXECUTED;
}


