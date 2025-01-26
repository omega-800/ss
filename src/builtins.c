#include "global.h"
#include "lib.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ss_schutzstaffel(char **args) {
  printf("%s\n%s\n", "My heart goes out to all of you <3", "https://github.com/omega-800/ss/blob/main/src/sus.jpg");
  return 1;
}

int ss_history(char **args) {
  int i = 0;
  do {
    printf("%i: %s\n", i, history->cur);
    history = history->next;
    i++;
  } while (history != history->first);
  return 1;
}

int ss_exec(char **args) {
  int status;
  pid_t pid = fork();
  if (strcmp("exec", args[0]) == 0)
    args += 1;

  if (pid == 0) {
    if (execvp(args[0], args) == -1)
      err("error executing command");
    exit(1);
  } else if (pid < 0) {
    err("error executing child");
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int ss_ne(char **args) {
  err("not implemented");
  return 0;
}

int ss_exit(char **args) { return 0; }

int ss_cd(char **args) {
  if (args[1] == NULL)
    fprintf(stderr, "ss: expected argument to \"cd\"\n");
  else if (chdir(args[1]) != 0)
    perror("ss");
  return 1;
}

// dash
char *ss_builtins[] = {
    "cd", "help", "exit", "exec", "history", "schutzstaffel"
    // - test, printf, pwd, ls, true, false, .
    /*
      ":",    "true", ".",        "alias",  "bg",      "command",   "eval",
 "export",   "fc",     "fg",      "getopts", "hash",
      "pwd",  "read", "readonly", "printf", "set",     "shift",   "test",
      "times", "trap", "type", "ulimit",   "umask",  "unalias", "unset", "wait"
    */
};

int ss_help(char **args) {
  printf("ss - a radically Simple Shell\n");
  printf("builtins:\n");
  for (int i = 0; i < sizeof(ss_builtins) / sizeof(char *); i++)
    printf("%s\n", ss_builtins[i]);
  return 1;
}

int (*ss_fn[])(char **) = {
    &ss_cd, &ss_help, &ss_exit, &ss_exec, &ss_history, &ss_schutzstaffel
    /*
      &ss_ne, &ss_ne, &ss_ne, &ss_ne, &ss_ne, &ss_ne,
    &ss_ne,  &ss_ne, &ss_ne, &ss_ne, &ss_ne, &ss_ne,
    &ss_ne, &ss_ne,   &ss_ne, &ss_ne, &ss_ne, &ss_ne, &ss_ne, &ss_ne,
    &ss_ne, &ss_ne,   &ss_ne, &ss_ne, &ss_ne, &ss_ne, &ss_ne
*/
};
