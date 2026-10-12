# SimpleShell

A lightweight Unix shell implemented in C, supporting command execution, multi-stage pipelines, and command history.

## Overview

SimpleShell demonstrates Unix process management and inter-process communication using system calls. It executes external commands, connects pipeline stages using pipes, and tracks command execution statistics.

## Features

- **Command Execution:** Runs external programs using `fork()` and `execvp()`.
- **Pipeline Support:** Connects multiple commands using `pipe()` and `dup2()`.
- **Process Management:** Synchronizes child processes using `waitpid()`.
- **Command History:** Stores up to 100 commands with process IDs and execution timings.
- **Execution Statistics:** Measures command execution duration in milliseconds.
- **Signal Handling:** Displays command history on `Ctrl+C` and exits.

## Project Structure

```text
SimpleShell/
├── simple-shell.c
└── README.md
```

## Compilation and Usage

Compile using GCC:

```bash
gcc -Wall -Wextra -o shell simple-shell.c
```

Run the shell:

```bash
./shell
```

Example commands:

```bash
ls -l
cat file.c | grep "main" | wc -l
history
```

Press `Ctrl+C` to display the command history and exit.

## Implementation Details

- Parses commands and separates pipeline stages using `|`.
- Creates child processes for pipeline commands and redirects input/output through pipes.
- Records command execution times using `gettimeofday()`.
- Maintains a fixed-size history of executed commands.

## Concepts Demonstrated

Unix system calls, process creation and synchronization, pipes, file descriptor redirection, signal handling, and execution-time measurement.
