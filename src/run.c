#include "builtins.h"
#include "flexer/evalolator.h"
#include "flexer/flexer.h"
#include "flexer/parser.h"
#include "global.h"
#include "input.h"
#include "lib.h"
#include <stdlib.h>
#include <string.h>

int run_eval(const char *line) {
  // printf("%s\n",line);
  struct TokenArray toks = flex(line);
  // print_tokens(toks);
  struct ASTNode *ast = parse_ast(toks);
  // print_ast(ast);
  // printf("\n");
  struct Primitive *res = evalolate(ast);
  print_primitive(res);
  printf("\n");
  free_primitive(res);
  free(res);
  free_ast(ast);
  free_tokens(toks);
  return 1;
}

void run_file(char *filename) {
  FILE *file = fopen(filename, "a+");
  if (file == NULL) {
    printf("error opening file");
    return;
  }
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *line = malloc(fsize + 1);
  if (fread(line, fsize, 1, file)) {
    line[fsize] = '\0';
    run_eval(line);
  } else {
    printf("error reading file");
  }
  fclose(file);
  free(line);
  exit(1);
}

void ss_loop(int sh) {
  init();
  int status;

  do {
    char *line = ss_read();
    if (is_empty(line)) {
      free(line);
      continue;
    }
    char **args;
    if (sh) {
      args = ss_split(line);
      status = ss_run(args);

      if (history && strcmp(line, history->first->prev->cur) != 0) {
        fprintf(histf, "%s\n", line);
        list_append(history, line);
      } else
        free(line);

      for (int i = 0; args[i] != NULL; i++)
        free(args[i]);
      free(args);
    } else {
      if (line[0] == ':' && line[1] == 'q') {
        free(line);
        break;
      }
      status = run_eval(line);

      if (history && strcmp(line, history->first->prev->cur) != 0) {
        fprintf(histf, "%s\n", line);
        list_append(history, line);
      } else
        free(line);
    }

  } while (status);
  destroy();
}
