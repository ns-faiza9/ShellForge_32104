#include <stdio.h>
#include <readline/history.h>
#include "../include/history.h"

void print_history(void)
{
    HIST_ENTRY **list = history_list();

    if (list == NULL)
    {
        printf("\033[38;5;213mHistory is currently empty.\033[0m\n");
        return;
    }

    printf("\n\033[38;5;205m╔════════════════════ SHELLFORGE COMMAND HISTORY ════════════════════╗\033[0m\n");
    for (int i = 0; list[i] != NULL; i++)
    {
        printf("  \033[38;5;141m[%3d]\033[0m  \033[1;37m%s\033[0m\n", i + history_base, list[i]->line);
    }
    printf("\033[38;5;205m╚════════════════════════════════════════════════════════════════════╝\033[0m\n\n");
}
