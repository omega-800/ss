#include "parser.h"
#include "../lib.h"
#include "flexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  }
  free(ast);
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
  else if (ast->type != NT_Primitive)
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
    print_type(ast->v.bin->value);
    print_ast(ast->v.bin->right);
    break;
  case NT_Unary:
    print_type(ast->v.un->value);
    print_ast(ast->v.un->child);
    break;
  case NT_Group:
    print_ast(ast->v.gr->child);
    break;
  case NT_Primitive:
    print_primitive(ast->v.prim);
    break;
  }
  if (ast->type == NT_Group)
    printf(")");
  else if (ast->type != NT_Primitive)
    printf("}");
}

struct ASTNode *parse_gr(struct TokenArray tokens, int *cursor) {
  if (tokens.tokens[*cursor]->type != TT_LParen)
    return NULL;
  struct ASTNode *node = malloc(sizeof(struct ASTNode));
  struct Group *gr = malloc(sizeof(struct Group));
  node->type = NT_Group;
  node->v.gr = gr;
  (*cursor)++;
  node->v.gr->child = parse_statement(tokens, cursor);
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
  if (tokens.tokens[*cursor]->type != TT_Then)
    goto freec;
  (*cursor)++;
  node->v.ifelse->truthy = parse_statement(tokens, cursor);
  if (tokens.tokens[*cursor]->type != TT_Else)
    goto freet;
  (*cursor)++;
  node->v.ifelse->falsy = parse_statement(tokens, cursor);
  (*cursor)++;

  return node;
freet:
  free_ast(node->v.ifelse->truthy);
  goto freec;
freec:
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
  node->type = NT_Unary;
  node->v.un = val;
  node->v.un->value = tokens.tokens[*cursor]->type;
  (*cursor)++;
  node->v.un->child = parse_expression(tokens, cursor);
  return node;
}

struct ASTNode *parse_expression(struct TokenArray tokens, int *cursor) {
  struct ASTNode *node = NULL;
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

struct ASTNode *parse_prim(struct TokenArray tokens, int *cursor) {
  if (tokens.tokens[*cursor]->type != TT_String &&
      tokens.tokens[*cursor]->type != TT_Number &&
      tokens.tokens[*cursor]->type != TT_False &&
      tokens.tokens[*cursor]->type != TT_True)
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

struct ASTNode *parse_statement(struct TokenArray tokens, int *cursor) {
  struct ASTNode *ast = NULL;
  if (*cursor >= tokens.len)
    return ast;
  ast = parse_bin(tokens, cursor);
  if (ast == NULL)
    ast = parse_expression(tokens, cursor);
  if (ast == NULL)
    ast = parse_ifelse(tokens, cursor);
  return ast;
}

struct ASTNode *parse_ast(const struct TokenArray tokens) {
  int cursor = 0;
  struct ASTNode *ast = parse_statement(tokens, &cursor);
  return ast;
}
