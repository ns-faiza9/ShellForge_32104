#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include "../include/builtin.h"
#include "../include/job_control.h"
#include "../include/jobs.h"
#include "../include/history.h"

/* Helper to get ~/.shellforge directory */
static void get_shellforge_dir(char *path, size_t size) {
    char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(path, size, "%s/.shellforge", home);
    mkdir(path, 0755);
}

void print_banner(void) {
    printf("\033[38;5;205m  ███████╗██╗  ██╗███████╗██╗     ██╗     ███████╗ ██████╗ ██████╗  ██████╗ ███████╗\n\033[0m");
    printf("\033[38;5;213m  ██╔════╝██║  ██║██╔════╝██║     ██║     ██╔════╝██╔═══██╗██╔══██╗██╔════╝ ██╔════╝\n\033[0m");
    printf("\033[38;5;141m  ███████╗███████║█████╗  ██║     ██║     █████╗  ██║   ██║██████╔╝██║  ███╗█████╗  \n\033[0m");
    printf("\033[38;5;135m  ╚════██║██╔══██║██╔══╝  ██║     ██║     ██╔══╝  ██║   ██║██╔══██╗██║   ██║██╔══╝  \n\033[0m");
    printf("\033[38;5;39m  ███████║██║  ██║███████╗███████╗███████╗██║     ╚██████╔╝██║  ██║╚██████╔╝███████╗\n\033[0m");
    printf("\033[38;5;51m  ╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝╚═╝      ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚══════╝\n\033[0m");
    printf("\n  \033[38;5;205m::\033[0m \033[38;5;141mForge Your Custom Terminal Experience\033[0m | \033[38;5;39mOS & Systems Programming Edition\033[0m\n");
    printf("  \033[38;5;51m::\033[0m \033[1;32mv2.0 | POSIX C11 | Pipelines, Redirections, Job Control & Custom Commands\033[0m\n\n");
    printf("  \033[38;5;141m[TIP]\033[0m  Type \033[38;5;205mhelp\033[0m to view grouped commands, or \033[38;5;39msysinfo\033[0m for live machine telemetry.\n");
    printf("  \033[38;5;205m────────────────────────────────────────────────────────────────────────────────────\033[0m\n\n");
}

static void builtin_help(void) {
    printf("\n");
    printf("\033[38;5;205m◆ Built-in Commands\033[0m \033[38;5;240m─────────────────────────────────────────────────────────────────\033[0m\n");
    printf("  \033[1;37mgo [dir]\033[0m                               change directory\n");
    printf("  \033[1;37mhome\033[0m                                   navigate to home directory (cd ~)\n");
    printf("  \033[1;37mback\033[0m                                   navigate to parent directory (cd ..)\n");
    printf("  \033[1;37mloc\033[0m                                    current directory\n");
    printf("  \033[1;37mscan [dir]\033[0m                             list files\n");
    printf("  \033[1;37mdetail [dir]\033[0m                           list all files with full details (ls -la)\n");
    printf("  \033[1;37mrecent [dir]\033[0m                           list files sorted by newest first (ls -lt)\n");
    printf("  \033[1;37mheavy [dir]\033[0m                            list files sorted by largest size (ls -lSh)\n");
    printf("  \033[1;37mlshidden [dir]\033[0m                         list hidden files\n");
    printf("  \033[1;37mtree [dir]\033[0m                             display directory tree structure\n");
    printf("  \033[1;37mpeek <file> [n]\033[0m                        preview file lines\n");
    printf("  \033[1;37mflush\033[0m                                  clear screen\n");
    printf("  \033[1;37mhistory\033[0m                                command history\n");
    printf("  \033[1;37mhelp\033[0m                                   command reference\n");
    printf("  \033[1;37mbye\033[0m                                    exit shell\n\n");

    printf("\033[38;5;51m◆ External Commands\033[0m \033[38;5;240m─────────────────────────────────────────────────────────────────\033[0m\n");
    printf("  \033[1;37mshow <file>\033[0m                            display file\n");
    printf("  \033[1;37minit <file>\033[0m                            create file\n");
    printf("  \033[1;37mfork <src> <dest>\033[0m                      copy file\n");
    printf("  \033[1;37mshift <src> <dest>\033[0m                     move or rename\n");
    printf("  \033[1;37mdel <target>\033[0m                           delete file\n");
    printf("  \033[1;37mhunt <dir> [pattern]\033[0m                   search files\n");
    printf("  \033[1;37msig <pattern> <file>\033[0m                   search text\n\n");

    printf("\033[38;5;141m◆ Date, Time & Calendar\033[0m \033[38;5;240m────────────────────────────────────────────────────────────\033[0m\n");
    printf("  \033[1;37mtoday\033[0m                                  current date\n");
    printf("  \033[1;37mnow\033[0m                                    current time\n");
    printf("  \033[1;37mday <YYYY-MM-DD>\033[0m                       day of week\n");
    printf("  \033[1;37mcal [month year]\033[0m                       monthly calendar\n");
    printf("  \033[1;37myr [year]\033[0m                              whole year calendar\n\n");

    printf("\033[38;5;213m◆ Tasks & Notes (Productivity)\033[0m \033[38;5;240m─────────────────────────────────────────────────────\033[0m\n");
    printf("  \033[1;37mTasks\033[0m                                  list tasks\n");
    printf("  \033[1;37mTasks add <text>\033[0m                       add task\n");
    printf("  \033[1;37mTasks done <id>\033[0m                        complete task\n");
    printf("  \033[1;37mTasks undo <id>\033[0m                        uncheck task\n");
    printf("  \033[1;37mTasks del <id>\033[0m                         delete task\n");
    printf("  \033[1;37mnotes\033[0m                                  list notes\n");
    printf("  \033[1;37mnotes add <title> : <body>\033[0m             save note\n");
    printf("  \033[1;37mnotes view <id>\033[0m                        read note\n");
    printf("  \033[1;37mnotes find <query>\033[0m                     search notes\n");
    printf("  \033[1;37mnotes del <id>\033[0m                         delete note\n\n");

    printf("\033[38;5;39m◆ Job Control & Telemetry\033[0m \033[38;5;240m──────────────────────────────────────────────────────────\033[0m\n");
    printf("  \033[1;37mjobs / tasks\033[0m                           list jobs\n");
    printf("  \033[1;37mfg <job_id>\033[0m                            foreground job\n");
    printf("  \033[1;37mbg <job_id>\033[0m                            background job\n");
    printf("  \033[1;37mkill-job <id>\033[0m                          stop job\n");
    printf("  \033[1;37msysinfo\033[0m                                system metrics\n\n");

    printf("\033[38;5;135m◆ Pipelines, Redirections & Signals\033[0m \033[38;5;240m────────────────────────────────────────────────\033[0m\n");
    printf("  \033[1;37mcmd1 | cmd2\033[0m                            pipeline\n");
    printf("  \033[1;37mcmd > file\033[0m                             redirect stdout\n");
    printf("  \033[1;37mcmd >> file\033[0m                            append stdout\n");
    printf("  \033[1;37mcmd < file\033[0m                             redirect stdin\n");
    printf("  \033[1;37mcmd &\033[0m                                  background run\n");
    printf("  \033[1;37mCtrl + C\033[0m                               exit shell\n");
    printf("  \033[1;37mCtrl + Z\033[0m                               suspend job\n");
    printf("\033[38;5;240m────────────────────────────────────────────────────────────────────────────────────\033[0m\n\n");
}

