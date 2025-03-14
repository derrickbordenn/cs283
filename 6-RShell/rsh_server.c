#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>

#include "dshlib.h"
#include "rshlib.h"

int start_server(char *ifaces, int port, int is_threaded) {
    (void)is_threaded; // I don't use this parameter 
    int svr_socket;
    int rc;

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0) {
        int err_code = svr_socket;
        return err_code;
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);

    return rc;
}

int stop_server(int svr_socket) {
    return close(svr_socket);
}

int boot_server(char *ifaces, int port) {
    int svr_socket;
    int ret;

    struct sockaddr_in addr;

    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    int enable = 1;
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ifaces);

    if (bind(svr_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    ret = listen(svr_socket, 20);
    if (ret == -1) {
        perror("listen");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

int process_cli_requests(int svr_socket) {
    int cli_socket;
    int rc = OK;
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    while (1) {
        cli_socket = accept(svr_socket, (struct sockaddr *) &cli_addr, &clilen);
        if (cli_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }

        rc = exec_client_requests(cli_socket);

        close(cli_socket);

        if (rc == OK_EXIT) {
            break;
        }
    }

    return rc;
}

int exec_client_requests(int cli_socket) {
    int io_size;
    command_list_t cmd_list;
    int rc;
    int cmd_rc;
    int last_rc;
    char *io_buff;

    io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (io_buff == NULL) {
        return ERR_RDSH_SERVER;
    }

    while (1) {
        io_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ - 1, 0);
        if (io_size <= 0) {
            free(io_buff);
            return (io_size == 0) ? OK : ERR_RDSH_COMMUNICATION;
        }

        io_buff[io_size] = '\0';

        for (int i = 0; i < CMD_MAX; i++) {
            alloc_cmd_buff(&cmd_list.commands[i]);
        }
        cmd_list.num = 0;

        rc = build_cmd_list(io_buff, &cmd_list);

        if (rc == WARN_NO_CMDS) {
            continue;
        } else if (rc == ERR_TOO_MANY_COMMANDS) {
            send_message_string(cli_socket, CMD_ERR_PIPE_LIMIT);
            continue;
        }

        if (cmd_list.num == 1) {
            Built_In_Cmds bi_cmd = rsh_built_in_cmd(&cmd_list.commands[0]);
            if (bi_cmd == BI_CMD_EXIT) {
                free(io_buff);
                return OK;
            } else if (bi_cmd == BI_CMD_STOP_SVR) {
                free(io_buff);
                return OK_EXIT;
            }
        }

        cmd_rc = rsh_execute_pipeline(cli_socket, &cmd_list);
        last_rc = cmd_rc;

        char rc_buff[20];
        snprintf(rc_buff, 20, "%d\n", last_rc);
        send_message_string(cli_socket, rc_buff);

        send_message_eof(cli_socket);

        for (int i = 0; i < cmd_list.num; i++) {
            clear_cmd_buff(&cmd_list.commands[i]);
        }
        cmd_list.num = 0;
    }

    free(io_buff);
    return WARN_RDSH_NOT_IMPL;
}

int send_message_eof(int cli_socket) {
    int send_len = (int) sizeof(RDSH_EOF_CHAR);
    int sent_len;
    sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

    if (sent_len != send_len) {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

int send_message_string(int cli_socket, char *buff) {
    int send_len = strlen(buff);
    int sent_len = send(cli_socket, buff, send_len, 0);

    if (sent_len != send_len) {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int pipes[clist->num - 1][2];
    pid_t pids[clist->num];
    int pids_st[clist->num];
    int exit_code;

    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pids[i] == 0) {
            if (i == 0) {
                dup2(cli_sock, STDIN_FILENO);
            } else {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            if (i == clist->num - 1) {
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            } else {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &pids_st[i], 0);
    }
	exit_code = WEXITSTATUS(pids_st[clist->num - 1]);
    return exit_code;
}

Built_In_Cmds rsh_match_command(const char *input) {
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "stop-server") == 0)
        return BI_CMD_STOP_SVR;
    if (strcmp(input, "rc") == 0)
        return BI_CMD_RC;
    return BI_NOT_BI;
}

Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds ctype = BI_NOT_BI;
    ctype = rsh_match_command(cmd->argv[0]);

    switch (ctype) {
        case BI_CMD_EXIT:
            return BI_CMD_EXIT;
        case BI_CMD_STOP_SVR:
            return BI_CMD_STOP_SVR;
        case BI_CMD_RC:
            return BI_CMD_RC;
        case BI_CMD_CD:
            chdir(cmd->argv[1]);
            return BI_EXECUTED;
        default:
            return BI_NOT_BI;
    }
}
