# ShellForge

```text
  ███████╗██╗  ██╗███████╗██╗     ██╗     ███████╗ ██████╗ ██████╗  ██████╗ ███████╗
  ██╔════╝██║  ██║██╔════╝██║     ██║     ██╔════╝██╔═══██╗██╔══██╗██╔════╝ ██╔════╝
  ███████╗███████║█████╗  ██║     ██║     █████╗  ██║   ██║██████╔╝██║  ███╗█████╗  
  ╚════██║██╔══██║██╔══╝  ██║     ██║     ██╔══╝  ██║   ██║██╔══██╗██║   ██║██╔══╝  
  ███████║██║  ██║███████╗███████╗███████╗██║     ╚██████╔╝██║  ██║╚██████╔╝███████╗
  ╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝╚═╝      ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚══════╝
```

> **A custom POSIX C11 command-line shell with pipelines, I/O redirections, process group job control, custom file utilities, productivity tools, and modern retro terminal styling.**

---

## Overview

**ShellForge** is an interactive Unix shell built in C11 for an Operating Systems and Systems Programming course. It implements a complete command execution pipeline:

1. **Lexical Analysis (Lexer)**: Tokenizes commands, strings, arguments, pipes, background operators, and redirections.
2. **Abstract Syntax Parsing (Parser)**: Builds multi-stage inter-process command pipelines (`pipeline_t`).
3. **Environment Variable Expansion**: Dynamically expands `$VAR` expressions (e.g. `$HOME`, `$USER`, `$PATH`).
4. **Custom Built-in & Alias Engine**: Fast internal handlers for shell navigation, directory inspection, calendar suite, and to-do/notes productivity modules.
5. **Process Group & Job Control**: Manages foreground and background process groups (`tcsetpgrp`, `setpgid`) with signal dispatch (`SIGINT`, `SIGTSTP`, `SIGCHLD`).
6. **GNU Readline Integration**: Provides persistent command history, arrow navigation, Tab completion, and terminal restoration upon exit.

---

## Key Features

### 1. Interactive Terminal Experience
- **Two-Tier Bracket Prompt**:
  ```text
  ╭─[shellforge]─(~/projects/shellforge)
  ╰─>>> 
  ```
- **Terminal Palette**: Electric cyan, pink, purple, and blue syntax themes with pure bold white for user typing (`\033[1;37m`).
- **Clean Signal Handling**: Pressing `Ctrl+C` or `Ctrl+Z` gracefully resets or exits the shell while restoring Linux terminal attributes (`rl_deprep_terminal()`, `tcsetattr()`, `tcsetpgrp()`) so parent shells never freeze.

### 2. POSIX Pipelines & Redirections
- **Pipelines**: Seamless multi-stage process execution with inter-process pipes:
  ```bash
  cat file.txt | grep "pattern" | wc -l
  ```
- **I/O Redirections**:
  - Overwrite output: `cmd > file`
  - Append output: `cmd >> file`
  - Input redirection: `cmd < file`
- **Asynchronous Execution**: Run background processes using `&` (e.g., `sleep 10 &`).

### 3. Comprehensive Command Catalog

#### Built-in Commands
| Command | Arguments | Description |
| :--- | :--- | :--- |
| `go` / `cd` | `[dir]` | Change working directory (supports `~`, `..`, absolute & relative paths) |
| `home` | — | Navigate directly to user's home directory (`cd ~`) |
| `back` | — | Navigate to parent directory (`cd ..`) |
| `loc` / `pwd` | — | Print absolute path of current working directory |
| `scan` | `[dir]` | Clean directory list showing permissions, sizes, and file types (omits dotfile clutter) |
| `detail` | `[dir]` | Full directory list with detailed permissions, links, and ownership (`ls -la`) |
| `recent` | `[dir]` | List directory contents sorted by newest modification time first (`ls -lt`) |
| `heavy` | `[dir]` | List directory contents sorted by file size (`ls -lSh`) |
| `lshidden` | `[dir]` | Exclusively inspect hidden files (`.gitignore`, `.git`, dotfiles, NTFS metadata) |
| `tree` | `[dir]` | Display recursive visual directory tree hierarchy |
| `peek` | `<file> [n]` | Preview the first `n` lines of a file (default: 10) |
| `flush` / `clear` | — | Clear terminal screen and reset prompt |
| `history` / `hist` | — | Display indexed command history backed by GNU Readline |
| `sysinfo` | — | Display host OS, kernel, machine architecture, and PID telemetry |
| `help` | — | Open categorized command manual |
| `bye` / `exit` | — | Safely shut down ShellForge session |

