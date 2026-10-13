#ifndef EXECUTOR_H
#define EXECUTOR_H

// Simple wrapper structure matching the architecture layout
typedef struct {
    char **args;
    char *input_file;
    char *output_file;
    int append;
} ShellCommand;

// The official, unified signature for the execution module
int execute_pipeline(ShellCommand *cmds, int cmd_count, int is_background, char *full_cmd_str);

#endif // EXECUTOR_H
