#include "echo.h"
#include "jobs.h"
#include "ls.h"
#include "signal_handlers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct job {
  int pid;
  char *command;
  int running;
  int exited;
};

extern int curr_fg;
extern int next_job_no;
extern struct job job_store[1024];
extern char currDir[256];
extern char homeDir[256];
extern char whole_command[1024];

int run_cmd(int start, int end, char **original_argv, int max_size_arg,
            int *pid_fg) {
  // Returns
  // 0 -> All OK
  // 1 -> Error occured while forking
  // -1 -> Quit shell
  int argc = end - start;
  char **argv = original_argv + start;
  if (!strcmp("ls", argv[0])) {
    ls(argc, argv);
  } else if (!strcmp("pwd", argv[0])) {
    getcwd(currDir, 256);
    printf("%s\n", currDir);
  } else if (!(strcmp("echo", argv[0]))) {
    echo(argc, argv, max_size_arg);
  } else if (!(strcmp("quit", argv[0]))) {
    return -1;
  } else if (!(strcmp("jobs", argv[0]))) {
    jobs(argc, argv);
  } else if (strcmp("", argv[0]) && argv[0] != NULL) {
    // Background processes are handled in main.c, Here only foreground are
    // dealt
    pid_t fk = fork();
    if (fk == -1) {
      perror("Error occured while creating process");
      return 1;
    }
    argv[argc] = NULL;
    if (fk == 0) {
      int rk = execvp(argv[0], argv);
      if (rk < 0) {
        fprintf(stderr, "Command \"%s\" not found\n", argv[0]);
        exit(1);
      }
    } else {
      if (pid_fg != NULL)
        *pid_fg = fk;
      curr_fg = fk;
      int status;
      waitpid(fk, &status, WUNTRACED);
      curr_fg = -1;
    }
  }
  return 0;
}