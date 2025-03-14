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

static int last_return_code = 0;

Built_In_Cmds exec_cd_command(cmd_buff_t *cmd) {
    if (cmd->argc == 1) {
        return BI_EXECUTED;
    }
    if (chdir(cmd->argv[1]) != 0) {
        printf("cd: %s: No such file or directory\n", cmd->argv[1]);
        last_return_code = ENOENT;
        return BI_EXECUTED;
    }
    last_return_code = 0;
    return BI_EXECUTED;
}

Built_In_Cmds exec_rc_command() {
    printf("%d\n", last_return_code);
    return BI_EXECUTED;
}

Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, EXIT_CMD) == 0) return BI_CMD_EXIT;
    if (strcmp(input, "cd") == 0) return BI_CMD_CD;
    if (strcmp(input, "rc") == 0) return BI_EXECUTED;
    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds type = match_command(cmd->argv[0]);
    switch(type) {
        case BI_CMD_CD:
            return exec_cd_command(cmd);
        case BI_EXECUTED:
            return exec_rc_command();
        default:
            return type;
    }
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    bool in_quotes = false;
    int pos = 0, arg_pos = 0;
    clear_cmd_buff(cmd_buff);

    while (isspace(cmd_line[pos])) pos++;
    int arg_start = pos;

    while (cmd_line[pos] != '\0') {
        if (cmd_line[pos] == '"') {
            in_quotes = !in_quotes;
            if (in_quotes) {
                arg_start = pos + 1;
            } else {
                int len = pos - arg_start;
                if (len > 0) {
                    cmd_buff->argv[arg_pos] = malloc(len + 1);
                    strncpy(cmd_buff->argv[arg_pos], &cmd_line[arg_start], len);
                    cmd_buff->argv[arg_pos][len] = '\0';
                    arg_pos++;
                }
                arg_start = pos + 1;
            }
        } else if (isspace(cmd_line[pos]) && !in_quotes) {
            if (pos > arg_start) {
                int len = pos - arg_start;
                cmd_buff->argv[arg_pos] = malloc(len + 1);
                strncpy(cmd_buff->argv[arg_pos], &cmd_line[arg_start], len);
                cmd_buff->argv[arg_pos][len] = '\0';
                arg_pos++;
            }
            while (isspace(cmd_line[pos + 1])) pos++;
            arg_start = pos + 1;
        }
        pos++;
    }

    if (pos > arg_start) {
        int len = pos - arg_start;
        cmd_buff->argv[arg_pos] = malloc(len + 1);
        strncpy(cmd_buff->argv[arg_pos], &cmd_line[arg_start], len);
        cmd_buff->argv[arg_pos][len] = '\0';
        arg_pos++;
    }

    cmd_buff->argc = arg_pos;
    cmd_buff->argv[arg_pos] = NULL;
    return (arg_pos > 0) ? OK : WARN_NO_CMDS;
}

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    cmd_buff->_cmd_buffer = NULL;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
    }
    for (int i = 0; i < cmd_buff->argc; i++) {
        if (cmd_buff->argv[i]) {
            free(cmd_buff->argv[i]);
        }
    }
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    free_cmd_buff(cmd_buff);
    return alloc_cmd_buff(cmd_buff);
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char *pipe_token;
    char *rest = cmd_line;
    int cmd_count = 0;
    
    clist->num = 0;
    
    // Split by pipe token and build cmd_buff for each command
    while ((pipe_token = strtok_r(rest, PIPE_STRING, &rest)) != NULL && cmd_count < CMD_MAX) {
        int rc = build_cmd_buff(pipe_token, &clist->commands[cmd_count]);
        if (rc == WARN_NO_CMDS) {
            continue;  // Skip empty commands
        }
        cmd_count++;
    }
    
    if (cmd_count == 0) {
        return WARN_NO_CMDS;
    }
    
    if (cmd_count > CMD_MAX) {
        return ERR_TOO_MANY_COMMANDS;
    }
    
    clist->num = cmd_count;
    return OK;
}

