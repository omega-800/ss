#include "parser.h"
#include "../lib.h"
#include "flexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_ARGS 8

struct ASTNode *parse_statement(struct TokenArray tokens, int *cursor);
struct ASTNode *parse_gr(struct TokenArray tokens, int *cursor);
struct ASTNode *parse_prim(struct TokenArray tokens, int *cursor);
struct ASTNode *parse_expression(struct TokenArray tokens, int *cursor);
struct ASTNode *parse_ifelse(struct TokenArray tokens, int *cursor);
struct ASTNode *parse_un(struct TokenArray tokens, int *cursor);
struct ASTNode *parse_bin(struct TokenArray tokens, int *cursor);

void free_primitive(struct Primitive *prim) {
  if (prim->type == PT_String)
    free(prim->v.str);
  free(prim);
}

void free_ast(struct ASTNode *ast) {
  switch (ast->type) {
  case NT_IfElse:
    free_ast(ast->v.ifelse->condition);
    free_ast(ast->v.ifelse->truthy);
    free_ast(ast->v.ifelse->falsy);
    free(ast->v.ifelse);
    break;
  case NT_Binary:
    free_ast(ast->v.bin->left);
    free_ast(ast->v.bin->right);
    // free(ast->v.bin->value);
    free(ast->v.bin);
    break;
  case NT_Unary:
    free_ast(ast->v.un->child);
    // free(ast->v.un->value);
    free(ast->v.un);
    break;
  case NT_Group:
    free_ast(ast->v.gr->child);
    free(ast->v.gr);
    break;
  case NT_Primitive:
    // free(ast->v.prim->value);
    // free_primitive(ast->v.prim);
    free(ast->v.prim);
    break;
  case NT_Call:
    for (int i = 0; i < ast->v.call->len; i++) {
      free_ast(ast->v.call->args[i]);
    }
    free(ast->v.call->args);
    free(ast->v.call);
    break;
  case NT_LetIn:
    for (int i = 0; i < ast->v.letin->len; i++) {
      free_ast(ast->v.letin->let[i]->val);
      free(ast->v.letin->let[i]->args);
      free(ast->v.letin->let[i]);
    }
    free(ast->v.letin->let);
    free_ast(ast->v.letin->in);
    free(ast->v.letin);
    break;
  case NT_Identifier:
    break;
  }
  free(ast);
}

void print_primitive_type(const enum PrimitiveType type) {
  switch (type) {
  case PT_Number:
    printf("Number");
    break;
  case PT_String:
    printf("String");
    break;
  case PT_Boolean:
    printf("Boolean");
    break;
  }
}

void print_primitive(const struct Primitive *prim) {
  switch (prim->type) {
  case PT_Number:
    printf("%.2f", prim->v.num);
    break;
  case PT_String:
    printf("%s", prim->v.str);
    break;
  case PT_Boolean:
    printf("%s", prim->v.b == 1 ? "true" : "false");
    break;
  }
}

void print_ast(struct ASTNode *ast) {
  if (ast->type == NT_Group)
    printf("(");
  else if (ast->type != NT_Primitive && ast->type != NT_Call)
    printf("{");
  switch (ast->type) {
  case NT_IfElse:
    print_ast(ast->v.ifelse->condition);
    printf("?");
    print_ast(ast->v.ifelse->truthy);
    printf(":");
    print_ast(ast->v.ifelse->falsy);
    break;
  case NT_Binary:
    print_ast(ast->v.bin->left);
    print_token_type(ast->v.bin->value);
    print_ast(ast->v.bin->right);
    break;
  case NT_Unary:
    print_token_type(ast->v.un->value);
    print_ast(ast->v.un->child);
    break;
  case NT_Group:
    print_ast(ast->v.gr->child);
    break;
  case NT_Primitive:
    print_primitive(ast->v.prim);
    break;
  case NT_LetIn:
    printf("let[");
    for (int i = 0; i < ast->v.letin->len; i++) {
      printf("%s->", ast->v.letin->let[i]->name);
      for (int j = 0; j < ast->v.letin->let[i]->len; j++)
        printf("%s->", ast->v.letin->let[i]->args[j]);
      print_ast(ast->v.letin->let[i]->val);
      printf(";");
    }
    printf("]in[");
    print_ast(ast->v.letin->in);
    printf("]");
    break;
  case NT_Call:
    printf("|%s", ast->v.call->name);
    if (ast->v.call->len != 0) {
      printf("<|");
      for (int i = 0; i < ast->v.call->len; i++) {
        print_ast(ast->v.call->args[i]);
        if (i < ast->v.call->len - 1)
          printf("<|");
      }
    }
    printf("|");
    break;
  case NT_Identifier:
    break;
  }
  if (ast->type == NT_Group)
    printf(")");
  else if (ast->type != NT_Primitive && ast->type != NT_Call)
    printf("}");
}

