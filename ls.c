#include "ls.h"
#include <dirent.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

extern char currDir[256];
extern char homeDir[256];

int ignoreDotfiles(const struct dirent *t) {
  if (t->d_name[0] == '.')
    return 0;
  return 1;
}

char *getusername(uid_t uid) {
  struct passwd *store = getpwuid(uid);
  return store->pw_name;
}

char *getgroupname(gid_t gid) {
  struct group *store = getgrgid(gid);
  return store->gr_name;
}

void print_modified_time(time_t time) {
  char *buff = (char *)(malloc(128));
  struct tm lt;
  localtime_r(&time, &lt);
  strftime(buff, sizeof(buff), "%b %d ", &lt);
  char temp[10];
  sprintf(temp, "%02d:%02d", lt.tm_hour, lt.tm_min);
  strcat(buff, temp);
  printf(" %s ", buff);
  free(buff);
}

void longlistprint(const struct stat *details, const char *name,
                   const char *path) {
  if (S_ISLNK(details->st_mode))
    printf("l");
  else
    printf((S_ISDIR(details->st_mode)) ? "d" : "-");
  printf((details->st_mode & S_IRUSR) ? "r" : "-");
  printf((details->st_mode & S_IWUSR) ? "w" : "-");
  printf((details->st_mode & S_IXUSR) ? "x" : "-");
  printf((details->st_mode & S_IRGRP) ? "r" : "-");
  printf((details->st_mode & S_IWGRP) ? "w" : "-");
  printf((details->st_mode & S_IXGRP) ? "x" : "-");
  printf((details->st_mode & S_IROTH) ? "r" : "-");
  printf((details->st_mode & S_IWOTH) ? "w" : "-");
  printf((details->st_mode & S_IXOTH) ? "x" : "-");
  printf(" %3llu ", details->st_nlink);
  printf(" %s ", getusername(details->st_uid));
  printf(" %s ", getgroupname(details->st_gid));
  printf(" %8lld ", details->st_size);
  print_modified_time(details->st_mtime);
  if (S_ISLNK(details->st_mode)) {
    char original[256];
    int ret = readlink(path, original, 256);
    if (ret < 0) {
      perror("Error while Reading Link");
      return;
    } else {
      original[ret] = '\0';
    }
    // Handle softlinks : Display **(Broken Softlink)** at end of line
    struct stat tmp;
    if (lstat(original, &tmp)) {
      strcat(original, " ==> **(Broken Softlink)**");
    }
    char toprint[1024];
    strcpy(toprint, name);
    strcat(toprint, " -> ");
    strcat(toprint, original);
    printf(" %s\n", toprint);
  } else
    printf(" %s\n", name);
}

void run_ls(const char *path, int includeHidden, int longList) {
  if (strcmp(path, ".") == 0) {
    run_ls(currDir, includeHidden, longList);
    return;
  }
  if (strcmp(path, "..") == 0) {
    if (strcmp(currDir, "/") == 0)
      return;
    char *act_path = (char *)(malloc(256));
    for (int i = strlen(currDir) - 1; i >= 0; i--) {
      if ((i > 0) && (currDir[i] == '/' && currDir[i - 1] != '\\')) {
        currDir[i] = '\0';
        strcpy(act_path, currDir);
        currDir[i] = '/';
        break;
      } else if (i == 0 && currDir[i] == '/') {
        strcpy(act_path, "/");
      }
    }
    run_ls(act_path, includeHidden, longList);
    free(act_path);
    return;
  }
  int (*filter)(const struct dirent *) = NULL;
  if (!includeHidden)
    filter = &ignoreDotfiles;
  // Assuming max path len is 256
  char final_path[256];
  if (path[0] == '/') {
    strcpy(final_path, path);
  } else if (path[0] == '~') {
    strcpy(final_path, homeDir);
    strcat(final_path, "/");
    strcat(final_path, path + 1);
  } else {
    strcpy(final_path, currDir);
    strcat(final_path, "/");
    strcat(final_path, path);
  }

  struct stat path_details;
  int rst = lstat(final_path, &path_details);
  if (rst < 0) {
    char error[400];
    strcpy(error, "Cannot access ");
    strcat(error, final_path);
    perror(error);
    return;
  }
  if (S_ISDIR(path_details.st_mode)) {
    struct dirent **namelist;
    int num = scandir(final_path, &namelist, filter, alphasort);
    if (num < 0) {
      char error[400];
      strcpy(error, "Cannot access ");
      strcat(error, final_path);
      perror(error);
      return;
    }
    if (!longList) {
      for (int i = 0; i < num; i++) {
        printf("%s\n", namelist[i]->d_name);
      }
    } else {
      // Assuming path length max 256
      char filepath[256];
      for (int i = 0; i < num; i++) {
        // Obtain file path by concatenating file name & dir path
        strcpy(filepath, path);
        strcat(filepath, "/");
        strcat(filepath, namelist[i]->d_name);
        struct stat details;
        int ret = lstat(filepath, &details);
        if (ret < 0) {
          char error[400];
          strcpy(error, "Cannot access ");
          strcat(error, final_path);
          perror(error);
          return;
        }
        longlistprint(&details, namelist[i]->d_name, filepath);
      }
    }
  } else {
    char *filename = (char *)(malloc(256));
    char *copy_for_free = filename;
    strcpy(filename, final_path);
    for (int i = strlen(filename) - 1; i >= 0; i--) {
      if ((i > 0) && (filename[i] == '/' && filename[i - 1] != '\\')) {
        filename = filename + i + 1;
        break;
      }
    }
    if (!longList) {
      printf("%s\n", filename);
    } else {
      struct stat details;
      int ret = lstat(final_path, &details);
      if (ret < 0) {
        char error[400];
        strcpy(error, "Cannot access ");
        strcat(error, final_path);
        perror(error);
        free(copy_for_free);
        return;
      }
      longlistprint(&details, filename, final_path);
    }
    free(copy_for_free);
  }
}

void ls(const int argc, char *const *argv) {
  int opt_id = 0, invalid = 0;
  int includeHidden = 0, longList = 0;
  while ((opt_id = getopt(argc, argv, "al")) != -1) {
    switch (opt_id) {
    case 'a':
      includeHidden = 1;
      break;
    case 'l':
      longList = 1;
      break;
    default:
      invalid = 1;
      continue;
    }
  }
  optind = 0;

  if (invalid)
    return;

  int cnt = 0;
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      cnt++;
      run_ls(argv[i], includeHidden, longList);
    }
  }
  if (cnt == 0)
    run_ls(currDir, includeHidden, longList);
}
