# CISC 361 Project 2

## Description
This project consists of a custom shell that employs many of the features of a regular unix shell

## Installation and running
Clone the repository and run `make` or `make dest`
- If receiving a bash error, ensure that gcc and make are installed
- If receiving an error for readline, ensure that readline-dev is installed
- Ensure that valgrind is installed if running `make dest` to check for memory leaks
- Can also build and run with valgrind using `make valgrind`
- Remove the executable with `make clean`


## Usage

- The shell comes with multiple builtin commands
1. exit
2. which
3. list
4. pwd
5. cd
6. pid
7. prompt
8. printenv
9. setenv

- The shell can also run any commands or executables that are already on your machine and will even make use of your system's PATH
- The shell also includes *autocomplete* for commands and files

