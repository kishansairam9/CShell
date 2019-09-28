#include "cd.h"
#include "cronjob.h"
#include "itoa.h"
#include "jobs.h"
#include "pinfo.h"
#include "prompt.h"
#include "run_command.h"
#include "signal_handlers.h"
#include <errno.h>
#include <fcntl.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

char currDir[256];
char homeDir[256];
int curr_fg = -1;

struct job {
  int pid;
  char *command;
  int running;
  int exited;
};

int next_job_no = 1;
struct job job_store[1024];

int main() {
  // Activate signal handlers
  signal(SIGINT, ctrlC);
  signal(SIGTSTP, ctrlZ);
  signal(SIGQUIT, ctrlD);
  signal(SIGCHLD, sigchld_handler);
  // -> For home to be directory in which executable (as mentioned in
  // moodle) is present uncomment following lines
  // char exe_path[128];
  // sprintf(exe_path,"/proc/%d/exe", getpid()); int tp_ret =
  // readlink(exe_path, currDir, 256); currDir[tp_ret] = '\0'; for (int i =
  // strlen(currDir) - 1; i >= 0; i--) {
  //   if ((i > 0) && (currDir[i] == '/' && currDir[i - 1] != '\\')) {
  //     currDir[i] = '\0';
  //     break;
  //   }
  // }
  // -> Path is set to Directory from which executable is invoked (as
  // mentioned in pdf)
  getcwd(currDir, 256);
  strcpy(homeDir, currDir);
  using_history();
  while (1) {
    char pmt_str[128];
    storePromptString(pmt_str);
    char *whole_command = NULL;
    whole_command = readline(pmt_str);
    add_history(whole_command);
    char *whc_token;
    char *with_pipes_command = strtok_r(whole_command, ";", &whc_token);
    while (with_pipes_command != NULL) {
      int has_pipes = 0;
      for (int i = 0; i < strlen(with_pipes_command); i++) {
        if (with_pipes_command[i] == '|') {
          has_pipes = 1;
          break;
        }
      }
      char *pipe_token;
      char *command = strtok_r(with_pipes_command, "|", &pipe_token);
      int next_in = 0;
      int sq_pipe[2];
      while (command != NULL) {
        if (has_pipes) {
          if (next_in != 0)
            close(sq_pipe[1]);
          pipe(sq_pipe);
        }
        while (command[0] == ' ' || command[0] == '\t')
          command++;
        int argc = 0, max_size_arg = 0;
        for (int t = 0; t < strlen(command); t++) {
          int size = 0;
          while (t < strlen(command) &&
                 !(command[t] == ' ' || command[t] == '\t')) {
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
        int err_fork = 0, end = 0, start = 0;
        for (int i = 0; i < argc; i++) {
          int ret_rcmd = -9; // Flag set as -9 to test if child created
          int stat_ret_rcmd_pipe[2];
          pipe(stat_ret_rcmd_pipe);
          // When argv[i] is one of ">", ">>", "<", "|"
          if (!strcmp(">", argv[i])) {
            if (i == argc - 1 || i == 0) {
              // When command ends / starts with these special strings
              fprintf(stderr, "Invalid command\n");
              break;
            }
            end = i;
            if (strcmp(">", argv[i + 1]) == 0 ||
                strcmp(">>", argv[i + 1]) == 0 ||
                strcmp("<", argv[i + 1]) == 0) {
              // If file to be written to is a special string
              fprintf(stderr, "Invalid command\n");
              break;
            }
            int fk = fork();
            if (fk == -1) {
              perror("Error while forking");
              err_fork = 1;
              break;
            }
            if (fk == 0) {
              if (next_in != 0) {
                // Replace stdin with next_in
                dup2(next_in, 0);
                close(next_in);
              }
              // Replace stdout with file to write
              int fd = open(argv[i + 1], O_CREAT | O_WRONLY | O_TRUNC,
                            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
              if (fd < 0) {
                perror("Error while creating file");
                exit(1);
              }
              dup2(fd, 1);
              char *tmp = (char *)(malloc(1));
              *tmp = run_cmd(start, end, argv, max_size_arg, NULL) + '0';
              close(stat_ret_rcmd_pipe[0]);
              write(stat_ret_rcmd_pipe[1], tmp, 1);
              exit(0);
            }
            waitpid(fk, NULL, 0);
            start = end + 2; // This takes care of invalid commands in next iter
            i++;
          } else if (!strcmp(">>", argv[i])) {
            if (i == argc - 1 || i == 0) {
              // When command ends / starts with these special strings
              fprintf(stderr, "Invalid command\n");
              break;
            }
            end = i;
            if (strcmp(">", argv[i + 1]) == 0 ||
                strcmp(">>", argv[i + 1]) == 0 ||
                strcmp("<", argv[i + 1]) == 0) {
              // If file to be written to is a special string
              fprintf(stderr, "Invalid command\n");
              break;
            }
            int fk = fork();
            if (fk == -1) {
              perror("Error while forking");
              err_fork = 1;
              break;
            }
            if (fk == 0) {
              if (next_in != 0) {
                // Replace stdin with next_in
                dup2(next_in, 0);
                close(next_in);
              }
              // Replace stdout with file to write
              int fd = open(argv[i + 1], O_CREAT | O_APPEND | O_WRONLY,
                            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
              if (fd < 0) {
                perror("Error while appending to file");
                exit(1);
              }
              dup2(fd, 1);
              char *tmp = (char *)(malloc(1));
              *tmp = run_cmd(start, end, argv, max_size_arg, NULL) + '0';
              close(stat_ret_rcmd_pipe[0]);
              write(stat_ret_rcmd_pipe[1], tmp, 1);
              exit(0);
            }
            waitpid(fk, NULL, 0);
            start = end + 2; // This takes care of invalid commands in next iter
            i++;
          } else if (!strcmp("<", argv[i])) {
            int send_out_to_file = 0;
            if (i == argc - 1 || i == 0) {
              // When command ends / starts with these special strings
              fprintf(stderr, "Invalid command\n");
              break;
            }
            end = i;
            if (end + 3 < argc && (strcmp(argv[end + 2], ">") == 0 ||
                                   strcmp(argv[end + 2], ">>") == 0)) {
              send_out_to_file = 1;
            }
            if (strcmp(">", argv[i + 1]) == 0 ||
                strcmp(">>", argv[i + 1]) == 0 ||
                strcmp("<", argv[i + 1]) == 0) {
              // If file to be written to is a special string
              fprintf(stderr, "Invalid command\n");
              break;
            }
            int fk = fork();
            if (fk == -1) {
              perror("Error while forking");
              err_fork = 1;
              break;
            }
            if (fk == 0) {
              int out_fd;
              if (send_out_to_file) {
                // Send to file instead of stdout when having > / >> after <
                if (strcmp(argv[end + 2], ">>") == 0) {
                  out_fd = open(argv[end + 3], O_CREAT | O_APPEND | O_WRONLY,
                                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                } else {
                  out_fd = open(argv[end + 3], O_CREAT | O_WRONLY | O_TRUNC,
                                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                }
                if (out_fd < 0) {
                  perror("Error while appending to file");
                  exit(1);
                }
                // Replace stdout with out_fd
                dup2(out_fd, 1);
                close(out_fd);
              }
              // Replace stdin with file to readfrom
              int fd = open(argv[i + 1], O_RDONLY);
              if (fd < 0) {
                perror("Error while reading file");
                exit(1);
              }
              dup2(fd, 0);
              close(fd);
              if (has_pipes) {
                // Replace stdout with sq_pipe[1]
                if (send_out_to_file == 0)
                  dup2(sq_pipe[1], 1);
                close(sq_pipe[1]);
              }
              char *tmp = (char *)(malloc(1));
              *tmp = run_cmd(start, end, argv, max_size_arg, NULL) + '0';
              close(stat_ret_rcmd_pipe[0]);
              write(stat_ret_rcmd_pipe[1], tmp, 1);
              exit(0);
            }
            waitpid(fk, NULL, 0);
            start = end + 2; // This takes care of invalid commands in next iter
            if (send_out_to_file) {
              start = end + 4;
              i += 2;
            }
            i++;
          } else if (i == argc - 1) {
            end = argc;
            if (strcmp(argv[start], "cd") == 0) {
              cd(end - start, (argv + start));
            } else if (!(strcmp("pinfo", argv[start]))) {
              pinfo(end - start, (argv + start));
            } else if (!(strcmp("setenv", argv[start]))) {
              if (end - start == 2) {
                if (setenv(argv[start + 1], "", 1) == -1) {
                  perror("Error occured while setting environment variable");
                }
              } else if (end - start == 3) {
                if (setenv(argv[start + 1], argv[start + 2], 1) == -1) {
                  perror("Error occured while setting environment variable");
                }
              } else {
                fprintf(stderr,
                        "Error, setenv takes either one or two arguments\n");
              }
            } else if (!(strcmp("unsetenv", argv[start]))) {
              if (end - start == 2) {
                if (unsetenv(argv[start + 1]) == -1) {
                  perror("Error occurred while unsetting environment variable");
                }
              } else {
                fprintf(stderr, "Error, unsetenv takes only one argument\n");
              }
            } else if (!(strcmp("kjob", argv[start]))) {
              kjob(end - start, argv);
            } else if (!(strcmp("bg", argv[start]))) {
              bg(end - start, argv);
            } else if (!(strcmp("fg", argv[start]))) {
              fg(end - start, (argv + start));
            } else if (!(strcmp("overkill", argv[start]))) {
              overkill(end - start, (argv + start));
            } else if (!(strcmp("cronjob", argv[start]))) {
              int fk = fork();
              if (fk == 0) {
                close(sq_pipe[1]);
                close(sq_pipe[0]);
                cronjob(end - start, (argv + start), max_size_arg);
                exit(0);
              }
              ret_rcmd = 0;
            } else {
              char **temp_argv = argv + start;
              int temp_argc = end - start;
              if (!strcmp("&", temp_argv[temp_argc - 1])) {
                // Handle background processes here
                char *temporary_store = temp_argv[argc - 1];
                temp_argv[argc - 1] = NULL;
                job_store[next_job_no].command =
                    (char *)(malloc(sizeof(temp_argv[0])));
                strcpy(job_store[next_job_no].command, temp_argv[0]);
                job_store[next_job_no].running = 1;
                job_store[next_job_no].exited = 0;
                int fk = fork();
                if (fk == -1) {
                  perror("Error while forking");
                  err_fork = 1;
                  break;
                }
                if (fk == 0) {
                  if (has_pipes)
                    close(sq_pipe[1]);
                  int rk = execvp(temp_argv[0], temp_argv);
                  if (rk < 0) {
                    fprintf(stderr, "Command \"%s\" not found\n", temp_argv[0]);
                    exit(1);
                  }
                }
                setpgid(fk, fk);
                job_store[next_job_no].pid = fk;
                next_job_no++;
                temp_argv[argc - 1] = temporary_store;
                ret_rcmd = 0;
              } else {
                int pid_pipe[2];
                pipe(pid_pipe);
                int fk = fork();
                if (fk == -1) {
                  perror("Error while forking");
                  err_fork = 1;
                  break;
                }
                if (fk == 0) {
                  int pid_fg = 0; // If inbuilt command then give pid as 0
                  // Replace stdin with next_in
                  if (next_in != 0) {
                    dup2(next_in, 0);
                    close(next_in);
                  }
                  if (has_pipes) {
                    // Replace stdout with sq_pipe[1]
                    dup2(sq_pipe[1], 1);
                    close(sq_pipe[1]);
                  }
                  char *tmp = (char *)(malloc(1));
                  *tmp = run_cmd(start, end, argv, max_size_arg, &pid_fg) + '0';
                  close(stat_ret_rcmd_pipe[0]);
                  write(stat_ret_rcmd_pipe[1], tmp, 1);
                  // Write pid_fg to pid_pipe
                  close(pid_pipe[0]);
                  char str_pid[100];
                  b10itoa(pid_fg, str_pid);
                  write(pid_pipe[1], str_pid, 100);
                  exit(0);
                }
                waitpid(fk, NULL, 0);
                int fg_pid;
                char str_pid[100];
                close(pid_pipe[1]);
                read(pid_pipe[0], str_pid, 100);
                fg_pid = atoi(str_pid);
                if (fg_pid != 0) {
                  // Check if fg process got suspended
                  char state[5];
                  sleep(1);
                  if (get_state(fg_pid, state) != -1)
                    if (strcmp(state, "T") == 0) {
                      // Process got suspended
                      job_store[next_job_no].command =
                          (char *)(malloc(sizeof((argv + start)[0])));
                      strcpy(job_store[next_job_no].command, (argv + start)[0]);
                      job_store[next_job_no].running = 0;
                      job_store[next_job_no].exited = 0;
                      job_store[next_job_no].pid = fg_pid;
                      next_job_no++;
                    }
                }
              }
            }
          } else {
            continue;
          }
          if (ret_rcmd == -9) {
            // If child created, get return status of run_cmd
            close(stat_ret_rcmd_pipe[1]);
            char *tmp = (char *)(malloc(1));
            read(stat_ret_rcmd_pipe[0], tmp, 1);
            ret_rcmd = *tmp - '0';
          }
          if (ret_rcmd == -1) {
            // When encountered quit
            overkill(1, NULL);
            return 0;
          }
          if (ret_rcmd == 1) {
            // When error occurs due to forking
            command = NULL;
            err_fork = 1;
            break;
          }
        }
        for (int i = 0; i <= argc; i++) {
          free(argv[i]);
        }
        free(argv);
        if (err_fork != 1) {
          command = strtok_r(NULL, "|", &pipe_token);
        }
        if (has_pipes) {
          next_in = sq_pipe[0];
        }
      }
      write(sq_pipe[1], "\0", 1);
      close(sq_pipe[1]);
      if (has_pipes) {
        int fk = fork();
        if (fk == 0) {
          char buff[1024];
          while (read(next_in, buff, 1024) > 0) {
            printf("%s", buff);
          }
          close(next_in);
          exit(0);
        }
        waitpid(fk, NULL, 0);
      }
      with_pipes_command = strtok_r(NULL, ";", &whc_token);
      if (with_pipes_command != NULL)
        printf("\n"); // To distinguish b/w outputs of tokenized commands
    }
    free(whole_command);
  }
  overkill(1, NULL);
  return 0;
}
