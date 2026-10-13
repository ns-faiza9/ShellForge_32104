#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include "../include/executor.h"
#include "../include/job_control.h"
#include "../include/jobs.h"

int execute_pipeline(ShellCommand *cmds, int cmd_count, int is_background, char *full_cmd_str) {
    if (cmd_count == 0) return 0;

    int num_pipes = cmd_count - 1;
    int *pipefds = NULL;

    if (num_pipes > 0) {
        pipefds = malloc(sizeof(int) * 2 * num_pipes);
        for (int i = 0; i < num_pipes; i++) {
            if (pipe(pipefds + i * 2) < 0) {
                fprintf(stderr, "\033[1;31m[-] shellforge: pipe creation failed (%s)\033[0m\n", strerror(errno));
                free(pipefds);
                return -1;
            }
        }
    }

    pid_t pgid = 0;
    pid_t last_child_pid = 0;

    for (int i = 0; i < cmd_count; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Child: Put in child process group
            if (i == 0) setpgid(0, 0);
            else setpgid(0, pgid);

            // Connect input pipe or redirected input file
            if (i > 0) {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            } else if (cmds[i].input_file) {
                int infd = open(cmds[i].input_file, O_RDONLY);
                if (infd < 0) {
                    fprintf(stderr, "\033[1;31m[-] shellforge: cannot open input file '%s': %s\033[0m\n",
                            cmds[i].input_file, strerror(errno));
                    exit(1);
                }
                dup2(infd, STDIN_FILENO);
                close(infd);
            }

            // Connect output pipe or redirected output file
            if (i < cmd_count - 1) {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            } else if (cmds[i].output_file) {
                int flags = O_WRONLY | O_CREAT | (cmds[i].append ? O_APPEND : O_TRUNC);
                int outfd = open(cmds[i].output_file, flags, 0644);
                if (outfd < 0) {
                    fprintf(stderr, "\033[1;31m[-] shellforge: cannot open output file '%s': %s\033[0m\n",
                            cmds[i].output_file, strerror(errno));
                    exit(1);
                }
                dup2(outfd, STDOUT_FILENO);
                close(outfd);
            }

            // Close all pipe copies in child
            if (num_pipes > 0) {
                for (int j = 0; j < 2 * num_pipes; j++) close(pipefds[j]);
            }

            // Restore all default signals in child process
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);

            // Execute program via PATH
            execvp(cmds[i].args[0], cmds[i].args);

            // Detailed Red Error on failure
            if (errno == ENOENT) {
                fprintf(stderr, "\033[1;31m[-] shellforge: command not found: '%s'\033[0m\n", cmds[i].args[0]);
            } else if (errno == EACCES) {
                fprintf(stderr, "\033[1;31m[-] shellforge: permission denied: '%s'\033[0m\n", cmds[i].args[0]);
            } else {
                fprintf(stderr, "\033[1;31m[-] shellforge: execution failed for '%s': %s\033[0m\n",
                        cmds[i].args[0], strerror(errno));
            }
            exit(127);
        } else if (pid > 0) {
            if (i == 0) pgid = pid;
            setpgid(pid, pgid);
            if (i == cmd_count - 1) last_child_pid = pid;
        } else {
            fprintf(stderr, "\033[1;31m[-] shellforge: fork error (%s)\033[0m\n", strerror(errno));
            if (pipefds) free(pipefds);
            return -1;
        }
    }

    // Close pipes in parent
    if (num_pipes > 0) {
        for (int j = 0; j < 2 * num_pipes; j++) close(pipefds[j]);
        free(pipefds);
    }

    int final_status = 0;
    if (!is_background) {
        // Hand over terminal control to foreground process group
        if (isatty(STDIN_FILENO)) {
            tcsetpgrp(STDIN_FILENO, pgid);
        }

        install_execution_signals(pgid);

        int any_stopped = 0;
        int any_sigint = 0;

        for (int i = 0; i < cmd_count; i++) {
            int wstatus;
            pid_t wpid = waitpid(-pgid, &wstatus, WUNTRACED);
            if (wpid > 0) {
                if (WIFSTOPPED(wstatus)) {
                    any_stopped = 1;
                }
                if (WIFSIGNALED(wstatus) && WTERMSIG(wstatus) == SIGINT) {
                    any_sigint = 1;
                }
                if (wpid == last_child_pid) {
                    final_status = wstatus;
                }
            }
        }

        // Reclaim terminal control for shell
        if (isatty(STDIN_FILENO)) {
            tcsetpgrp(STDIN_FILENO, getpgrp());
        }

        install_prompt_signals();

        if (any_stopped) {
            int jid = job_add(pgid, full_cmd_str, JOB_STOPPED);
            printf("\n\033[38;5;141m[%d] Suspended / Stopped\t\t\033[1;33m%s\033[0m\n", jid, full_cmd_str);
        } else if (any_sigint) {
            printf("\n");
        }
    } else {
        int jid = job_add(pgid, full_cmd_str, JOB_RUNNING);
        printf("\033[38;5;141m[%d]\033[0m \033[38;5;205mBackground process spawned: %s\033[0m\n", jid, full_cmd_str);
    }

    return WIFEXITED(final_status) ? WEXITSTATUS(final_status) : final_status;
}
