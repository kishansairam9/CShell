#include "cd.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern char currDir[256];
extern char homeDir[256];

void cd(const int argc, char *const *argv) {
  if (argc == 1) {
    if (chdir(homeDir) != 0)
      perror("cd failed");
  } else if (argc == 2) {
    if (strcmp(argv[1], ".") == 0)
      return;
    if (strcmp(argv[1], "..") == 0) {
      if (strcmp(currDir, "/") == 0)
        return;
      for (int i = strlen(currDir) - 1; i >= 0; i--) {
        if ((i > 0) && (currDir[i] == '/' && currDir[i - 1] != '\\')) {
          currDir[i] = '\0';
          break;
        } else if (i == 0 && currDir[i] == '/') {
          currDir[i + 1] = '\0';
        }
      }
      chdir(currDir);
    } else {
      char final_path[256];
      if (argv[1][0] == '~') {
        strcpy(final_path, homeDir);
        strcat(final_path, argv[1] + 1);
      } else {
        strcpy(final_path, argv[1]);
      }
      struct stat details;
      lstat(final_path, &details);
      if (S_ISLNK(details.st_mode)) {
        char handleLink[256];
        readlink(argv[1], handleLink, 256);
        if (chdir(handleLink) != 0)
          perror("cd failed");
      } else {
        if (chdir(final_path) != 0)
          perror("cd failed");
      }
    }
    getcwd(currDir, 256);
  } else {
    fprintf(stderr,
            "Incorrect Number of Arguments: cd takes only one argument\n");
  }
}
