#include "builtins.h"
#include "global.h"
#include "input.h"
#include "lib.h"
#include <stdlib.h>
#include <string.h>

void ss_loop() {
  init();
  int status;
  do {
    char *line = ss_read();
    if (is_empty(line))
      continue;
    char **args = ss_split(line);
    status = ss_run(args);

    if (history && strcmp(line, history->first->prev->cur) != 0) {
      fprintf(histf, "%s\n", line);
      list_append(history, line);
    }

    for (int i = 0; args[i] != NULL; i++) 
      free(args[i]);
    free(args);
  } while (status);
  destroy();
}