struct ASTNode *parse_gr(struct TokenArray tokens, int *cursor) {
  if (tokens.tokens[*cursor]->type != TT_LParen)
    return NULL;
  struct ASTNode *node = malloc(sizeof(struct ASTNode));
  struct Group *gr = malloc(sizeof(struct Group));
  int prevcur = *cursor;
  node->type = NT_Group;
  node->v.gr = gr;
  (*cursor)++;
  node->v.gr->child = parse_statement(tokens, cursor);
  if (node->v.gr->child == NULL || *cursor >= tokens.len ||
      tokens.tokens[*cursor]->type != TT_RParen) {
    if (node->v.gr->child != NULL)
      free_ast(node->v.gr->child);
    free(node);
    (*cursor) = prevcur;
    return NULL;
  }
  (*cursor)++;
  return node;
}

struct ASTNode *parse_ifelse(struct TokenArray tokens, int *cursor) {
  if (tokens.tokens[*cursor]->type != TT_If)
    return NULL;
  struct ASTNode *node = malloc(sizeof(struct ASTNode));
  struct IfElse *val = malloc(sizeof(struct IfElse));
  int prevcur = *cursor;
  node->type = NT_IfElse;
  node->v.ifelse = val;
  (*cursor)++;
  node->v.ifelse->condition = parse_statement(tokens, cursor);
  if (node->v.ifelse->condition == NULL || *cursor >= tokens.len ||
      tokens.tokens[*cursor]->type != TT_Then)
    goto freec;
  (*cursor)++;
  node->v.ifelse->truthy = parse_statement(tokens, cursor);
  if (node->v.ifelse->truthy == NULL || *cursor >= tokens.len ||
      tokens.tokens[*cursor]->type != TT_Else)
    goto freet;
  (*cursor)++;
  node->v.ifelse->falsy = parse_statement(tokens, cursor);
  if (node->v.ifelse->falsy == NULL)
    goto freet;

  return node;
freet:
  if (node->v.ifelse->truthy != NULL)
    free_ast(node->v.ifelse->truthy);
freec:
  if (node->v.ifelse->condition != NULL)
    free_ast(node->v.ifelse->condition);
  free(node->v.ifelse);
  free(node);
  (*cursor) = prevcur;
  return NULL;
}

struct ASTNode *parse_un(struct TokenArray tokens, int *cursor) {
  if (tokens.tokens[*cursor]->type != TT_Exclamation &&
      tokens.tokens[*cursor]->type != TT_Minus)
    return NULL;
  struct ASTNode *node = malloc(sizeof(struct ASTNode));
  struct Unary *val = malloc(sizeof(struct Unary));
  int prevcur = *cursor;
  node->type = NT_Unary;
  node->v.un = val;
  node->v.un->value = tokens.tokens[*cursor]->type;
  (*cursor)++;
  node->v.un->child = parse_expression(tokens, cursor);
  if (node->v.un->child != NULL)
    return node;
  (*cursor) = prevcur;
  free(node->v.un);
  free(node);
  return NULL;
}

struct ASTNode *parse_expression(struct TokenArray tokens, int *cursor) {
  struct ASTNode *node = NULL;
  if (*cursor >= tokens.len)
    return node;
  node = parse_gr(tokens, cursor);
  if (node == NULL)
    node = parse_un(tokens, cursor);
  if (node == NULL)
    node = parse_prim(tokens, cursor);
  return node;
}

int bin_op_prec(enum TokenType type) {
  switch (type) {
  case TT_GraveAccent:
    return 4;
  case TT_Slash:
  case TT_Asterisk:
  case TT_Percent:
    return 3;
  case TT_Eq:
  case TT_Less:
  case TT_Leq:
  case TT_Greater:
  case TT_Geq:
  case TT_Neq:
    return 2;
  case TT_And:
  case TT_Or:
    return 1;
  default:
    return 0;
  }
}

