#include "global.h"
#include "lib.h"
#include <stdlib.h>
#include <string.h>

void ss_prompt(const char *text) {
  printf("\33[2K\r");
  if (text)
    printf("> %s", text);
  else
    printf("> ");
}
void ss_chpos(const int sub) {
  // printf("\033[1;%dH", new);
  /*
  if (new > old)
    for (int i = new - 1; i > old; i--)
      printf("\033[1C");
  if (old > new)
    for (int i = old; i > new; i--)
      printf("\033[1D");
      */
  for (int i = 0; i < sub; i++)
    printf("\033[1D");
}

char *ss_read() {
  ss_prompt("");
  size_t len = RL_BUF;
  char *str = calloc(sizeof(char), len);
  if (!str) {
    err("cannot allocate input string");
    return str;
  }
  str[0] = '\0';
  char *prev = calloc(sizeof(char), len);

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
    } else if (c == 0x7f && pos > 0) {
      pos--;
      str = removeat(str, pos);
    } else if (c == 27) {
      int c2 = getch();
      int c3 = getch();
      if (c2 == 91) {
        switch (c3) {
        case 65:
          // up
          if (!history) break;
          if (curhist == NULL) {
            prev = restrcpy(prev, str);
            curhist = history->first->prev;
          } else if (curhist != history->first) {
            curhist = curhist->prev;
          } else {
            break;
          }
          str = restrcpy(str, curhist->cur);
          len = strlen(str);
          pos = len;
          break;
        case 66:
          // down
          if (!history) break;
          if (curhist == history->first->prev) {
            curhist = NULL;
            if (prev)
              str = restrcpy(str, prev);
            else {
              str = calloc(sizeof(char), RL_BUF);
              str[0] = '\0';
            }
            len = strlen(str);
            pos = len;
          } else if (curhist != NULL) {
            curhist = curhist->next;
            str = restrcpy(str, curhist->cur);
            len = strlen(str);
            pos = len;
          }
          break;
        case 67:
          // right
          if (pos < strlen(str))
            pos++;
          break;
        case 68:
          // left
          if (pos > 0)
            pos--;
          break;
        }
      }
    } else {
      size_t l = strlen(str);
      if (l + 1 >= len) {
        len += (((int)l / RL_BUF) + 1) * sizeof(char);
        str = realloc(str, len);
        if (!str)
          err("cannot allocate input string");
      }
      str = insertat(str, pos, c);
      pos++;
    }
    ss_prompt(str);
    ss_chpos(strlen(str) - pos);
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

  char quote = '0';

  for (int i = 0; i <= len; i++) {
    if ((is_space(line[i]) && quote == '0') || i == len) {
      tok[itok] = '\0';
      tokens[itokens] = tok;
      toklen = TOK_BUF * sizeof(char);
      tok = malloc(toklen);
      itok = 0;
      itokens++;
    } else if (is_quote(line[i])) {
      if (quote == line[i])
        quote = '0';
      else if (quote == '0')
        quote = line[i];
      else {
        tok[itok] = line[i];
        itok++;
      }
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
