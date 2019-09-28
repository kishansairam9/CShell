#include <signal.h>
#include <stdio.h>
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

void ctrlD(int sig) {
  return; // Ignore ctrl + D
}

void ctrlC(int sig) {
  if (curr_fg != -1)
    kill(curr_fg, SIGINT);
}

void ctrlZ(int sig) {
  if (curr_fg != -1) {
    kill(curr_fg, SIGTSTP);
    printf(" Current process with pid %d added to jobs\n", curr_fg);
  }
  // TODO: Add push to background
}

void sigchld_handler(int sig) {
  int status;
  pid_t pid = waitpid(-1, &status, WNOHANG);
  if (pid == -1)
    return;
  for (int i = 1; i < next_job_no; i++) {
    if (job_store[i].pid == pid) {
      job_store[i].exited = 1;
      printf("\n%s with pid %d exited\n", job_store[i].command, pid);
      if (WIFEXITED(status)) {
        printf("normally with exit status %d\n", WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        printf("with signal %d\n", WTERMSIG(status));
      }
    }
  }
}