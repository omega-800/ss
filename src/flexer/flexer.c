#include "flexer.h"
#include "../lib.h"
#include "parser.h"
#include "stdio.h"
#include <regex.h>
#include <stdlib.h>
#include <string.h>

#define NR_RG 10

regex_t *r_if;
regex_t *r_then;
regex_t *r_else;
regex_t *r_let;
regex_t *r_in;
regex_t *r_true;
regex_t *r_false;
regex_t *r_identifier;
regex_t *r_string;
regex_t *r_number;
regex_t *r_arena;

void flex_init() {
  r_arena = malloc(sizeof(regex_t) * NR_RG);
  r_if = r_arena;
  r_then = r_arena + 1;
  r_else = r_arena + 2;
  r_let = r_arena + 3;
  r_in = r_arena + 4;
  r_true = r_arena + 5;
  r_false = r_arena + 6;
  r_identifier = r_arena + 7;
  r_string = r_arena + 8;
  r_number = r_arena + 9;
  regcomp(r_if, "^if", 0);
  regcomp(r_then, "^then", 0);
  regcomp(r_else, "^else", 0);
  regcomp(r_let, "^let", 0);
  regcomp(r_in, "^in", 0);
  regcomp(r_true, "^true", 0);
  regcomp(r_false, "^false", 0);
  regcomp(r_identifier, "^[a-zA-Z_][0-9a-zA-Z_'-]*", 0);
  regcomp(r_string, "^\"\\([^\"]\\|[^\\\\]\\\\\"\\)*\"", 0);
  regcomp(r_number, "^-\\?[0-9]*\\([0-9]\\|\\([0-9]\\.[0-9]\\)\\)[0-9]*", 0);
}

void flex_destroy() {
  regfree(r_if);
  regfree(r_then);
  regfree(r_else);
  regfree(r_let);
  regfree(r_in);
  regfree(r_true);
  regfree(r_false);
  regfree(r_identifier);
  regfree(r_string);
  regfree(r_number);
  free(r_arena);
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
    // puts("no match");
  } else {
    char msgbuf[100];
    regerror(v, preg, msgbuf, sizeof(msgbuf));
    fprintf(stderr, "match failed: %s\n", msgbuf);
    exit(1);
  }
  return 0;
}

int flex_match(int val, regmatch_t *match) {
  match->rm_so = 0;
  switch (val) {
  case TT_Leq:
  case TT_Geq:
  case TT_Neq:
  case TT_Eq:
  case TT_And:
  case TT_Or:
  case TT_Arrow:
  case TT_APipe:
    match->rm_eo = 2;
    break;
  default:
    match->rm_eo = 1;
  }
  return val + 1;
}