int is_bin_op(enum TokenType type) {
  switch (type) {
  case TT_Plus:
  case TT_Percent:
  case TT_Minus:
  case TT_Slash:
  case TT_Asterisk:
  case TT_Circumflex:
  case TT_And:
  case TT_Or:
  case TT_Leq:
  case TT_Less:
  case TT_Geq:
  case TT_Greater:
  case TT_Eq:
  case TT_Neq:
    return 1;
  default:
    return 0;
  }
}

struct ASTNode *parse_bin(struct TokenArray tokens, int *cursor) {
  struct ASTNode *node = malloc(sizeof(struct ASTNode));
  node->type = NT_Binary;
  struct Binary *val = malloc(sizeof(struct Binary));
  node->v.bin = val;
  int prevc = *cursor;
  node->v.bin->left = parse_expression(tokens, cursor);
  if (node->v.bin->left != NULL && *cursor < tokens.len &&
      is_bin_op(tokens.tokens[*cursor]->type)) {
    node->v.bin->value = tokens.tokens[*cursor]->type;
    (*cursor)++;
    node->v.bin->right = parse_expression(tokens, cursor);
  } else {
    if (node->v.bin->left != NULL)
      free_ast(node->v.bin->left);
    free(node->v.bin);
    free(node);
    // this is too hacky
    (*cursor) = prevc;
    return NULL;
  }
  while (1) {
    if (*cursor >= tokens.len || !is_bin_op(tokens.tokens[*cursor]->type))
      break;
    struct ASTNode *nnode = malloc(sizeof(struct ASTNode));
    nnode->type = NT_Binary;
    struct Binary *nval = malloc(sizeof(struct Binary));
    nnode->v.bin = nval;

    int type = tokens.tokens[*cursor]->type;
    (*cursor)++;
    struct ASTNode *expr = parse_expression(tokens, cursor);
    nnode->v.bin->value = type;
    nnode->v.bin->right = expr;
    // ah yes this is what good quality code looks like
    if (bin_op_prec(type) > bin_op_prec(node->v.bin->value)) {
      // do a little switcheroo
      nnode->v.bin->left = node->v.bin->right;
      node->v.bin->right = nnode;
    } else {
      nnode->v.bin->left = node;
      node = nnode;
    }
  }
  return node;
}

char *parse_str(char *str) {
  size_t len = strlen(str) - 1;
  str = removeat(removeat(str, len), 0);
  len--;
  if (len > 0) {
    int i = 0;
    for (; i < len - 1; i++)
      if (str[i] == '\\' && (str[i + 1] == '"' || str[i + 1] == '\\')) {
        str = removeat(str, i);
        len--;
      }
  }
  // str[strlen(str) - 1] = '\0';
  return str;
}

int is_prim(enum TokenType type) {
  return type == TT_String || type == TT_Number || type == TT_False ||
         type == TT_True;
}

struct ASTNode *parse_prim(struct TokenArray tokens, int *cursor) {
  if (!is_prim(tokens.tokens[*cursor]->type))
    return NULL;
  struct ASTNode *node = malloc(sizeof(struct ASTNode));
  node->type = NT_Primitive;
  struct Primitive *val = malloc(sizeof(struct Primitive));
  node->v.prim = val;
  switch (tokens.tokens[*cursor]->type) {
  case TT_String:
    val->type = PT_String;
    val->v.str = parse_str(tokens.tokens[*cursor]->value);
    break;
  case TT_Number:
    val->type = PT_Number;
    val->v.num = atof(tokens.tokens[*cursor]->value);
    break;
  case TT_True:
  case TT_False:
    val->type = PT_Boolean;
    val->v.b = tokens.tokens[*cursor]->type == TT_True ? 1 : 0;
    break;
  default:
    return NULL;
  }
  (*cursor)++;
  return node;
}

char *parse_identifier(struct TokenArray tokens, int *cursor) {
  if (tokens.tokens[*cursor]->type != TT_Identifier)
    return NULL;
  char *val = tokens.tokens[*cursor]->value;
  (*cursor)++;
  return val;
}

struct Assignment *parse_assignment(struct TokenArray tokens, int *cursor) {
  char *name = parse_identifier(tokens, cursor);
  if (name == NULL)
    return NULL;
  int prevcur = *cursor;
  struct Assignment *res = malloc(sizeof(struct Assignment));
  res->len = 0;
  res->cap = BUF_ARGS;
  res->args = malloc(sizeof(char *) * res->cap);
  res->name = name;
  while (1) {
    if (res->len >= res->cap) {
      res->cap += BUF_ARGS;
      res->args = realloc(res->args, sizeof(char *) * res->cap);
    }
    res->args[res->len] = parse_identifier(tokens, cursor);
    if (res->args[res->len] == NULL)
      break;
    res->len++;
  }
  if (*cursor + 1 >= tokens.len || tokens.tokens[*cursor]->type != TT_Assign)
    // for(int i = 0; i < res->len; i++) free(res->args[i]);
    goto freea;
  (*cursor)++;

