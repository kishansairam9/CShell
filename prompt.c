#include "prompt.h"
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

extern char currDir[256];
extern char homeDir[256];

// Assuming 128 to be max length of hostname
char *username, hostname[128];

void getUserHostNames() {
  struct passwd *temp = getpwuid(getuid());
  username = temp->pw_name;
  gethostname(hostname, 128);
}

void storePromptString(char *store) {
  getcwd(currDir, 256);
  char *toprintDir = (char *)(malloc(256));
  char *copy_for_free = toprintDir;
  strcpy(toprintDir, currDir);
  if (strncmp(currDir, homeDir, strlen(homeDir)) == 0) {
    toprintDir += strlen(homeDir) - 1;
    toprintDir[0] = '~';
  }
  if (username == NULL)
    getUserHostNames();
  sprintf(store, "<%s@%s:%s> ", username, hostname, toprintDir);
  free(copy_for_free);
}
