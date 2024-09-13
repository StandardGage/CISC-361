# CISC 361 Project 1

## Description
This project creates a doubly linked list of student structs that can then be interacted with

## Installation and running
Clone the repository and run `make` or `make slist`
- If receiving a bash error, ensure that gcc and make are installed
- Ensure that valgrind is installed if running `make valgrind` to check for memory leaks
- Remove the executable with `make clean`


## Usage
Users have 4 main options
1. Add a student
2. Delete all students by last name
3. Print all students
4. Print all students in reverse

Users can also exit the program

## Student data
Each student consists of 5 attributes
- last name
- first name
- id number
- year
- graduation year