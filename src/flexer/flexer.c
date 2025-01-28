#include "flexer.h"
#include "../lib.h"
#include "stdio.h"
#include <regex.h>
#include <stdlib.h>
#include <string.h>

regex_t *r_string;
regex_t *r_number;
regex_t *r_keyword;
regex_t *r_identifier;
regex_t *r_identifier;
regex_t *r_operator;

void flex_init() {
  r_string = malloc(sizeof(regex_t));
  r_number = malloc(sizeof(regex_t));
  r_keyword = malloc(sizeof(regex_t));
  r_identifier = malloc(sizeof(regex_t));
  r_operator = malloc(sizeof(regex_t));
  // TODO: escape sequences
  regcomp(r_string, "^\"[^\"]*\"", 0);
  regcomp(r_number, "^-\\?[0-9]*\\([0-9]\\|\\([0-9]\\.[0-9]\\)\\)[0-9]*", 0);
  regcomp(r_keyword, "^\\(if\\|then\\|else\\|true\\|false\\|each\\|map\\|filter\\|list\\)", 0);
  regcomp(r_identifier, "^[a-zA-Z_][0-9a-zA-Z_'-]*", 0);
  regcomp(r_operator,
          "^\\({\\|}\\|[\\|]\\|(\\|)\\|||\\||>\\||\\|<=\\|<\\|>=\\|>\\|!=\\|!\\|&&\\|&\\|==\\|=\\|-\\|\\+\\|\\*\\|/\\|%\\|\\^\\|?\\|~\\|\\$\\|\\.\\|,\\|:\\|#\\|@\\)",
          0);
}

void flex_destroy() {
  regfree(r_string);
  regfree(r_number);
  regfree(r_keyword);
  regfree(r_identifier);
  regfree(r_operator);
}

int flexec(regex_t *preg, const char *input, regmatch_t *match) {
  int v = regexec(preg, input, 1, match, 0);
  if (!v) {
    /*
    printf("eo: %i, so: %i, m: %.*s\n", match->rm_eo, match->rm_so,
           (match->rm_eo - match->rm_so), input + match->rm_so);
    */
    return 1;
  } else if (v == REG_NOMATCH) {
    //puts("no match");
  } else {
    char msgbuf[100];
    regerror(v, r_number, msgbuf, sizeof(msgbuf));
    fprintf(stderr, "match failed: %s\n", msgbuf);
    exit(1);
  }
  return 0;
}

int flex_match_next(const char *input, regmatch_t *match) {
  regex_t *r_all[] = {r_keyword, r_identifier, r_string, r_number, r_operator};
  for (int i = 0; i <= 4; i++)
    if (flexec(r_all[i], input, match))
      return i + 1;
  return 0;
}

int flex_nextnows(const char *input) {
  int i = 0;
  for (; i < strlen(input); i++)
    if (!is_space(input[i]))
      return i;
  return i;
}

void free_tokens(struct TokenArray toks) {
  for (int i = 0; i < toks.len; i++) {
    free(toks.tokens[i]->value);
    free(toks.tokens[i]);
  }
  free(toks.tokens);
}

void print_tokens(struct TokenArray toks) {
  for (int i = 0; i < toks.len; i++)
    printf("i: %i, type: %i, value: '%s'\n", i, toks.tokens[i]->type,
           toks.tokens[i]->value);
}

#define TOK_BUF 16
struct TokenArray flex(const char *input) {
  flex_init();
  size_t cursor = 0;
  size_t len = strlen(input);
  regmatch_t match;
  size_t toklen = TOK_BUF;
  struct TokenArray tokens = {
      .len = 0, .tokens = malloc(toklen * sizeof(struct Token **))};
  while (1) {
    cursor += flex_nextnows(input + cursor);
    //printf("c1: %lu\n", cursor);
    if (cursor >= len)
      break;
    // printf("pos: %lu, len: %i\n", cursor, tokens.len);
    int res = flex_match_next(input + cursor, &match);
    if (!res) {
      printf("error at pos: %lu\n", cursor);
      exit(1);
    }
    char *value = malloc(sizeof(char) * (match.rm_eo + 1));
    value = substr(input + cursor, value, 0, match.rm_eo);
    cursor += match.rm_eo;
    //printf("c: %lu, val: '%s'\n", cursor, value);
    struct Token *t = malloc(sizeof(struct Token));
    t->type = res - 1;
    t->value = value;
    tokens.tokens[tokens.len] = t;
    tokens.len += 1;
    if (tokens.len >= toklen) {
      toklen += TOK_BUF;
      tokens.tokens = realloc(tokens.tokens, toklen * sizeof(struct Token **));
    }
  }

  print_tokens(tokens);

  flex_destroy();
  return tokens;
}