int free_cmd_list(command_list_t *cmd_list) {
    for (int i = 0; i < cmd_list->num; i++) {
        free_cmd_buff(&cmd_list->commands[i]);
    }
    cmd_list->num = 0;
    return OK;
}

int exec_cmd(cmd_buff_t *cmd) {
    pid_t pid = fork();
    if (pid < 0) {
        printf("Fork failed\n");
        last_return_code = 1;
        return ERR_EXEC_CMD;
    } else if (pid == 0) {
        execvp(cmd->argv[0], cmd->argv);
        if (errno == ENOENT) {
            printf("Command not found in PATH\n");
        } else if (errno == EACCES) {
            printf("Permission denied\n");
        } else {
            printf("Failed to execute command\n");
        }
        exit(errno);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            last_return_code = WEXITSTATUS(status);
            if (last_return_code == EACCES) {
                printf("Permission denied\n");
            } else if (last_return_code == ENOENT) {
                printf("Command not found in PATH\n");
            }
        }
    }
    return OK;
}

int execute_pipeline(command_list_t *clist) {
    if (clist->num == 0) {
        return WARN_NO_CMDS;
    }
    
    if (clist->num == 1) {
        // Single command, no pipes needed
        Built_In_Cmds cmd_type = exec_built_in_cmd(&clist->commands[0]);
        if (cmd_type == BI_CMD_EXIT) {
            return OK_EXIT;
        } else if (cmd_type == BI_EXECUTED) {
            return OK;
        }
        return exec_cmd(&clist->commands[0]);
    }
    
    // Multiple commands requiring pipes
    int pipes[CMD_MAX-1][2];
    pid_t pids[CMD_MAX];
    
    // Create all the necessary pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }
    
    // Fork and execute each command
    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        
        if (pids[i] < 0) {
            perror("fork");
            return ERR_EXEC_CMD;
        } 
        
        if (pids[i] == 0) {
            // Child process
            
            // Setup input from previous pipe (if not first command)
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Setup output to next pipe (if not last command)
            if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipes in child
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Execute the command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            
            // If execvp returns, there was an error
            if (errno == ENOENT) {
                fprintf(stderr, "Command not found in PATH\n");
            } else if (errno == EACCES) {
                fprintf(stderr, "Permission denied\n");
            } else {
                fprintf(stderr, "Failed to execute command\n");
            }
            exit(errno);
        }
    }
    
    // Parent process - close all pipes
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all children to complete
    for (int i = 0; i < clist->num; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        
        // Only set return code from the last command in the pipeline
        if (i == clist->num - 1 && WIFEXITED(status)) {
            last_return_code = WEXITSTATUS(status);
        }
    }
    
    return OK;
}

int exec_local_cmd_loop() {
    char cmd_line[SH_CMD_MAX];
    command_list_t cmd_list;
    int rc;
    
    for (int i = 0; i < CMD_MAX; i++) {
        alloc_cmd_buff(&cmd_list.commands[i]);
    }
    cmd_list.num = 0;

    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_line, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        
        // Remove trailing newline
        cmd_line[strcspn(cmd_line, "\n")] = '\0';
        
        // Build command list by splitting on pipes
        rc = build_cmd_list(cmd_line, &cmd_list);
        
        if (rc == WARN_NO_CMDS) {
            continue;
        } else if (rc == ERR_TOO_MANY_COMMANDS) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        }
        
        // Check if the first command is a built-in that requires special handling
        if (cmd_list.num == 1) {
            Built_In_Cmds cmd_type = match_command(cmd_list.commands[0].argv[0]);
            if (cmd_type == BI_CMD_EXIT) {
                printf("exiting...\n");
                break;
            }
        }
        
        // Execute the pipeline
        rc = execute_pipeline(&cmd_list);
        if (rc == OK_EXIT) {
            printf("exiting...\n");
            break;
        }
        
        // Clear commands for next iteration
        for (int i = 0; i < cmd_list.num; i++) {
            clear_cmd_buff(&cmd_list.commands[i]);
        }
        cmd_list.num = 0;
    }
    
    // Clean up
    free_cmd_list(&cmd_list);
    return OK;
}
