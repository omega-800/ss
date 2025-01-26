#include "builtins.h"
#include "global.h"
#include "input.h"
#include "lib.h"
#include <stdlib.h>
#include <string.h>

int ss_run(char **args) {
  if (args[0] == NULL)
    return 1;

  for (int i = 0; i < sizeof(ss_builtins) / sizeof(char *); i++)
    if (strcmp(ss_builtins[i], args[0]) == 0)
      return (*ss_fn[i])(args);

  return ss_exec(args);
}

void ss_loop() {
  init();
  int status;
  do {
    char *line = ss_read();
    if (is_empty(line))
      continue;
    char **args = ss_split(line);
    status = ss_run(args);

    if (strcmp(line, history->first->prev->cur) != 0) {
      fprintf(histf, "%s\n", line);
      list_append(history, line);
    }

    for (int i = 0; args[i] != NULL; i++) {
      printf("'%s'\n", args[i]);
      free(args[i]);
    }
    free(args);
  } while (status);
  destroy();
}
