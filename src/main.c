#include "run.h"
#include <string.h>

int main(int argc, char **argv) {
  if (argc > 2 && strcmp(argv[1], "-f") == 0)
    run_file(argv[2]);
  else if (argc > 1 && strcmp(argv[1], "-s") == 0)
    ss_loop(1);
  else
    ss_loop(0);
  return 0;
}
