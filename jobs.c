#include "jobs.h"
#include "pinfo.h"
#include <fcntl.h>
#include <signal.h>
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

void jobs(int argc, char *const *argv) {
  if (argc != 1) {
    fprintf(stderr, "Incorrect no of arguments\n");
    return;
  }
  for (int i = 1; i < next_job_no; i++) {
    if (job_store[i].exited == 0) {
      if (job_store[i].running)
        printf("[%d] Running %s [%d]\n", i, job_store[i].command,
               job_store[i].pid);
      else
        printf("[%d] Stopped %s [%d]\n", i, job_store[i].command,
               job_store[i].pid);
    }
  }
}

void overkill(int argc, char *const *argv) {
  if (argc != 1) {
    fprintf(stderr, "Incorrect no of arguments\n");
    return;
  }
  for (int i = 1; i < next_job_no; i++) {
    if (job_store[i].exited == 0) {
      kill(job_store[i].pid, SIGKILL);
      job_store[i].exited = 1;
    }
  }
}

void kjob(int argc, char *const *argv) {
  if (argc != 3 || atoi(argv[1]) >= next_job_no) {
    fprintf(stderr, "Invalid arguments\n");
    return;
  }
  int pos_in_arr = atoi(argv[1]);
  kill(job_store[pos_in_arr].pid, atoi(argv[2]));
  char state[5];
  sleep(1);
  if (get_state(job_store[pos_in_arr].pid, state) != -1) {
    if (strcmp(state, "T") == 0) {
      // Process got suspended
      job_store[pos_in_arr].running = 0;
    } else if (strcmp(state, "Z") == 0) {
      // Process got terminated
      job_store[pos_in_arr].exited = 1;
    } else {
      // Process is running
      job_store[pos_in_arr].running = 1;
    }
  } else {
    job_store[pos_in_arr].exited = 1;
  }
}

void fg(int argc, char *const *argv) {
  if (argc != 2 || atoi(argv[1]) >= next_job_no) {
    fprintf(stderr, "Invalid arguments\n");
    return;
  }
  int pos_in_arr = atoi(argv[1]);
  if (job_store[pos_in_arr].exited == 1) {
    fprintf(stderr, "Job Exited, can't do fg on it\n");
    return;
  }
  int jobpid = job_store[pos_in_arr].pid;
  kill(jobpid, SIGCONT);
  char process_stdin[128];
  sprintf(process_stdin, "/proc/%d/fd/0", jobpid);
  int str = dup(0);
  int new_stdin = open(process_stdin, O_APPEND);
  dup2(new_stdin, 0);
  char state[5];
  curr_fg = job_store[pos_in_arr].pid;
  sleep(1);
  int bysuspend = 0;
  while (1) {
    // Waiting for process to get terminated / suspended
    if (strcmp(state, "Z") == 0)
      break;
    if (strcmp(state, "T") == 0) {
      bysuspend = 1;
      break;
    }
    if (get_state(job_store[pos_in_arr].pid, state) == -1)
      break;
  }
  dup2(str, 0);
  close(str);
  close(new_stdin);
  if (bysuspend == 0)
    job_store[pos_in_arr].exited = 1;
  else
    job_store[pos_in_arr].running = 0;
  curr_fg = -1;
}

void bg(int argc, char *const *argv) {
  if (argc != 2 || atoi(argv[1]) >= next_job_no) {
    fprintf(stderr, "Invalid arguments\n");
    return;
  }
  int pos_in_arr = atoi(argv[1]);
  kill(job_store[pos_in_arr].pid, SIGCONT);
  job_store[pos_in_arr].running = 1;
}
