#include "echo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define isnormalchar (ip == ' ' || (ip != '\"' && ip != '\''))

void echo(const int argc, char *const *argv, const int max_size_arg) {
  char *total_inp = (char *)(malloc(argc * max_size_arg));
  strcpy(total_inp, "");
  for (int i = 1; i < argc; i++) {
    strcat(total_inp, argv[i]);
    if (i != argc - 1)
      strcat(total_inp, " ");
  }
  char currstate = 'n';
  // Used DFA to evaluate the string to be printed
  // States 'n', 'd', 'q', 'c'
  for (int i = 0; i < strlen(total_inp); i++) {
    char ip = total_inp[i];
    switch (currstate) {
    case 'n':
      if (ip == '\"')
        currstate = 'd';
      else if (isnormalchar) {
        currstate = 'c';
        printf("%c", ip);
      } else if (ip == '\'')
        currstate = 'q';
      break;
    case 'c':
      if (isnormalchar)
        printf("%c", ip);
      else if (ip == '\'')
        currstate = 'q';
      else if (ip == '\"')
        currstate = 'd';
      break;
    case 'd':
      if (isnormalchar || ip == '\'')
        printf("%c", ip);
      else if (ip == '\"')
        currstate = 'n';
      break;
    case 'q':
      if (isnormalchar || ip == '\"')
        printf("%c", ip);
      else if (ip == '\'')
        currstate = 'n';
      break;
    }
  }
  printf("\n");
  free(total_inp);
}
