# Programs-in-C

A collection of four console applications written in C. Each application demonstrates different aspects of programming,
from working with files and encryption to emulating a banking system and a file manager.

## Repository Content

The repository contains four independent projects:

- ATM_Simulator - Simulating ATM operation
- Shif-Def - File Encryption/Decryption Software 
- Task Manager - Stateful Task Manager
- VFC_system - Virtual File system

## Start

1. Clone the repository:

git clone https://github.com/SergeyKisa228/Programs-in-C.git

cd Programs-in-C

2. Select a project to run and compile it:

# ATM_Simulator

cd ATM_Simulator

gcc -o atm main.c

./atm

# Shif-Def

cd ../Shif-Def

gcc -o shifdef main.c

./shifdef

# Task Manager

cd ../Task_Manager

gcc -o taskmanager main.c

./taskmanager

# VFC_system

cd ../VFC_system

gcc -o vfc main.c

./vfc

## Description of projects

## 1. ATM_Simulator

Implementation of an ATM system with user authentication and balance operations.

Features:

--- Registration and authorization by login and PIN code (4 digits)

--- Password hashing using the djb2 algorithm

--- Operations: checking balance, depositing funds, and withdrawing cash

--- Transaction history with timestamp

--- Data is stored in text files (user_pass.txt, [username]_balance.txt, [username]_history.txt)

## 2. Shif-Def

A utility for encrypting and decrypting files using XOR encryption.

Features:

--- Two modes of operation: interactive and command-line

--- Creates a projects folder for storing files

--- Checks the correctness of the key during decryption

--- Cross-platform (Windows/Linux)

--- File signature for identifying encrypted files

Using it via the command line:

# Encryption
./shifdef -e mysecretkey file.txt

# Decryption
./shifdef -d mysecretkey file.txt.enc

## 3. Task Manager

Console task manager that saves the state to a file.

Features:

--- Add, view, complete, and delete tasks

--- Autosave on exit

--- Load previous state on startup

--- Input validation (check for empty tasks)

--- Data is saved in the tasks.dat file

## 4. VFC_system

A virtual file system that supports files and directories.

Features:

--- Creating files and folders

--- Navigating through directories (cd, ls)

--- Writing data to files

--- Saving the state between runs

--- Implementing inodes and block systems

--- Saving the state in the vfs_save.bin file

## Requirements and compatibility

--- C compiler (GCC, Clang, or similar)

--- C standard library

--- Compatible with Windows, Linux, and macOS

## Implementation features

All programs use only the standard C library and do not require additional dependencies. 
The code is written with a focus on portability and clarity. Each project is self-contained and can be studied separately.

## Author

SergeyKisa228 - development of all projects
