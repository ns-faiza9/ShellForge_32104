#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include <sys/types.h>
#include <signal.h>

extern volatile pid_t current_fg_pgid;
extern volatile sig_atomic_t sigint_received;
extern volatile sig_atomic_t sigtstp_received;

void do_jobs_builtin(void);
void do_fg_builtin(int job_id);
void do_bg_builtin(int job_id);
void check_background_jobs(void);
void init_job_signals(void);
void install_prompt_signals(void);
void install_execution_signals(pid_t fg_pgid);
void restore_terminal(void);

#endif // JOB_CONTROL_H
