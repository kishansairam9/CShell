# C Shell

> Developed on git repository [here](https://github.com/kishansairam9/Operating-Systems-Assignments) under folders Assignment2 & Assignment3. Refer for commit history

- Computer Systems Engineering - 1 (Assignment 2 & 3)
- By Kishan Sairam Adapa - 2018101026

## Features

- All commands present in system's path work. This is done by using `execvp`.
- Directory from which shell is invoked is made home directory i.e, `~`.
  - Home directory can be set to directory in which the executable is present.
  - This is done by doing these in `main.c`
    - Uncommenting line `42` to `50`
    - Commenting line `53`
- Some commands are implemented on own, they are
  - Built ins
    - cd
    - ls
    - pwd
    - echo
  - Process related
    - pinfo
  - Job related
    - jobs
    - kjob
    - overkill
- Processes are made background when `&` is used
- Pipes and Redirection are handled

## Running Shell

- Run `make` in directory in which codes are present to generate executable `main.out`
- Run `./main.out` to invoke shell
- **Dependency:** This shell implemented uses `readline` library which is not part of standard. Which needs to be installed in case `make` fails

## Description of Files

### Compiling
- Makefile
  
### Header files
  - cd.h
    - cd
  - cronjob.h
    - cronjob
  - echo.h
    - echo
  - itoa.h
    - b10itoa 
      - Conversion of base 10 integers to strings
      - Used to convert integers into strings for passing through pipes
  - jobs.h
    - jobs
    - kjob
    - fg
    - bg
  - ls.h
    - ls
  - pinfo.h
    - pinfo
    - get_state
      - Stores current state of process in passed string given its pid
      - Used for updating data structure in which jobs are stored
  - prompt.h
    - storePromptString
      - Stores what prompt to print in a string which is passed as parameter
      - Used to implement prompt using readline library
  - run_command.h
    - run_cmd
      - Given correct parameters it runs the command passed through parameters in foreground
  - signal_handlers.h
    - ctrlD
      - Handler for SIGQUIT (Ctrl + D)
    - ctrlC
      - Handler for SIGINT (Ctrl + C)
    - ctrlZ
      - Hander for SIGTSTP (Ctrl + Z)
    - sigchld_handler
      - Handler for SIGCHLD (Triggered whhen a child exits)

### Implementation files (Contain Implemntation of functions in respective header files)
  - cd.c
  - cronjob.c
  - echo.c
  - itoa.c
  - jobs.c
  - ls.c
  - pinfo.c
  - prompt.c
  - run_command.c
  - signal_handlers.c

### Shell program file - main.c
  - Implements C Shell by using system headers and headers mentioned here
