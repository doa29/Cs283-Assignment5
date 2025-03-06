#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = strdup(cmd_line);
    if (!cmd_buff->_cmd_buffer)
        return ERR_MEMORY;
    cmd_buff->argc = 0;
    cmd_buff->in_file = NULL;
    cmd_buff->out_file = NULL;
    cmd_buff->append = 0;
    char *token = strtok(cmd_buff->_cmd_buffer, " ");
    while (token && cmd_buff->argc < CMD_ARGV_MAX - 1) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            if (token)
                cmd_buff->in_file = token;
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            if (token) {
                cmd_buff->out_file = token;
                cmd_buff->append = 0;
            }
        } else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, " ");
            if (token) {
                cmd_buff->out_file = token;
                cmd_buff->append = 1;
            }
        } else {
            cmd_buff->argv[cmd_buff->argc++] = token;
        }
        token = strtok(NULL, " ");
    }
    cmd_buff->argv[cmd_buff->argc] = NULL;
    if (cmd_buff->argc == 0)
        return WARN_NO_CMDS;
    return OK;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    clist->num = 0;
    char *line = strdup(cmd_line);
    if (!line)
        return ERR_MEMORY;
    char *token = strtok(line, PIPE_STRING);
    while (token) {
        if (clist->num >= CMD_MAX) {
            free(line);
            return ERR_TOO_MANY_COMMANDS;
        }
        while (*token == ' ')
            token++;
        if (build_cmd_buff(token, &clist->commands[clist->num]) != OK) {
            token = strtok(NULL, PIPE_STRING);
            continue;
        }
        clist->num++;
        token = strtok(NULL, PIPE_STRING);
    }
    free(line);
    if (clist->num == 0)
        return WARN_NO_CMDS;
    return OK;
}

int free_cmd_list(command_list_t *cmd_lst) {
    int i;
    for (i = 0; i < cmd_lst->num; i++) {
        free(cmd_lst->commands[i]._cmd_buffer);
        cmd_lst->commands[i]._cmd_buffer = NULL;
    }
    return OK;
}

int execute_pipeline(command_list_t *clist) {
    int n = clist->num;
    int prev_fd = -1;
    pid_t pids[CMD_MAX];
    int i;
    for (i = 0; i < n; i++) {
        int pipefd[2];
        if (i < n - 1) {
            if (pipe(pipefd) < 0)
                return ERR_EXEC_CMD;
        }
        pid_t pid = fork();
        if (pid < 0)
            return ERR_EXEC_CMD;
        if (pid == 0) {
            if (clist->commands[i].in_file) {
                int fd = open(clist->commands[i].in_file, O_RDONLY);
                if (fd < 0)
                    exit(ERR_EXEC_CMD);
                dup2(fd, 0);
                close(fd);
            } else if (i > 0) {
                dup2(prev_fd, 0);
            }
            if (clist->commands[i].out_file) {
                int fd;
                if (clist->commands[i].append)
                    fd = open(clist->commands[i].out_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                else
                    fd = open(clist->commands[i].out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0)
                    exit(ERR_EXEC_CMD);
                dup2(fd, 1);
                close(fd);
            } else if (i < n - 1) {
                dup2(pipefd[1], 1);
            }
            if (i < n - 1)
                close(pipefd[0]);
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            exit(ERR_EXEC_CMD);
        } else {
            pids[i] = pid;
            if (i > 0)
                close(prev_fd);
            if (i < n - 1) {
                close(pipefd[1]);
                prev_fd = pipefd[0];
            }
        }
    }
    for (i = 0; i < n; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }
    return OK;
}

int exec_local_cmd_loop() {
    char input[SH_CMD_MAX];
    command_list_t clist;
    while (1) {
        printf(SH_PROMPT);
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }
        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0)
            continue;
        if (strcmp(input, EXIT_CMD) == 0)
            break;
        if (strncmp(input, "cd", 2) == 0) {
            char *cmd = strtok(input, " ");
            char *arg = strtok(NULL, " ");
            if (arg == NULL) {
                fprintf(stderr, "cd: missing argument\n");
            } else {
                if (chdir(arg) != 0)
                    perror("cd");
            }
            continue;
        }
        if (build_cmd_list(input, &clist) != OK) {
            write(1, CMD_WARN_NO_CMD, strlen(CMD_WARN_NO_CMD));
            continue;
        }
        execute_pipeline(&clist);
        free_cmd_list(&clist);
    }
    return OK;
}
