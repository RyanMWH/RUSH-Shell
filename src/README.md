# Rush Command Interpreter

This project is a simplified UNIX shell interpreter written in C, designed to execute basic shell commands with support for redirection, parallel execution, and path management. It is a part of a Unix system project created by Ryan Hanner, and it emulates some functionality of standard UNIX shells.

## Overview

Rush reads commands from standard input, tokenizes them, and interprets several basic commands (`exit`, `cd`, `path`). It also supports running programs from specified paths, redirection (`>`), and parallel execution (`&`). The program displays an error message for invalid commands or arguments.

## Features

- **Basic Shell Commands**:
  - **exit** - Exits the program.
  - **cd** - Changes the current directory.
  - **path** - Configures paths where Rush searches for executables.
- **Redirection**: Supports output redirection to files using `>`.
- **Parallel Execution**: Runs multiple commands in parallel using `&`.

## Installation

1. Clone the repository to your local machine.
2. Compile the code using `gcc`:
   ```bash
   gcc -o rush rush.c
