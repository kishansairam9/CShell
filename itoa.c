#include "itoa.h"
#include <stdio.h>

void b10itoa(int val, char *store) {
  // Assuming val is positive
  int itr = 0;
  while (val > 0) {
    store[itr] = '0' + (val % 10);
    val /= 10;
    itr++;
  }
  store[itr] = '\0';
  for (int i = 0; i < itr / 2; i++) {
    char temp = store[itr - 1 - i];
    store[itr - 1 - i] = store[i];
    store[i] = temp;
  }
}