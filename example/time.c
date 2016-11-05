#include <stdio.h>
#include <time.h>

int main() {
  time_t date;
  time(&date);
  printf("Date: %s", ctime(&date));
  return 0;
}