int flex_match_next(const char *input, regmatch_t *match) {
  switch (input[0]) {
  case '<':
    if (input[1] == '=')
      return flex_match(TT_Leq, match);
    return flex_match(TT_Less, match);
  case '=':
    if (input[1] == '=')
      return flex_match(TT_Eq, match);
    return flex_match(TT_Assign, match);
  case '>':
    if (input[1] == '=')
      return flex_match(TT_Geq, match);
    return flex_match(TT_Greater, match);
  case '!':
    if (input[1] == '=')
      return flex_match(TT_Neq, match);
    return flex_match(TT_Exclamation, match);
  case '$':
    return flex_match(TT_Dollar, match);
  case '%':
    return flex_match(TT_Percent, match);
  case '&':
    if (input[1] == '&')
      return flex_match(TT_And, match);
    return flex_match(TT_Ampersand, match);
  case '(':
    return flex_match(TT_LParen, match);
  case ')':
    return flex_match(TT_RParen, match);
  case '*':
    return flex_match(TT_Asterisk, match);
  case '+':
    return flex_match(TT_Plus, match);
  case ',':
    return flex_match(TT_Comma, match);
  case '-':
    if (input[1] == '>')
      return flex_match(TT_Arrow, match);
    return flex_match(TT_Minus, match);
  case '.':
    return flex_match(TT_Dot, match);
  case '/':
    return flex_match(TT_Slash, match);
  case ':':
    return flex_match(TT_Colon, match);
  case ';':
    return flex_match(TT_Semicolon, match);
  case '?':
    return flex_match(TT_Question, match);
  case '@':
    return flex_match(TT_At, match);
  case '[':
    return flex_match(TT_LBracket, match);
  case ']':
    return flex_match(TT_RBracket, match);
  case '\\':
    return flex_match(TT_Backslash, match);
  case '^':
    return flex_match(TT_Circumflex, match);
  case '`':
    return flex_match(TT_GraveAccent, match);
  case '{':
    return flex_match(TT_LBrace, match);
  case '}':
    return flex_match(TT_RBrace, match);
  case '|':
    if (input[1] == '>')
      return flex_match(TT_APipe, match);
    if (input[1] == '|')
      return flex_match(TT_Or, match);
    return flex_match(TT_Pipe, match);
  case '~':
    return flex_match(TT_Tilde, match);
  case '#':
    return flex_match(TT_Pound, match);
  }
  regex_t *r_all[NR_RG] = {r_if,    r_then,       r_else,   r_let,    r_in,   r_true,
                      r_false, r_identifier, r_string, r_number};
  for (int i = 0; i < NR_RG; i++)
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

void print_token_type(enum TokenType type) {
  switch (type) {
  case TT_If:
    printf("if");
    break;
  case TT_Then:
    printf("then");
    break;
  case TT_Else:
    printf("else");
    break;
  case TT_Let:
    printf("let");
    break;
  case TT_In:
    printf("in");
    break;
  case TT_True:
    printf("true");
    break;
  case TT_False:
    printf("false");
    break;
  case TT_Identifier:
    printf("[ID]");
    break;
  case TT_String:
    printf("[STRING]");
    break;
  case TT_Number:
    printf("[NUMBER]");
    break;
  case TT_Leq:
    printf("<=");
    break;
  case TT_Less:
    printf("<");
    break;
  case TT_Eq:
    printf("==");
    break;
  case TT_Neq:
    printf("!=");
    break;
  case TT_Assign:
    printf("=");
    break;
  case TT_Geq:
    printf(">=");
    break;
  case TT_Greater:
    printf(">");
    break;
  case TT_Exclamation:
    printf("!");
    break;
  case TT_Dollar:
    printf("$");
    break;
  case TT_Percent:
    printf("%%");
    break;
  case TT_And:
    printf("&&");
    break;
  case TT_Ampersand:
    printf("&");
    break;
  case TT_LParen:
    printf("(");
    break;
  case TT_RParen:
    printf(")");
    break;
  case TT_Asterisk:
    printf("*");
    break;
  case TT_Plus:
    printf("+");
    break;
  case TT_Comma:
    printf(",");
    break;
  case TT_Arrow:
    printf("->");
    break;
  case TT_Minus:
    printf("-");
    break;
  case TT_Dot:
    printf(".");
    break;
  case TT_Slash:
    printf("/");
    break;
  case TT_Colon:
    printf(":");
    break;
  case TT_Semicolon:
    printf(";");
    break;
  case TT_Question:
    printf("?");
    break;
  case TT_At:
    printf("@");
    break;
  case TT_LBracket:
    printf("[");
    break;
  case TT_RBracket:
    printf("]");
    break;
  case TT_Backslash:
    printf("\\");
    break; /* \ */
  case TT_Circumflex:
    printf("^");
    break;
  case TT_GraveAccent:
    printf("`");
    break;
  case TT_LBrace:
    printf("{");
    break;
  case TT_RBrace:
    printf("}");
    break;
  case TT_APipe:
    printf("|>");
    break;
  case TT_Or:
    printf("||");
    break;
  case TT_Pipe:
    printf("|");
    break;
  case TT_Tilde:
    printf("~");
    break;
  case TT_Pound:
    printf("#");
    break;
  }
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
    // printf("c1: %lu\n", cursor);
    if (cursor >= len)
      break;
    // printf("pos: %lu, len: %i\n", cursor, tokens.len);
    int res = flex_match_next(input + cursor, &match);
    if (res == 0) {
      printf("error at pos: %lu, char: %c, prev tok: %s, type: ", cursor, input[cursor], tokens.tokens[tokens.len-1]->value);
      print_token_type(tokens.tokens[tokens.len-1]->type);
      printf("\n");
      exit(1);
    }
    char *value = malloc(sizeof(char) * (match.rm_eo + 1));
    value = substr(input + cursor, value, 0, match.rm_eo);
    cursor += match.rm_eo;
    // printf("c: %lu, val: '%s'\n", cursor, value);
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

  flex_destroy();
  return tokens;
}
