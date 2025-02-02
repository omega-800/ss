#include "run.h"
#include <string.h>

int main(int argc, char **argv) {
  if (argc > 2 && strcmp(argv[1], "-f") == 0)
    run_file(argv[2]);
  else
    ss_loop();
  return 0;
}
