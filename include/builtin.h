#ifndef BUILTIN_H
#define BUILTIN_H

#include "parser.h"

/* Display startup ASCII banner and context */
void print_banner(void);

/* Check whether a command is a built-in */
int is_builtin(const command_t *cmd);

/* Resolve custom ShellForge command aliases (show -> cat, fork -> cp, etc.) */
void resolve_aliases(pipeline_t *pipeline);

/* Execute a built-in command from a command_t */
int execute_builtin(command_t *cmd);

/* Handle built-in commands by raw argument vector */
int handle_builtin_commands(char **args);

#endif
