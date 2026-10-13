#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include "../include/job_control.h"
#include "../include/jobs.h"

volatile pid_t current_fg_pgid = 0;
volatile sig_atomic_t sigint_received = 0;
volatile sig_atomic_t sigtstp_received = 0;

static struct termios orig_termios;
static pid_t orig_pgrp = 0;
static int have_orig_termios = 0;

/* Cleanly restore terminal attributes and foreground process group to Ubuntu shell */
void restore_terminal(void) {
    if (isatty(STDIN_FILENO)) {
        rl_deprep_terminal();
        if (have_orig_termios) {
            tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
        }
        if (orig_pgrp > 0) {
            signal(SIGTTOU, SIG_IGN);
            tcsetpgrp(STDIN_FILENO, orig_pgrp);
        }
    }
}

/* Signal handler for Ctrl+C at the prompt: Exit ShellForge and restore terminal to Ubuntu */
static void handle_sigint_prompt(int sig) {
    (void)sig;
    static const char exit_msg[] = "\n\033[38;5;205mExiting ShellForge via Ctrl+C. Goodbye!\033[0m\n";
    write(STDOUT_FILENO, exit_msg, sizeof(exit_msg) - 1);
    restore_terminal();
    _exit(0);
}

/* Signal handler for Ctrl+Z at the prompt: Suspend ShellForge back to Ubuntu */
static void handle_sigtstp_prompt(int sig) {
    (void)sig;
    static const char susp_msg[] = "\n\033[38;5;141m[Suspending ShellForge... Type 'fg' in your terminal to resume]\033[0m\n";
    write(STDOUT_FILENO, susp_msg, sizeof(susp_msg) - 1);
    restore_terminal();
    signal(SIGTSTP, SIG_DFL);
    kill(getpid(), SIGTSTP);
}

/* Signal handler for when ShellForge is resumed via 'fg' in Ubuntu */
static void handle_sigcont_prompt(int sig) {
    (void)sig;
    if (isatty(STDIN_FILENO)) {
        signal(SIGTTOU, SIG_IGN);
        tcsetpgrp(STDIN_FILENO, getpid());
    }
    install_prompt_signals();
    rl_prep_terminal(0);
    rl_on_new_line();
    rl_redisplay();
}

/* Signal handler for Ctrl+C while an external child command is executing */
static void handle_sigint_exec(int sig) {
    (void)sig;
    if (current_fg_pgid > 0) {
        kill(-current_fg_pgid, SIGINT);
    }
}

/* Signal handler for Ctrl+Z while an external child command is executing */
static void handle_sigtstp_exec(int sig) {
    (void)sig;
    if (current_fg_pgid > 0) {
        kill(-current_fg_pgid, SIGTSTP);
    }
}

void install_prompt_signals(void) {
    current_fg_pgid = 0;
    rl_catch_signals = 0; // Disable Readline signal trapping so our handlers run directly

    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = handle_sigint_prompt;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    struct sigaction sa_tstp;
    memset(&sa_tstp, 0, sizeof(sa_tstp));
    sa_tstp.sa_handler = handle_sigtstp_prompt;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = 0;
    sigaction(SIGTSTP, &sa_tstp, NULL);

    struct sigaction sa_cont;
    memset(&sa_cont, 0, sizeof(sa_cont));
    sa_cont.sa_handler = handle_sigcont_prompt;
    sigemptyset(&sa_cont.sa_mask);
    sa_cont.sa_flags = 0;
    sigaction(SIGCONT, &sa_cont, NULL);
}

void install_execution_signals(pid_t fg_pgid) {
    current_fg_pgid = fg_pgid;

    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = handle_sigint_exec;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    struct sigaction sa_tstp;
    memset(&sa_tstp, 0, sizeof(sa_tstp));
    sa_tstp.sa_handler = handle_sigtstp_exec;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = 0;
    sigaction(SIGTSTP, &sa_tstp, NULL);
}

void init_job_signals(void) {
    if (isatty(STDIN_FILENO)) {
        orig_pgrp = tcgetpgrp(STDIN_FILENO);
        if (tcgetattr(STDIN_FILENO, &orig_termios) == 0) {
            have_orig_termios = 1;
        }
        atexit(restore_terminal);

        signal(SIGQUIT, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

        pid_t shell_pgid = getpid();
        setpgid(shell_pgid, shell_pgid);
        tcsetpgrp(STDIN_FILENO, shell_pgid);
    }

    install_prompt_signals();
}

void do_jobs_builtin(void) {
    for (int i = 1; i <= MAX_JOBS; i++) {
        job_t *job = job_find(i);
        if (job && job->job_id != 0 && job->state != JOB_DONE) {
            int status;
            pid_t res = waitpid(-job->pgid, &status, WNOHANG | WUNTRACED);
            if (res > 0) {
                if (WIFSTOPPED(status)) job_stop(job->pgid);
                else if (WIFSIGNALED(status) || WIFEXITED(status)) job_done(job->pgid);
            }
        }
    }
    jobs_print();
}

void check_background_jobs(void) {
    for (int i = 1; i <= MAX_JOBS; i++) {
        job_t *job = job_find(i);
        if (job && job->job_id != 0 && job->state == JOB_RUNNING) {
            int status;
            pid_t res = waitpid(-job->pgid, &status, WNOHANG);
            if (res > 0) {
                if (WIFSIGNALED(status) || WIFEXITED(status)) {
                    printf("\033[38;5;141m[%d] Done\t\t\033[1;32m%s\033[0m\n", job->job_id, job->command);
                    job_done(job->pgid);
                }
            }
        }
    }
}

void do_fg_builtin(int job_id) {
    job_t *job = job_find(job_id);
    if (!job || job->job_id == 0 || job->state == JOB_DONE) {
        fprintf(stderr, "\033[1;31m[-] fg: no such job with ID [%d]\033[0m\n", job_id);
        return;
    }

    pid_t pgid = job->pgid;

    // Transfer terminal control to the job's process group
    if (isatty(STDIN_FILENO)) {
        tcsetpgrp(STDIN_FILENO, pgid);
    }

    install_execution_signals(pgid);

    // If stopped, send SIGCONT to resume
    if (job->state == JOB_STOPPED) {
        kill(-pgid, SIGCONT);
        job_continue(pgid);
    }

    printf("\033[38;5;141m%s\033[0m\n", job->command);

    int status;
    int job_stopped = 0;
    pid_t wpid = waitpid(-pgid, &status, WUNTRACED);
    if (wpid > 0 && WIFSTOPPED(status)) {
        job_stopped = 1;
    }

    // Reclaim terminal control for shell
    if (isatty(STDIN_FILENO)) {
        tcsetpgrp(STDIN_FILENO, getpgrp());
    }

    install_prompt_signals();

    if (job_stopped) {
        job_stop(pgid);
        printf("\n\033[38;5;141m[%d] Suspended / Stopped\t\t\033[1;33m%s\033[0m\n", job->job_id, job->command);
    } else {
        job_done(pgid);
        if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT) {
            printf("\n");
        }
    }
}

void do_bg_builtin(int job_id) {
    job_t *job = job_find(job_id);
    if (!job || job->job_id == 0 || job->state == JOB_DONE) {
        fprintf(stderr, "\033[1;31m[-] bg: no such job with ID [%d]\033[0m\n", job_id);
        return;
    }

    if (job->state == JOB_STOPPED) {
        kill(-job->pgid, SIGCONT);
        job_continue(job->pgid);
        printf("\033[38;5;141m[%d] %s &\033[0m\n", job->job_id, job->command);
    } else {
        printf("bg: job [%d] is already running in background\n", job->job_id);
    }
}
