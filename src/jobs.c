#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "../include/jobs.h"

static job_t job_table[MAX_JOBS];
static int next_job_id = 1;

/* =========================================================
   INITIALIZE JOB TABLE
   ========================================================= */
void jobs_init(void)
{
    memset(job_table, 0, sizeof(job_table));
    next_job_id = 1;
}

/* =========================================================
   ADD JOB
   ========================================================= */
int job_add(pid_t pgid, const char *command, job_state_t state)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (job_table[i].job_id == 0)
        {
            job_table[i].job_id = next_job_id++;
            job_table[i].pgid = pgid;
            job_table[i].state = state;
            strncpy(job_table[i].command, command, MAX_JOB_COMMAND - 1);
            job_table[i].command[MAX_JOB_COMMAND - 1] = '\0';
            return job_table[i].job_id;
        }
    }
    printf("jobs: maximum number of jobs reached\n");
    return -1;
}

/* =========================================================
   FIND JOB BY JOB ID
   ========================================================= */
job_t *job_find(int job_id)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (job_table[i].job_id == job_id)
        {
            return &job_table[i];
        }
    }
    return NULL;
}

/* =========================================================
   FIND JOB BY PROCESS GROUP
   ========================================================= */
job_t *job_find_by_pgid(pid_t pgid)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (job_table[i].job_id != 0 && job_table[i].pgid == pgid)
        {
            return &job_table[i];
        }
    }
    return NULL;
}

/* =========================================================
   REMOVE JOB
   ========================================================= */
void job_remove(int job_id)
{
    job_t *job = job_find(job_id);
    if (job != NULL)
    {
        memset(job, 0, sizeof(job_t));
    }
}

/* =========================================================
   JOB STATE STRING
   ========================================================= */
static const char *state_string(job_state_t state)
{
    switch (state)
    {
        case JOB_RUNNING: return "Running";
        case JOB_STOPPED: return "Stopped";
        case JOB_DONE:    return "Done";
        default:          return "Unknown";
    }
}

/* =========================================================
   PRINT JOBS
   ========================================================= */
void jobs_print(void)
{
    int count = 0;
    printf("\n\033[38;5;205m╔════════════════════ ACTIVE PIPELINE JOBS ════════════════════╗\033[0m\n");
    printf("\033[38;5;141m%-6s %-12s %-10s %s\033[0m\n", "JID", "Status", "PGID", "Command");
    printf("\033[38;5;135m───────────────────────────────────────────────────────────────\033[0m\n");
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (job_table[i].job_id != 0 && job_table[i].state != JOB_DONE)
        {
            const char *color = (job_table[i].state == JOB_RUNNING) ? "\033[38;5;51m" : "\033[1;33m";
            printf("\033[38;5;213m[%d]\033[0m    %s%-12s\033[0m %-10d %s\n",
                job_table[i].job_id,
                color,
                state_string(job_table[i].state),
                job_table[i].pgid,
                job_table[i].command
            );
            count++;
        }
    }
    if (!count)
    {
        printf("  \033[38;5;213mNo active background or suspended jobs.\033[0m\n");
    }
    printf("\033[38;5;205m╚═══════════════════════════════════════════════════════════════╝\033[0m\n\n");
}

/* =========================================================
   MARK JOB STATUS CHANGES
   ========================================================= */
void job_stop(pid_t pgid)
{
    job_t *job = job_find_by_pgid(pgid);
    if (job != NULL) job->state = JOB_STOPPED;
}

void job_continue(pid_t pgid)
{
    job_t *job = job_find_by_pgid(pgid);
    if (job != NULL) job->state = JOB_RUNNING;
}

void job_done(pid_t pgid)
{
    job_t *job = job_find_by_pgid(pgid);
    if (job != NULL) job->state = JOB_DONE;
}
