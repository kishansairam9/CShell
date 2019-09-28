#include "pinfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

pid_t str_to_pid(char *str_pid) {
  pid_t pid = 0;
  for (int i = 0; i < strlen(str_pid); i++) {
    pid *= 10;
    pid += str_pid[i] - '0';
  }
  return pid;
}

int get_state(pid_t pid, char *store) {
  char buff[256];
  sprintf(buff, "/proc/%d/stat", pid);
  FILE *stat = fopen(buff, "r");
  if (stat == NULL) {
    return -1;
  }
  char *tp = (char *)(malloc(1000));
  for (int i = 1; i <= 23; i++) {
    fscanf(stat, "%s", tp);
    if (i == 3) {
      strcpy(store, tp);
    }
  }
  free(tp);
  fclose(stat);
  return 0;
}

void pid_info(pid_t pid) {
  char state[5];
  char vsize[100];
  char buff[256];
  sprintf(buff, "/proc/%d/stat", pid);
  FILE *stat = fopen(buff, "r");
  if (stat == NULL) {
    printf("Process not found\n");
    return;
  }
  char *tp = (char *)(malloc(1000));
  for (int i = 1; i <= 23; i++) {
    fscanf(stat, "%s", tp);
    if (i == 3) {
      strcpy(state, tp);
    }
    if (i == 23) {
      strcpy(vsize, tp);
    }
  }
  free(tp);
  fclose(stat);
  printf("pid -- %d\n", pid);              // pid
  printf("Process Status -- %s\n", state); // state
  printf("memory -- %s\n", vsize);         // vsize
  // Retrieve Executable
  sprintf(buff, "/proc/%d/exe", pid);
  char location[256];
  int ret = readlink(buff, location, 256);
  location[ret] = '\0';
  printf("Executable Path -- %s\n", location);
}

void pinfo(int argc, char *const *argv) {
  if (argc == 1)
    pid_info(getpid());
  else if (argc == 2)
    pid_info(str_to_pid(argv[1]));
  else
    fprintf(stderr, "Error, pinfo takes only one argument pid\n");
}