  res->val = parse_statement(tokens, cursor);
  if (res->val == NULL || *cursor >= tokens.len ||
      tokens.tokens[*cursor]->type != TT_Semicolon) {
    if (res->val != NULL)
      free(res->val);
    goto freea;
  }

  (*cursor)++;

  return res;
freea:
  (*cursor) = prevcur;
  free(res->args);
  free(res);
  return NULL;
}

struct ASTNode *parse_letin(struct TokenArray tokens, int *cursor) {
  if (tokens.tokens[*cursor]->type != TT_Let)
    return NULL;
  struct ASTNode *node = malloc(sizeof(struct ASTNode));
  struct LetIn *val = malloc(sizeof(struct LetIn));
  int prevcur = *cursor;
  node->type = NT_LetIn;
  node->v.letin = val;
  node->v.letin->len = 0;
  node->v.letin->cap = BUF_ARGS;
  node->v.letin->let = malloc(sizeof(struct Assignment *) * node->v.letin->cap);
  (*cursor)++;

  while (1) {
    if (node->v.letin->len >= node->v.letin->cap) {
      node->v.letin->cap += BUF_ARGS;
      node->v.letin->let = realloc(
          node->v.letin->let, sizeof(struct Assignment *) * node->v.letin->cap);
    }
    node->v.letin->let[node->v.letin->len] = parse_assignment(tokens, cursor);
    if (*cursor >= tokens.len || node->v.letin->let[node->v.letin->len] == NULL)
      break;
    node->v.letin->len++;
  }
  if (*cursor + 1 >= tokens.len || tokens.tokens[*cursor]->type != TT_In)
    goto free_in;

  (*cursor)++;

  node->v.letin->in = parse_statement(tokens, cursor);

  if (node->v.letin->in == NULL)
    goto free_in;

  return node;

free_in:
  for (int i = 0; i < node->v.letin->len; i++) {
    free_ast(node->v.letin->let[i]->val);
    free(node->v.letin->let[i]);
  }
  free(node->v.letin->let);
  free(node->v.letin);
  free(node);
  (*cursor) = prevcur;
  return NULL;
}

struct ASTNode *parse_call(struct TokenArray tokens, int *cursor) {
  if (tokens.tokens[*cursor]->type != TT_Identifier)
    return NULL;

  struct ASTNode *node = malloc(sizeof(struct ASTNode));
  struct Call *val = malloc(sizeof(struct Call));
  node->type = NT_Call;
  node->v.call = val;
  node->v.call->name = parse_identifier(tokens, cursor);
  node->v.call->len = 0;
  node->v.call->cap = BUF_ARGS;
  node->v.call->args = malloc(sizeof(struct ASTNode *) * node->v.call->cap);

  while (1) {
    if (node->v.call->len >= node->v.call->cap) {
      node->v.call->cap += BUF_ARGS;
      node->v.call->args = realloc(
          node->v.call->args, sizeof(struct ASTNode *) * node->v.call->cap);
    }
    node->v.call->args[node->v.call->len] = parse_expression(tokens, cursor);
    if (node->v.call->args[node->v.call->len] == NULL)
      break;
    node->v.call->len++;
  }

  return node;
}

struct ASTNode *parse_statement(struct TokenArray tokens, int *cursor) {
  struct ASTNode *node = NULL;
  if (*cursor >= tokens.len)
    return node;
  node = parse_call(tokens, cursor);
  if (node == NULL)
    node = parse_bin(tokens, cursor);
  if (node == NULL)
    node = parse_expression(tokens, cursor);
  if (node == NULL)
    node = parse_ifelse(tokens, cursor);
  if (node == NULL)
    node = parse_letin(tokens, cursor);
  return node;
}

struct ASTNode *parse_ast(const struct TokenArray tokens) {
  int cursor = 0;
  struct ASTNode *stmt = parse_statement(tokens, &cursor);
  if (cursor < tokens.len) {
    printf("\ncouldn't parse everything, stopped at token %i of %i\n", cursor,
           tokens.len);
    free_ast(stmt);
    return NULL;
  }
  return stmt;
}