#### External Command Aliases
| Command | Mapping | Description |
| :--- | :--- | :--- |
| `show` | `cat` | Display file contents |
| `init` | `touch` | Create an empty file or touch timestamp |
| `fork` | `cp` | Copy files or directories |
| `shift` | `mv` | Move or rename files |
| `del` | `rm` | Delete files or directories |
| `hunt` | `find` | Search for files and directories matching patterns |
| `sig` | `grep` | Search text lines using regular expressions/patterns |

#### Date, Time & Calendar Suite
| Command | Arguments | Description |
| :--- | :--- | :--- |
| `today` | — | Print formatted current date (e.g., `Monday, October 12, 2026`) |
| `now` | — | Print current live time with timezone (e.g., `11:21:29 PM (IST)`) |
| `day` | `<date>` | Calculate day of week for dates (supports `2026 10 13`, `12 oct 26`, `2026-10-13`) |
| `cal` | `[month year]` | Render interactive colored monthly calendar grid (highlights current date) |
| `yr` | `[year]` | Render full 12-month calendar grid in a clean 3-column layout |

#### Productivity Suite (Tasks & Notes)
*Data is saved persistently to `~/.shellforge/`.*

- **To-Do Checklist (`Tasks`)**:
  - `Tasks`: View checklist with pending `[ ]` and completed `[X]` items.
  - `Tasks add <text>`: Add a new task to your checklist.
  - `Tasks done <id>`: Mark a task as completed `[X]`.
  - `Tasks undo <id>`: Revert task back to pending `[ ]`.
  - `Tasks del <id>`: Delete a task from your checklist.

- **Keep Notes / Cards (`notes`)**:
  - `notes`: View all saved note nodes/cards.
  - `notes add <title> : <body>`: Create a new note node with title and content.
  - `notes view <id>`: View the full content card of a note node.
  - `notes find <query>`: Case-insensitive search across all note titles and bodies.
  - `notes del <id>`: Delete a note node.

#### Job Control
| Command | Arguments | Description |
| :--- | :--- | :--- |
| `jobs` / `tasks` | — | List active background and suspended pipeline jobs |
| `fg` | `<job_id>` | Bring background or stopped job into foreground |
| `bg` | `<job_id>` | Resume a stopped job in the background |
| `kill-job` | `<job_id>` | Terminate a background job via `SIGTERM` |

---

## Project Structure

```text
shellforge/
├── include/              # Header declarations
│   ├── builtin.h         # Built-in handlers, banners & alias resolver
│   ├── executor.h        # Pipeline execution and fork/exec logic
│   ├── expand.h          # Environment variable expansion ($VAR)
│   ├── history.h         # GNU Readline history interface
│   ├── job_control.h     # Process group signals (SIGINT, SIGTSTP, SIGCHLD)
│   ├── jobs.h            # Job table structures and lifecycle tracking
│   ├── lexer.h           # Token stream generator
│   ├── parser.h          # Grammar parser & pipeline data structures
│   └── token.h           # Token definitions and types
├── src/                  # Implementation source files
│   ├── builtin.c         # Built-in commands, calendar suite & productivity tools
│   ├── executor.c        # Process pipelines, redirection & child wait
│   ├── expand.c          # Variable expansion implementation
│   ├── history.c         # History recording
│   ├── job_control.c     # Signal installation & terminal restoration
│   ├── jobs.c            # Job table management (fg, bg, kill)
│   ├── lexer.c           # Lexical tokenizer
│   ├── main.c            # Shell REPL, dynamic prompt & pipeline coordinator
│   ├── parser.c          # Pipeline parser
│   └── token.c           # Token list utilities
├── makefile              # Build configuration (gcc, c11, -lreadline)
└── README.md             # Project documentation
```

---

## Building and Running

### Prerequisites
- GCC / Clang with C11 support
- GNU Readline development library (`libreadline-dev`)
- Linux / WSL (Ubuntu)

Install prerequisites on Debian/Ubuntu:
```bash
sudo apt-get update
sudo apt-get install build-essential libreadline-dev
```

### Compilation
Build the binary with `make`:
```bash
make clean
make
```

### Launch
```bash
./shellforge
```

---

## License

Developed for educational and academic use in OS & Systems Programming coursework.
