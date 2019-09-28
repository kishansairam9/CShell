#include "cronjob.h"
#include "run_command.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cronjob(const int cron_argc, char *const *cron_argv,
             const int cron_max_size_arg) {
  // TO BE RUN IN FORKED PROCESS
  if (cron_argc < 7) {
    fprintf(stderr, "Incorrect usage\n");
    return;
  }
  char **copy_cron_argv = (char **)(malloc(sizeof(char *) * (cron_argc + 1)));
  for (int i = 0; i < cron_argc; i++) {
    copy_cron_argv[i] = (char *)(malloc(cron_max_size_arg));
    strcpy(copy_cron_argv[i], cron_argv[i]);
  }
  opterr = 0;
  int opt_id = 0, every = 0, total = 0;
  char *command = (char *)(malloc(1024));
  while ((opt_id = getopt(cron_argc, cron_argv, "c:t:p:")) != -1) {
    switch (opt_id) {
    case 'c':
      strcpy(command, optarg);
      break;
    case 't':
      every = atoi(optarg);
      break;
    case 'p':
      total = atoi(optarg);
      break;
    default:
      continue;
    }
  }
  optind = 0;

  int got_c = 0;
  for (int i = 1; i < cron_argc; i++) {
    if (got_c && copy_cron_argv[i] != NULL &&
        (strcmp(copy_cron_argv[i], "-t") == 0 ||
         strcmp(copy_cron_argv[i], "-p") == 0)) {
      break;
    }
    if (got_c == 0 && strcmp(command, copy_cron_argv[i]) == 0) {
      got_c = 1;
      continue;
    }
    if (got_c) {
      strcat(command, " ");
      strcat(command, copy_cron_argv[i]);
    }
  }

  if (every == 0 || total == 0) {
    free(command);
    fprintf(stderr, "Incorrect usage\n");
    return;
  }
  while (command[0] == ' ' || command[0] == '\t')
    command++;
  int argc = 0, max_size_arg = 0;
  for (int t = 0; t < strlen(command); t++) {
    int size = 0;
    while (t < strlen(command) && !(command[t] == ' ' || command[t] == '\t')) {
      size += 1;
      t++;
    }
    if (size > max_size_arg)
      max_size_arg = size;
    if (size)
      argc++;
  }
  max_size_arg++;
  char **argv = (char **)(malloc(sizeof(char *) * (argc + 1)));
  for (int i = 0; i <= argc; i++) {
    argv[i] = (char *)(malloc(max_size_arg));
  }
  char *pa_token;
  char *part_arg = strtok_r(command, " \t", &pa_token);
  int i = 0;
  while (part_arg != NULL) {
    strcpy(argv[i++], part_arg);
    part_arg = strtok_r(NULL, " \t", &pa_token);
  }
  int times = total / every;
  for (int i = 0; i < times; i++) {
    sleep(every);
    run_cmd(0, argc, argv, max_size_arg, NULL);
  }
  for (int i = 0; i <= argc; i++) {
    free(argv[i]);
  }
  free(argv);
  free(command);
}