/* scan: Pretty-print visible files/directories (omitting hidden dotfiles) */
static void builtin_scan(const char *dir_path) {
    const char *target = (dir_path && strlen(dir_path) > 0) ? dir_path : ".";
    DIR *d = opendir(target);
    if (!d) {
        fprintf(stderr, "\033[1;31m[-] shellforge: scan: cannot open '%s': %s\033[0m\n", target, strerror(errno));
        return;
    }

    printf("\n\033[38;5;205m[SCAN DIRECTORY CONTENT] in '%s':\033[0m\n", target);
    printf("\033[38;5;141m%-10s %-12s %-10s %s\033[0m\n", "Type", "Permissions", "Size", "Name");
    printf("\033[38;5;135m──────────────────────────────────────────────────────────────────\033[0m\n");

    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue; // Exclude hidden files
        if (strstr(entry->d_name, "Zone.Identifier") != NULL) continue;

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", target, entry->d_name);
        struct stat st;
        char type_str[16] = "File";
        char perms[11] = "----------";
        long size = 0;

        if (lstat(fullpath, &st) == 0) {
            if (S_ISDIR(st.st_mode)) strcpy(type_str, "Directory");
            else if (S_ISLNK(st.st_mode)) strcpy(type_str, "Symlink");
            else strcpy(type_str, "File");

            perms[0] = S_ISDIR(st.st_mode) ? 'd' : (S_ISLNK(st.st_mode) ? 'l' : '-');
            perms[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
            perms[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
            perms[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
            perms[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
            perms[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
            perms[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
            perms[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
            perms[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
            perms[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';
            perms[10] = '\0';
            size = (long)st.st_size;
        }

        char size_str[32];
        if (size < 1024) snprintf(size_str, sizeof(size_str), "%ld B", size);
        else if (size < 1024 * 1024) snprintf(size_str, sizeof(size_str), "%.1f KB", size / 1024.0);
        else snprintf(size_str, sizeof(size_str), "%.1f MB", size / (1024.0 * 1024.0));

        const char *color = "\033[0m";
        if (strcmp(type_str, "Directory") == 0) color = "\033[38;5;39m";
        else if (perms[3] == 'x' || perms[6] == 'x' || perms[9] == 'x') color = "\033[1;32m";
        else color = "\033[38;5;213m";

        printf("%-10s %-12s %-10s %s%s\033[0m\n", type_str, perms, size_str, color, entry->d_name);
        count++;
    }
    closedir(d);
    printf("\033[38;5;135m──────────────────────────────────────────────────────────────────\033[0m\n");
    printf("\033[38;5;51mTotal Items: %d\033[0m\n\n", count);
}

/* lshidden: Exclusively displays hidden dotfiles, .gitignore, NTFS Zone Identifiers */
static void builtin_lshidden(const char *dir_path) {
    const char *target = (dir_path && strlen(dir_path) > 0) ? dir_path : ".";
    DIR *d = opendir(target);
    if (!d) {
        fprintf(stderr, "\033[1;31m[-] shellforge: lshidden failed on '%s': %s\033[0m\n", target, strerror(errno));
        return;
    }

    printf("\n\033[38;5;205m[HIDDEN & METADATA OBJECTS] in '%s':\033[0m\n", target);
    printf("\033[38;5;141m%-10s %-12s %-10s %s\033[0m\n", "Type", "Permissions", "Size", "Name");
    printf("\033[38;5;135m──────────────────────────────────────────────────────────────────\033[0m\n");

    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.' || strstr(entry->d_name, "Zone.Identifier") != NULL) {
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", target, entry->d_name);
            struct stat st;
            char type_str[16] = "File";
            char perms[11] = "----------";
            long size = 0;

            if (lstat(fullpath, &st) == 0) {
                if (S_ISDIR(st.st_mode)) strcpy(type_str, "Directory");
                else if (S_ISLNK(st.st_mode)) strcpy(type_str, "Symlink");
                else strcpy(type_str, "File");

                perms[0] = S_ISDIR(st.st_mode) ? 'd' : (S_ISLNK(st.st_mode) ? 'l' : '-');
                perms[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
                perms[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
                perms[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
                perms[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
                perms[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
                perms[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
                perms[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
                perms[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
                perms[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';
                perms[10] = '\0';
                size = (long)st.st_size;
            }

            char size_str[32];
            if (size < 1024) snprintf(size_str, sizeof(size_str), "%ld B", size);
            else if (size < 1024 * 1024) snprintf(size_str, sizeof(size_str), "%.1f KB", size / 1024.0);
            else snprintf(size_str, sizeof(size_str), "%.1f MB", size / (1024.0 * 1024.0));

            const char *color = "\033[0m";
            if (strcmp(type_str, "Directory") == 0) color = "\033[38;5;39m";
            else if (strstr(entry->d_name, "Zone.Identifier")) color = "\033[1;31m";
            else color = "\033[38;5;213m";

            printf("%-10s %-12s %-10s %s%s\033[0m\n", type_str, perms, size_str, color, entry->d_name);
            count++;
        }
    }
    closedir(d);
    printf("\033[38;5;135m──────────────────────────────────────────────────────────────────\033[0m\n");
    printf("\033[38;5;51mTotal Hidden Objects Found: %d\033[0m\n\n", count);
}

/* Helper recursive function for tree */
static void print_tree(const char *dir_path, const char *prefix, int depth, int max_depth, int *dir_count, int *file_count) {
    if (depth >= max_depth) return;
    DIR *d = opendir(dir_path);
    if (!d) return;

    struct dirent **namelist;
    int n = scandir(dir_path, &namelist, NULL, alphasort);
    closedir(d);
    if (n < 0) return;

    // Filter out . and .. and hidden dotfiles
    int valid_count = 0;
    for (int i = 0; i < n; i++) {
        if (namelist[i]->d_name[0] != '.' && strstr(namelist[i]->d_name, "Zone.Identifier") == NULL) {
            valid_count++;
        }
    }

    int seen = 0;
    for (int i = 0; i < n; i++) {
        char *name = namelist[i]->d_name;
        if (name[0] == '.' || strstr(name, "Zone.Identifier") != NULL) {
            free(namelist[i]);
            continue;
        }
        seen++;
        int is_last = (seen == valid_count);

        char child_path[1024];
        snprintf(child_path, sizeof(child_path), "%s/%s", dir_path, name);

        struct stat st;
        int is_dir = 0;
        if (lstat(child_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            is_dir = 1;
            (*dir_count)++;
        } else {
            (*file_count)++;
        }

        const char *branch = is_last ? "└── " : "├── ";
        const char *color = is_dir ? "\033[1;34m" : "\033[0m";

        printf("%s%s%s%s\033[0m\n", prefix, branch, color, name);

        if (is_dir) {
            char new_prefix[512];
            snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "│   ");
            print_tree(child_path, new_prefix, depth + 1, max_depth, dir_count, file_count);
        }

        free(namelist[i]);
    }
    free(namelist);
}

static void builtin_tree(const char *dir_path) {
    const char *target = (dir_path && strlen(dir_path) > 0) ? dir_path : ".";
    printf("\n\033[1;34m%s\033[0m\n", target);
    int dirs = 0, files = 0;
    print_tree(target, "", 0, 4, &dirs, &files);
    printf("\n\033[38;5;141m%d directories, %d files\033[0m\n\n", dirs, files);
}

/* loc: show current working directory */
static void builtin_loc(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        printf("\033[38;5;39m%s\033[0m\n", cwd);
    } else {
        fprintf(stderr, "\033[1;31m[-] shellforge: loc failed: %s\033[0m\n", strerror(errno));
    }
}

/* peek: Preview first n lines of file */
static void builtin_peek(const char *filename, int max_lines) {
    if (!filename) {
        fprintf(stderr, "\033[1;31m[-] Usage error: peek <filename> [num_lines]\033[0m\n");
        return;
    }
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "\033[1;31m[-] shellforge: peek failed for '%s': %s\033[0m\n", filename, strerror(errno));
        return;
    }
    if (max_lines <= 0) max_lines = 10;
    printf("\n\033[38;5;141m─── [PEEK: %s (first %d lines)] ───\033[0m\n", filename, max_lines);
    char line[1024];
    int line_num = 1;
    while (fgets(line, sizeof(line), f) && line_num <= max_lines) {
        printf("\033[38;5;205m%3d │\033[0m %s", line_num++, line);
    }
    printf("\n\033[38;5;141m────────────────────────────────────\033[0m\n\n");
    fclose(f);
}

/* sysinfo: Live system telemetry */
static void builtin_sysinfo(void) {
    struct utsname u;
    if (uname(&u) != 0) {
        fprintf(stderr, "\033[1;31m[-] shellforge: uname failed: %s\033[0m\n", strerror(errno));
        return;
    }
    char *user = getenv("USER");
    if (!user) user = "unknown";

    printf("\n\033[38;5;205m════════════════════ SHELLFORGE SYSTEM TELEMETRY ════════════════════\033[0m\n");
    printf("  \033[38;5;141mShell Version:\033[0m  ShellForge v2.0 (Custom POSIX Systems Programming)\n");
    printf("  \033[38;5;141mCurrent User:\033[0m   \033[38;5;51m%s\033[0m\n", user);
    printf("  \033[38;5;141mOperating Sys:\033[0m  \033[38;5;213m%s\033[0m\n", u.sysname);
    printf("  \033[38;5;141mHost Node:\033[0m      %s\n", u.nodename);
    printf("  \033[38;5;141mKernel Release:\033[0m %s\n", u.release);
    printf("  \033[38;5;141mArchitecture:\033[0m   %s\n", u.machine);
    printf("  \033[38;5;141mProcess ID:\033[0m     %d (Parent: %d)\n", getpid(), getppid());
    printf("\033[38;5;205m═════════════════════════════════════════════════════════════════════\033[0m\n\n");
}

/* flush / clear */
static void builtin_flush(void) {
    printf("\033[H\033[J");
}

/* kill-job */
static void builtin_kill_job(int job_id) {
    job_t *job = job_find(job_id);
    if (!job || job->job_id == 0 || job->state == JOB_DONE) {
        fprintf(stderr, "\033[1;31m[-] shellforge: kill-job: no active job with ID [%d]\033[0m\n", job_id);
        return;
    }
    kill(-job->pgid, SIGTERM);
    printf("\033[1;31mSent SIGTERM to Job [%d] (PGID %d: %s)\033[0m\n", 
           job->job_id, job->pgid, job->command);
}

/* today: format current date */
static void builtin_today(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buf[128];
    strftime(buf, sizeof(buf), "%A, %B %d, %Y", t);
    printf("\033[38;5;205mToday:\033[0m \033[38;5;51m%s\033[0m\n", buf);
}

/* now: format current time */
static void builtin_now(void) {
    time_t t_now = time(NULL);
    struct tm *t = localtime(&t_now);
    char buf[128];
    strftime(buf, sizeof(buf), "%I:%M:%S %p (%Z)", t);
    printf("\033[38;5;205mTime:\033[0m \033[38;5;51m%s\033[0m\n", buf);
}

/* month_from_name helper */
static int month_from_name(const char *str) {
    if (!str) return 0;
    const char *months_abbr[] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};
    const char *months_full[] = {"january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december"};
    for (int i = 0; i < 12; i++) {
        if (strncasecmp(str, months_abbr[i], 3) == 0 || strcasecmp(str, months_full[i]) == 0) {
            return i + 1;
        }
    }
    return 0;
}

/* day [date]: show what day of the week that date was/is */
static void builtin_day(char **args) {
    if (!args || !args[1]) {
        fprintf(stderr, "\033[1;31m[-] Usage: day <date> (e.g. day 2026 10 13, day 12 oct 26, day 2026-10-13)\033[0m\n");
        return;
    }

    int y = 0, m = 0, d = 0;

    // Check if arguments were passed separated by spaces (e.g., day 2026 10 13 or day 12 oct 26)
    if (args[1] && args[2] && args[3]) {
        // Form 1: day 12 oct 26 / 12 oct 2026
        int m_test = month_from_name(args[2]);
        if (m_test > 0) {
            d = atoi(args[1]);
            m = m_test;
            y = atoi(args[3]);
        }
        // Form 2: day oct 12 26 / oct 12 2026
        else if ((m_test = month_from_name(args[1])) > 0) {
            m = m_test;
            d = atoi(args[2]);
            y = atoi(args[3]);
        }
        // Form 3: day 2026 10 13
        else {
            int a = atoi(args[1]);
            int b = atoi(args[2]);
            int c = atoi(args[3]);
            if (a > 100) {
                // YYYY MM DD
                y = a; m = b; d = c;
            } else if (c > 100) {
                // DD MM YYYY
                d = a; m = b; y = c;
            } else {
                // Ambiguous or 2-digit year
                y = a; m = b; d = c;
            }
        }
    } else {
        // Single argument: day 2026-10-13 or 12/10/2026 or 12-oct-2026
        const char *datestr = args[1];
        char month_str[32] = "";

        if (sscanf(datestr, "%d-%d-%d", &y, &m, &d) == 3 ||
            sscanf(datestr, "%d/%d/%d", &y, &m, &d) == 3) {
            // Check if year is first or last
            if (y < 32 && d > 100) {
                int temp = y;
                y = d;
                d = temp;
            }
        } else if (sscanf(datestr, "%d-%31[a-zA-Z]-%d", &d, month_str, &y) == 3 ||
                   sscanf(datestr, "%d/%31[a-zA-Z]/%d", &d, month_str, &y) == 3) {
            m = month_from_name(month_str);
        } else if (sscanf(datestr, "%31[a-zA-Z]-%d-%d", month_str, &d, &y) == 3) {
            m = month_from_name(month_str);
        }
    }

    // Handle 2-digit year (e.g. 26 -> 2026)
    if (y > 0 && y < 100) {
        y += 2000;
    }

    if (y <= 0 || m <= 0 || m > 12 || d <= 0 || d > 31) {
        fprintf(stderr, "\033[1;31m[-] shellforge: invalid date. Examples: 'day 2026 10 13', 'day 12 oct 26', 'day 2026-10-13'\033[0m\n");
        return;
    }

    struct tm timeinfo;
    memset(&timeinfo, 0, sizeof(struct tm));
    timeinfo.tm_year = y - 1900;
    timeinfo.tm_mon = m - 1;
    timeinfo.tm_mday = d;
    timeinfo.tm_isdst = -1;

    if (mktime(&timeinfo) == (time_t)-1) {
        fprintf(stderr, "\033[1;31m[-] shellforge: unable to parse date\033[0m\n");
        return;
    }

    const char *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    const char *months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

    printf("\033[38;5;205m%s %d, %d\033[0m is a \033[38;5;51m\033[1m%s\033[0m\n",
           months[m - 1], d, y, days[timeinfo.tm_wday]);
}

/* cal [month year]: interactive colored calendar grid */
static int days_in_month(int m, int y) {
    if (m == 2) {
        if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) return 29;
        return 28;
    }
    if (m == 4 || m == 6 || m == 9 || m == 11) return 30;
    return 31;
}

static void builtin_cal(int m, int y) {
    time_t cur = time(NULL);
    struct tm *today_tm = localtime(&cur);
    int current_day = today_tm->tm_mday;
    int current_mon = today_tm->tm_mon + 1;
    int current_yr = today_tm->tm_year + 1900;

    if (m <= 0 || m > 12) m = current_mon;
    if (y <= 0) y = current_yr;

    const char *month_names[] = {
        "", "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    struct tm first_day;
    memset(&first_day, 0, sizeof(struct tm));
    first_day.tm_year = y - 1900;
    first_day.tm_mon = m - 1;
    first_day.tm_mday = 1;
    first_day.tm_isdst = -1;
    mktime(&first_day);
    int start_wday = first_day.tm_wday; // 0 = Sunday

    printf("\n\033[38;5;205m     %s %d\033[0m\n", month_names[m], y);
    printf("\033[38;5;141mSu  Mo  Tu  We  Th  Fr  Sa\033[0m\n");
    printf("\033[38;5;135m──────────────────────────\033[0m\n");

    for (int i = 0; i < start_wday; i++) {
        printf("    ");
    }

    int total_days = days_in_month(m, y);
    int col = start_wday;

    for (int day = 1; day <= total_days; day++) {
        if (day == current_day && m == current_mon && y == current_yr) {
            // Highlight today in bright cyan / bold
            printf("\033[1;37m\033[48;5;205m%2d\033[0m  ", day);
        } else if (col == 0 || col == 6) {
            // Weekend in pink
            printf("\033[38;5;213m%2d\033[0m  ", day);
        } else {
            printf("\033[38;5;51m%2d\033[0m  ", day);
        }

        col++;
        if (col == 7) {
            printf("\n");
            col = 0;
        }
    }
    if (col != 0) printf("\n");
    printf("\n");
}

static void builtin_yr(int y) {
    time_t cur = time(NULL);
    struct tm *today_tm = localtime(&cur);
    int current_day = today_tm->tm_mday;
    int current_mon = today_tm->tm_mon + 1;
    int current_yr = today_tm->tm_year + 1900;

    if (y <= 0) y = current_yr;
    if (y < 100) y += 2000;

    const char *month_names[] = {
        "", "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    printf("\n\033[38;5;205m╔══════════════════════════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[38;5;205m║\033[0m                                 \033[38;5;141mCALENDAR YEAR %4d\033[0m                                \033[38;5;205m║\033[0m\n", y);
    printf("\033[38;5;205m╚══════════════════════════════════════════════════════════════════════════════════╝\033[0m\n\n");

    // Display 12 months in 4 rows of 3 columns
    for (int q = 0; q < 4; q++) {
        int m1 = q * 3 + 1;
        int m2 = q * 3 + 2;
        int m3 = q * 3 + 3;

        // Month Names header
        printf("       \033[38;5;205m%-20s\033[0m       \033[38;5;205m%-20s\033[0m       \033[38;5;205m%-20s\033[0m\n",
               month_names[m1], month_names[m2], month_names[m3]);

        // Weekday header
        printf("\033[38;5;141mSu Mo Tu We Th Fr Sa\033[0m   \033[38;5;141mSu Mo Tu We Th Fr Sa\033[0m   \033[38;5;141mSu Mo Tu We Th Fr Sa\033[0m\n");

        // Determine starting day of week for all 3 months
        int start[3], total[3];
        for (int i = 0; i < 3; i++) {
            int m_curr = q * 3 + 1 + i;
            struct tm f_day;
            memset(&f_day, 0, sizeof(struct tm));
            f_day.tm_year = y - 1900;
            f_day.tm_mon = m_curr - 1;
            f_day.tm_mday = 1;
            f_day.tm_isdst = -1;
            mktime(&f_day);
            start[i] = f_day.tm_wday;
            total[i] = days_in_month(m_curr, y);
        }

        int day_idx[3] = {1, 1, 1};

        // Each month grid can take up to 6 rows
        for (int r = 0; r < 6; r++) {
            int any_printed = 0;
            for (int i = 0; i < 3; i++) {
                int m_curr = q * 3 + 1 + i;
                for (int c = 0; c < 7; c++) {
                    int slot = r * 7 + c;
                    if (slot < start[i] || day_idx[i] > total[i]) {
                        printf("   ");
                    } else {
                        any_printed = 1;
                        int d = day_idx[i];
                        if (d == current_day && m_curr == current_mon && y == current_yr) {
                            printf("\033[1;37m\033[48;5;205m%2d\033[0m ", d);
                        } else if (c == 0 || c == 6) {
                            printf("\033[38;5;213m%2d\033[0m ", d);
                        } else {
                            printf("\033[38;5;51m%2d\033[0m ", d);
                        }
                        day_idx[i]++;
                    }
                }
                if (i < 2) printf("  ");
            }
            printf("\n");
            if (!any_printed && day_idx[0] > total[0] && day_idx[1] > total[1] && day_idx[2] > total[2]) {
                break;
            }
        }
        printf("\n");
    }
}

/* Tasks: to-do manager with checkboxes */
static void builtin_tasks(char **args) {
    char dir[512], filepath[600];
    get_shellforge_dir(dir, sizeof(dir));
    snprintf(filepath, sizeof(filepath), "%s/tasks.txt", dir);

    // Subcommand: add <text>
    if (args[1] && strcmp(args[1], "add") == 0) {
        if (!args[2]) {
            fprintf(stderr, "\033[1;31m[-] Usage: Tasks add <description of task>\033[0m\n");
            return;
        }
        char desc[512] = "";
        for (int i = 2; args[i]; i++) {
            if (i > 2) strcat(desc, " ");
            strcat(desc, args[i]);
        }
        FILE *f = fopen(filepath, "a");
        if (!f) {
            fprintf(stderr, "\033[1;31m[-] shellforge: cannot open tasks file: %s\033[0m\n", strerror(errno));
            return;
        }
        fprintf(f, "0|%s\n", desc);
        fclose(f);
        printf("\033[38;5;51m[+] Added task: [ ] %s\033[0m\n", desc);
        return;
    }

    // Subcommand: done <id> or undo <id>
    if (args[1] && (strcmp(args[1], "done") == 0 || strcmp(args[1], "undo") == 0)) {
        if (!args[2]) {
            fprintf(stderr, "\033[1;31m[-] Usage: Tasks %s <task_id>\033[0m\n", args[1]);
            return;
        }
        int target_id = atoi(args[2]);
        int new_status = (strcmp(args[1], "done") == 0) ? 1 : 0;

        FILE *f = fopen(filepath, "r");
        if (!f) {
            fprintf(stderr, "\033[1;31m[-] shellforge: no tasks recorded yet.\033[0m\n");
            return;
        }
        char lines[256][600];
        int count = 0;
        char buf[512];
        while (fgets(buf, sizeof(buf), f) && count < 256) {
            buf[strcspn(buf, "\r\n")] = '\0';
            if (strlen(buf) > 0) {
                strncpy(lines[count++], buf, sizeof(lines[0]) - 1);
            }
        }
        fclose(f);

        if (target_id < 1 || target_id > count) {
            fprintf(stderr, "\033[1;31m[-] shellforge: task ID %d not found (range 1-%d)\033[0m\n", target_id, count);
            return;
        }

        // Update status
        char *pipe_pos = strchr(lines[target_id - 1], '|');
        if (pipe_pos) {
            char desc[512];
            strncpy(desc, pipe_pos + 1, sizeof(desc) - 1);
            desc[sizeof(desc) - 1] = '\0';
            snprintf(lines[target_id - 1], sizeof(lines[0]), "%d|%s", new_status, desc);
        }

        f = fopen(filepath, "w");
        if (!f) return;
        for (int i = 0; i < count; i++) {
            fprintf(f, "%s\n", lines[i]);
        }
        fclose(f);

        printf("\033[38;5;51m[+] Task #%d marked as %s\033[0m\n", target_id, new_status ? "[X] Completed" : "[ ] Pending");
        return;
    }

    // Subcommand: del <id>
    if (args[1] && strcmp(args[1], "del") == 0) {
        if (!args[2]) {
            fprintf(stderr, "\033[1;31m[-] Usage: Tasks del <task_id>\033[0m\n");
            return;
        }
        int target_id = atoi(args[2]);
        FILE *f = fopen(filepath, "r");
        if (!f) {
            fprintf(stderr, "\033[1;31m[-] shellforge: no tasks recorded yet.\033[0m\n");
            return;
        }
        char lines[256][512];
        int count = 0;
        char buf[512];
        while (fgets(buf, sizeof(buf), f) && count < 256) {
            buf[strcspn(buf, "\r\n")] = '\0';
            if (strlen(buf) > 0) {
                strncpy(lines[count++], buf, sizeof(lines[0]) - 1);
            }
        }
        fclose(f);

        if (target_id < 1 || target_id > count) {
            fprintf(stderr, "\033[1;31m[-] shellforge: task ID %d not found (range 1-%d)\033[0m\n", target_id, count);
            return;
        }

        f = fopen(filepath, "w");
        if (!f) return;
        for (int i = 0; i < count; i++) {
            if (i != target_id - 1) {
                fprintf(f, "%s\n", lines[i]);
            }
        }
        fclose(f);
        printf("\033[38;5;205m[-] Deleted Task #%d\033[0m\n", target_id);
        return;
    }

    // Default: list all tasks with checkboxes
    FILE *f = fopen(filepath, "r");
    if (!f) {
        printf("\n\033[38;5;205m[TASKS & TO-DO CHECKLIST]\033[0m\n");
        printf("  \033[38;5;141mNo tasks recorded yet.\033[0m Use: \033[38;5;51mTasks add <description>\033[0m\n\n");
        return;
    }

    printf("\n\033[38;5;205m╔═══════════════════════ SHELLFORGE TASK CHECKLIST ═══════════════════════╗\033[0m\n");
    char buf[512];
    int id = 1;
    int pending = 0, completed = 0;

    while (fgets(buf, sizeof(buf), f)) {
        buf[strcspn(buf, "\r\n")] = '\0';
        if (strlen(buf) == 0) continue;
        int status = 0;
        char *sep = strchr(buf, '|');
        char *desc = "";
        if (sep) {
            *sep = '\0';
            status = atoi(buf);
            desc = sep + 1;
        } else {
            desc = buf;
        }

        if (status) {
            completed++;
            printf("  \033[38;5;141m#%2d\033[0m  \033[1;32m[X]\033[0m  \033[38;5;245m%s\033[0m\n", id, desc);
        } else {
            pending++;
            printf("  \033[38;5;141m#%2d\033[0m  \033[38;5;205m[ ]\033[0m  \033[1;37m%s\033[0m\n", id, desc);
        }
        id++;
    }
    fclose(f);

    printf("\033[38;5;205m╚═════════════════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("  \033[38;5;213mSummary:\033[0m \033[38;5;205m%d pending\033[0m | \033[1;32m%d completed\033[0m | Commands: \033[38;5;51mTasks add\033[0m, \033[38;5;51mTasks done <id>\033[0m, \033[38;5;51mTasks undo <id>\033[0m, \033[38;5;51mTasks del <id>\033[0m\n\n",
           pending, completed);
}

/* notes: keep notes / leaf / node manager */
static void builtin_notes(char **args) {
    char dir[512], filepath[600];
    get_shellforge_dir(dir, sizeof(dir));
    snprintf(filepath, sizeof(filepath), "%s/notes.txt", dir);

    // Subcommand: add <title> : <body>
    if (args[1] && strcmp(args[1], "add") == 0) {
        if (!args[2]) {
            fprintf(stderr, "\033[1;31m[-] Usage: notes add <Title> : <Content of note>\033[0m\n");
            return;
        }
        char all[1024] = "";
        for (int i = 2; args[i]; i++) {
            if (i > 2) strcat(all, " ");
            strcat(all, args[i]);
        }
        char *colon = strchr(all, ':');
        char title[256], body[768];
        if (colon) {
            *colon = '\0';
            strncpy(title, all, sizeof(title) - 1);
            title[sizeof(title) - 1] = '\0';
            strncpy(body, colon + 1, sizeof(body) - 1);
            body[sizeof(body) - 1] = '\0';
        } else {
            strncpy(title, all, sizeof(title) - 1);
            title[sizeof(title) - 1] = '\0';
            strcpy(body, "(empty)");
        }

        // Trim leading spaces in body
        char *trimmed_body = body;
        while (*trimmed_body == ' ') trimmed_body++;

        FILE *f = fopen(filepath, "a");
        if (!f) {
            fprintf(stderr, "\033[1;31m[-] shellforge: cannot open notes file: %s\033[0m\n", strerror(errno));
            return;
        }
        fprintf(f, "%s|%s\n", title, trimmed_body);
        fclose(f);
        printf("\033[38;5;51m[+] Note Node Created: '\033[1;37m%s\033[38;5;51m'\033[0m\n", title);
        return;
    }

    // Subcommand: find <query>
    if (args[1] && strcmp(args[1], "find") == 0) {
        if (!args[2]) {
            fprintf(stderr, "\033[1;31m[-] Usage: notes find <query>\033[0m\n");
            return;
        }
        FILE *f = fopen(filepath, "r");
        if (!f) {
            fprintf(stderr, "\033[1;31m[-] shellforge: no notes recorded yet.\033[0m\n");
            return;
        }
        printf("\n\033[38;5;205m[SEARCH RESULTS IN NOTES: '%s']\033[0m\n", args[2]);
        char buf[1024];
        int id = 1, found = 0;
        while (fgets(buf, sizeof(buf), f)) {
            buf[strcspn(buf, "\r\n")] = '\0';
            if (strcasestr(buf, args[2]) != NULL) {
                found++;
                char *sep = strchr(buf, '|');
                if (sep) {
                    *sep = '\0';
                    printf("  \033[38;5;141mNode #%d ──\033[0m \033[38;5;205m[%s]\033[0m: \033[38;5;51m%s\033[0m\n", id, buf, sep + 1);
                }
            }
            id++;
        }
        fclose(f);
        if (!found) printf("  \033[38;5;245mNo matching note nodes found.\033[0m\n");
        printf("\n");
        return;
    }

    // Subcommand: view <id>
    if (args[1] && strcmp(args[1], "view") == 0) {
        if (!args[2]) {
            fprintf(stderr, "\033[1;31m[-] Usage: notes view <note_id>\033[0m\n");
            return;
        }
        int target_id = atoi(args[2]);
        FILE *f = fopen(filepath, "r");
        if (!f) return;
        char buf[1024];
        int id = 1;
        while (fgets(buf, sizeof(buf), f)) {
            buf[strcspn(buf, "\r\n")] = '\0';
            if (id == target_id) {
                char *sep = strchr(buf, '|');
                if (sep) {
                    *sep = '\0';
                    printf("\n\033[38;5;205m════════════════════ NOTE NODE #%d ════════════════════\033[0m\n", id);
                    printf("  \033[38;5;141mTitle:\033[0m   \033[1;37m%s\033[0m\n", buf);
                    printf("  \033[38;5;141mContent:\033[0m \033[38;5;51m%s\033[0m\n", sep + 1);
                    printf("\033[38;5;205m═══════════════════════════════════════════════════════\033[0m\n\n");
                }
                fclose(f);
                return;
            }
            id++;
        }
        fclose(f);
        fprintf(stderr, "\033[1;31m[-] Note #%d not found.\033[0m\n", target_id);
        return;
    }

    // Subcommand: del <id>
    if (args[1] && strcmp(args[1], "del") == 0) {
        if (!args[2]) {
            fprintf(stderr, "\033[1;31m[-] Usage: notes del <note_id>\033[0m\n");
            return;
        }
        int target_id = atoi(args[2]);
        FILE *f = fopen(filepath, "r");
        if (!f) return;
        char lines[256][1024];
        int count = 0;
        char buf[1024];
        while (fgets(buf, sizeof(buf), f) && count < 256) {
            buf[strcspn(buf, "\r\n")] = '\0';
            if (strlen(buf) > 0) {
                strncpy(lines[count++], buf, sizeof(lines[0]) - 1);
            }
        }
        fclose(f);

        if (target_id < 1 || target_id > count) {
            fprintf(stderr, "\033[1;31m[-] Note ID %d not found.\033[0m\n", target_id);
            return;
        }

        f = fopen(filepath, "w");
        if (!f) return;
        for (int i = 0; i < count; i++) {
            if (i != target_id - 1) {
                fprintf(f, "%s\n", lines[i]);
            }
        }
        fclose(f);
        printf("\033[38;5;205m[-] Deleted Note Node #%d\033[0m\n", target_id);
        return;
    }

    // Default: List all notes as Keep leaf/nodes
    FILE *f = fopen(filepath, "r");
    if (!f) {
        printf("\n\033[38;5;205m[SHELLFORGE KEEP NOTES / NODES]\033[0m\n");
        printf("  \033[38;5;141mNo notes recorded yet.\033[0m Use: \033[38;5;51mnotes add <Title> : <Content>\033[0m\n\n");
        return;
    }

    printf("\n\033[38;5;205m╔══════════════════════ SHELLFORGE KEEP NOTES / NODES ══════════════════════╗\033[0m\n");
    char buf[1024];
    int id = 1;
    while (fgets(buf, sizeof(buf), f)) {
        buf[strcspn(buf, "\r\n")] = '\0';
        if (strlen(buf) == 0) continue;
        char *sep = strchr(buf, '|');
        char *title = buf;
        char *body = "(empty)";
        if (sep) {
            *sep = '\0';
            body = sep + 1;
        }
        printf("  \033[38;5;141mNode #%2d\033[0m ── \033[1;37m[\033[38;5;205m%s\033[1;37m]\033[0m\n", id, title);
        printf("            \033[38;5;51m%s\033[0m\n", body);
        printf("  \033[38;5;135m┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄\033[0m\n");
        id++;
    }
    fclose(f);
    printf("\033[38;5;205m╚═════════════════════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("  Commands: \033[38;5;51mnotes add <Title> : <body>\033[0m | \033[38;5;51mnotes view <id>\033[0m | \033[38;5;51mnotes find <query>\033[0m | \033[38;5;51mnotes del <id>\033[0m\n\n");
}

int is_builtin(const command_t *cmd) {
    if (cmd == NULL || cmd->argc == 0 || cmd->argv[0] == NULL) {
        return 0;
    }

    const char *name = cmd->argv[0];
    return (strcmp(name, "exit") == 0 ||
            strcmp(name, "bye") == 0 ||
            strcmp(name, "quit") == 0 ||
            strcmp(name, "help") == 0 ||
            strcmp(name, "cd") == 0 ||
            strcmp(name, "go") == 0 ||
            strcmp(name, "home") == 0 ||
            strcmp(name, "back") == 0 ||
            strcmp(name, "goto") == 0 ||
            strcmp(name, "loc") == 0 ||
            strcmp(name, "pwd") == 0 ||
            strcmp(name, "scan") == 0 ||
            strcmp(name, "tree") == 0 ||
            strcmp(name, "lshidden") == 0 ||
            strcmp(name, "ls-hidden") == 0 ||
            strcmp(name, "peek") == 0 ||
            strcmp(name, "sysinfo") == 0 ||
            strcmp(name, "flush") == 0 ||
            strcmp(name, "cls") == 0 ||
            strcmp(name, "clear") == 0 ||
            strcmp(name, "history") == 0 ||
            strcmp(name, "hist") == 0 ||
            strcmp(name, "jobs") == 0 ||
            strcmp(name, "tasks") == 0 ||
            strcmp(name, "Tasks") == 0 ||
            strcmp(name, "notes") == 0 ||
            strcmp(name, "today") == 0 ||
            strcmp(name, "now") == 0 ||
            strcmp(name, "day") == 0 ||
            strcmp(name, "cal") == 0 ||
            strcmp(name, "yr") == 0 ||
            strcmp(name, "fg") == 0 ||
            strcmp(name, "bg") == 0 ||
            strcmp(name, "kill-job") == 0);
}

int handle_builtin_commands(char **args) {
    if (args == NULL || args[0] == NULL) return 0;

    // 1. exit / bye / quit
    if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "bye") == 0 || strcmp(args[0], "quit") == 0) {
        printf("\033[38;5;205mExiting ShellForge. Goodbye!\033[0m\n");
        exit(0);
    }

    // 2. help
    if (strcmp(args[0], "help") == 0) {
        builtin_help();
        return 1;
    }

    // 3. go / cd / goto / back / home
    if (strcmp(args[0], "go") == 0 || strcmp(args[0], "cd") == 0 || 
        strcmp(args[0], "goto") == 0 || strcmp(args[0], "back") == 0 ||
        strcmp(args[0], "home") == 0) {
        const char *target = args[1];
        if (strcmp(args[0], "back") == 0) {
            target = "..";
        } else if (strcmp(args[0], "home") == 0) {
            target = "~";
        }
        if (target == NULL || strcmp(target, "~") == 0) {
            char *home = getenv("HOME");
            if (home) {
                if (chdir(home) != 0) {
                    fprintf(stderr, "\033[1;31m[-] shellforge: go: HOME directory access failed: %s\033[0m\n", strerror(errno));
                }
            } else {
                fprintf(stderr, "\033[1;31m[-] shellforge: go: HOME environment variable not set\033[0m\n");
            }
        } else {
            if (chdir(target) != 0) {
                fprintf(stderr, "\033[1;31m[-] shellforge: go: cannot access '%s': %s\033[0m\n", target, strerror(errno));
            }
        }
        return 1;
    }

    // 4. loc / pwd
    if (strcmp(args[0], "loc") == 0 || strcmp(args[0], "pwd") == 0) {
        builtin_loc();
        return 1;
    }

    // 5. scan
    if (strcmp(args[0], "scan") == 0) {
        builtin_scan(args[1]);
        return 1;
    }

    // 6. tree
    if (strcmp(args[0], "tree") == 0) {
        builtin_tree(args[1]);
        return 1;
    }

    // 7. lshidden / ls-hidden
    if (strcmp(args[0], "lshidden") == 0 || strcmp(args[0], "ls-hidden") == 0) {
        builtin_lshidden(args[1]);
        return 1;
    }

    // 7. peek
    if (strcmp(args[0], "peek") == 0) {
        int lines = (args[2] != NULL) ? atoi(args[2]) : 10;
        builtin_peek(args[1], lines);
        return 1;
    }

    // 8. sysinfo
    if (strcmp(args[0], "sysinfo") == 0) {
        builtin_sysinfo();
        return 1;
    }

    // 9. flush / clear / cls
    if (strcmp(args[0], "flush") == 0 || strcmp(args[0], "cls") == 0 || strcmp(args[0], "clear") == 0) {
        builtin_flush();
        return 1;
    }

    // 10. today
    if (strcmp(args[0], "today") == 0) {
        builtin_today();
        return 1;
    }

    // 11. now
    if (strcmp(args[0], "now") == 0) {
        builtin_now();
        return 1;
    }

    // 12. day [date]
    if (strcmp(args[0], "day") == 0) {
        builtin_day(args);
        return 1;
    }

    // 13. cal [month year]
    if (strcmp(args[0], "cal") == 0) {
        int m = (args[1] != NULL) ? atoi(args[1]) : 0;
        int y = (args[2] != NULL) ? atoi(args[2]) : 0;
        builtin_cal(m, y);
        return 1;
    }

    // 14. yr [year]
    if (strcmp(args[0], "yr") == 0) {
        int y = (args[1] != NULL) ? atoi(args[1]) : 0;
        builtin_yr(y);
        return 1;
    }

    // 15. Tasks (To-do checklist)
    if (strcmp(args[0], "Tasks") == 0) {
        builtin_tasks(args);
        return 1;
    }

    // 15. notes (Keep notes / nodes)
    if (strcmp(args[0], "notes") == 0) {
        builtin_notes(args);
        return 1;
    }

    // 16. history / hist
    if (strcmp(args[0], "history") == 0 || strcmp(args[0], "hist") == 0) {
        print_history();
        return 1;
    }

    // 17. jobs / tasks (lowercase: OS job control)
    if (strcmp(args[0], "jobs") == 0 || strcmp(args[0], "tasks") == 0) {
        do_jobs_builtin();
        return 1;
    }

    // 18. fg
    if (strcmp(args[0], "fg") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "\033[1;31m[-] Usage error: fg <job_id>\033[0m\n");
            return 1;
        }
        do_fg_builtin(atoi(args[1]));
        return 1;
    }

    // 19. bg
    if (strcmp(args[0], "bg") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "\033[1;31m[-] Usage error: bg <job_id>\033[0m\n");
            return 1;
        }
        do_bg_builtin(atoi(args[1]));
        return 1;
    }

    // 20. kill-job
    if (strcmp(args[0], "kill-job") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "\033[1;31m[-] Usage error: kill-job <job_id>\033[0m\n");
            return 1;
        }
        builtin_kill_job(atoi(args[1]));
        return 1;
    }

    return 0; // Not a built-in command
}

void resolve_aliases(pipeline_t *pipeline) {
    if (!pipeline) return;
    for (int i = 0; i < pipeline->command_count; i++) {
        command_t *cmd = &pipeline->commands[i];
        if (cmd->argc > 0 && cmd->argv[0] != NULL) {
            if (strcmp(cmd->argv[0], "show") == 0) {
                cmd->argv[0] = "cat";
            } else if (strcmp(cmd->argv[0], "hunt") == 0) {
                cmd->argv[0] = "find";
            } else if (strcmp(cmd->argv[0], "sig") == 0) {
                cmd->argv[0] = "grep";
            } else if (strcmp(cmd->argv[0], "init") == 0) {
                cmd->argv[0] = "touch";
            } else if (strcmp(cmd->argv[0], "fork") == 0) {
                cmd->argv[0] = "cp";
            } else if (strcmp(cmd->argv[0], "shift") == 0) {
                cmd->argv[0] = "mv";
            } else if (strcmp(cmd->argv[0], "del") == 0) {
                cmd->argv[0] = "rm";
            } else if (strcmp(cmd->argv[0], "detail") == 0) {
                // ls -la --color=auto [args...]
                cmd->argv[0] = "ls";
                if (cmd->argc < MAX_ARGS - 2) {
                    for (int j = cmd->argc; j >= 1; j--) {
                        cmd->argv[j + 2] = cmd->argv[j];
                    }
                    cmd->argv[1] = "-la";
                    cmd->argv[2] = "--color=auto";
                    cmd->argc += 2;
                    cmd->argv[cmd->argc] = NULL;
                }
            } else if (strcmp(cmd->argv[0], "recent") == 0) {
                // ls -lt [args...]
                cmd->argv[0] = "ls";
                if (cmd->argc < MAX_ARGS - 1) {
                    for (int j = cmd->argc; j >= 1; j--) {
                        cmd->argv[j + 1] = cmd->argv[j];
                    }
                    cmd->argv[1] = "-lt";
                    cmd->argc += 1;
                    cmd->argv[cmd->argc] = NULL;
                }
            } else if (strcmp(cmd->argv[0], "heavy") == 0) {
                // ls -lSh [args...]
                cmd->argv[0] = "ls";
                if (cmd->argc < MAX_ARGS - 1) {
                    for (int j = cmd->argc; j >= 1; j--) {
                        cmd->argv[j + 1] = cmd->argv[j];
                    }
                    cmd->argv[1] = "-lSh";
                    cmd->argc += 1;
                    cmd->argv[cmd->argc] = NULL;
                }
            }
        }
    }
}

int execute_builtin(command_t *cmd) {
    if (cmd == NULL || cmd->argc == 0) return 0;
    return handle_builtin_commands(cmd->argv);
}
