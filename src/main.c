#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/expand.h"
#include "../include/builtin.h"
#include "../include/jobs.h"
#include "../include/job_control.h"
#include "../include/executor.h"

static void build_prompt(char *buf, size_t size) {
    char cwd[512];
    if (!getcwd(cwd, sizeof(cwd))) {
        strcpy(cwd, "?");
    }
    char *home = getenv("HOME");
    char display_path[512];
    if (home && strncmp(cwd, home, strlen(home)) == 0) {
        snprintf(display_path, sizeof(display_path), "~%s", cwd + strlen(home));
    } else {
        strncpy(display_path, cwd, sizeof(display_path) - 1);
        display_path[sizeof(display_path) - 1] = '\0';
    }

    // Classic terminal bracket prompt in Pink, Purple, and Electric Blue
    // Ends with \001\033[1;37m\002 ensuring user's typed command is always pure bold white
    snprintf(buf, size,
             "\001\033[38;5;205m\002╭─[\001\033[38;5;141m\002shellforge\001\033[38;5;205m\002]─(\001\033[38;5;39m\002%s\001\033[38;5;205m\002)\n"
             "\001\033[38;5;205m\002╰─\001\033[38;5;213m\002>\001\033[38;5;141m\002>\001\033[38;5;51m\002> \001\033[1;37m\002",
             display_path);
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    // Display ASCII ShellForge Banner & Context
    print_banner();

    // Initialize job table and signals (sets shell as process group leader)
    jobs_init();
    init_job_signals();

    char prompt_buf[1024];

    while (1) {
        // Poll and notify about any finished background processes
        check_background_jobs();

        // Ensure prompt signal handlers are active
        install_prompt_signals();

        // Dynamically build colored prompt with current working directory
        build_prompt(prompt_buf, sizeof(prompt_buf));

        errno = 0;
        char *line = readline(prompt_buf);

        // Reset terminal attributes after user submits command line
        printf("\033[0m");

        if (!line) {
            // EOF encountered (Ctrl+D) or signal exit
            printf("\n\033[38;5;205mExiting ShellForge. Goodbye!\033[0m\n");
            break;
        }

        // Skip blank lines
        if (line[0] == '\0') {
            free(line);
            continue;
        }

        // Add non-empty command to history
        add_history(line);

        // 1. Lexical Analysis
        token_list_t tokens;
        lexer(line, &tokens);

        // 2. Syntax Parsing into Pipeline
        pipeline_t pipeline;
        if (!parse(&tokens, &pipeline)) {
            free(line);
            continue;
        }

        // 3. Environment Variable Expansion ($VAR)
        expand_variables(&pipeline);

        if (pipeline.command_count == 0 || pipeline.commands[0].argc == 0) {
            free(line);
            continue;
        }

        // 4. Built-in command execution in the shell parent process
        if (pipeline.command_count == 1 && is_builtin(&pipeline.commands[0])) {
            execute_builtin(&pipeline.commands[0]);
            free(line);
            continue;
        }

        // 5. Custom Command Alias Resolution (show -> cat, fork -> cp, hunt -> find...)
        resolve_aliases(&pipeline);

        // 6. External Command & Pipeline Execution
        ShellCommand shell_cmds[MAX_COMMANDS];
        for (int i = 0; i < pipeline.command_count; i++) {
            shell_cmds[i].args = pipeline.commands[i].argv;
            shell_cmds[i].input_file = (pipeline.commands[i].input[0] != '\0') 
                                        ? pipeline.commands[i].input : NULL;
            shell_cmds[i].output_file = (pipeline.commands[i].output[0] != '\0') 
                                         ? pipeline.commands[i].output : NULL;
            shell_cmds[i].append = pipeline.commands[i].append;
        }

        int is_bg = 0;
        for (int i = 0; i < pipeline.command_count; i++) {
            if (pipeline.commands[i].background) {
                is_bg = 1;
                break;
            }
        }

        execute_pipeline(shell_cmds, pipeline.command_count, is_bg, line);
        free(line);
    }

    return 0;
}
