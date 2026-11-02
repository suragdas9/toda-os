/**
 * Toda OS — Enhanced Interactive Shell Header
 *
 * Rich terminal interface with ANSI color support,
 * diagnostic utilities, and process inspection tools.
 *
 * Copyright (c) Toda OS Project
 */

#ifndef TODA_SHELL_H
#define TODA_SHELL_H

#include "kernel.h"

/* ANSI Color Escape Sequences */
#define ANSI_RESET          "\033[0m"
#define ANSI_BOLD           "\033[1m"
#define ANSI_RED            "\033[1;31m"    /* Errors / Alerts */
#define ANSI_GREEN          "\033[1;32m"    /* Success / OK */
#define ANSI_YELLOW         "\033[1;33m"    /* Values / Highlights */
#define ANSI_BLUE           "\033[1;34m"
#define ANSI_MAGENTA        "\033[1;35m"
#define ANSI_CYAN           "\033[1;36m"    /* Headers / Titles */
#define ANSI_WHITE          "\033[1;37m"

/* Shell Entry Point */
void shell_run(void);

#endif /* TODA_SHELL_H */
