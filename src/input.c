#include "global.h"
#include "lib.h"
#include <stdlib.h>
#include <string.h>

void ss_prompt(const char *text) {
  printf("\33[2K\r");
  printf("> %s", text);
}

#define RL_BUF 1024
char *ss_read() {
  ss_prompt("");
  size_t len = RL_BUF;
  char *str = malloc(len * sizeof(char));
  char *prev = malloc(len * sizeof(char));
  if (!str)
    err("cannot allocate input string");

  size_t pos = 0;
  struct ListItem *curhist = NULL;
  while (1) {
    int c = getch();
    if (c == '\n' || c == EOF) {
      str[pos] = '\0';
      if (prev != str)
        free(prev);
      printf("\n");
      return str;
    }
    if (c == 27) {
      int c2 = getch();
      int c3 = getch();
      if (c2 == 91) {
        switch (c3) {
        case 65:
          // up
          if (curhist == NULL) {
            //strcpy(prev, str);
            curhist = history->last;
          } else if (curhist != history->first) {
            curhist = curhist->prev;
          }
          //strcpy(str, curhist->cur);
          ss_prompt(str);
          break;
        case 66:
          // down
          if (curhist == history->last) {
            curhist = NULL;
            //strcpy(str, prev);
          } else if (curhist != NULL) {
            curhist = curhist->next;
            //strcpy(str, curhist->cur);
          }
          ss_prompt(str);
          break;
        case 67:
          // right
          printf("\033[1C");
          break;
        case 68:
          // left
          printf("\033[1D");
          break;
        }
      }
      continue;
    }

    printf("%c", c);
    str[pos] = c;
    pos++;
    if (pos < len)
      continue;

    len += RL_BUF * sizeof(char);
    str = realloc(str, len);
    if (!str)
      err("cannot allocate input string");
  }
}

#define TOK_BUF 16
char **ss_split(const char *line) {
  size_t len = strlen(line);
  size_t toklen = TOK_BUF * sizeof(char);
  char *tok = malloc(toklen);
  int itok = 0;

  size_t tokenslen = TOK_BUF * sizeof(char *);
  char **tokens = malloc(tokenslen);
  int itokens = 0;

  for (int i = 0; i <= len; i++) {
    if (is_space(line[i]) || i == len) {
      tok[itok] = '\0';
      tokens[itokens] = tok;
      toklen = TOK_BUF * sizeof(char);
      tok = malloc(toklen);
      itok = 0;
      itokens++;
    } else {
      tok[itok] = line[i];
      itok++;
    }
    if (i == len)
      continue;
    if (itokens >= tokenslen) {
      tokenslen += TOK_BUF * sizeof(char *);
      tokens = realloc(tokens, tokenslen);
      if (!tokens)
        err("cannot allocate tokens");
    }
    if (itok >= toklen) {
      toklen += TOK_BUF * sizeof(char);
      tok = realloc(tok, toklen);
      if (!tok)
        err("cannot allocate tokens");
    }
  }
  tokens[itokens] = NULL;
  free(tok);
  return tokens;
}
