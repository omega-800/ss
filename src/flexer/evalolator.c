#include "../lib.h"
#include "flexer.h"
#include "parser.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

struct PrimitiveArena {
  struct Primitive *val;
  size_t len;
  size_t cap;
};
struct Primitive *eval_un(struct ASTNode *node, struct PrimitiveArena *arena);
struct Primitive *eval_ifelse(struct ASTNode *node, struct PrimitiveArena *arena);
struct Primitive *eval_gr(struct ASTNode *node, struct PrimitiveArena *arena);
struct Primitive *eval_prim(struct ASTNode *node, struct PrimitiveArena *arena);
struct Primitive *eval_bin(struct ASTNode *node, struct PrimitiveArena *arena);
struct Primitive *eval_ast(struct ASTNode *node, struct PrimitiveArena *arena);

struct Primitive *arena_add(struct PrimitiveArena *arena) {
  if (arena->len >= arena->cap) {
    arena->cap += 8;
    arena->val = realloc(arena->val, arena->cap * sizeof(struct Primitive));
    if (arena->val == NULL) {
      printf("shitters\n");
      exit(1);
    }
  }
  arena->len += 1;
  return arena->val + arena->len - 1;
}

struct Primitive *eval_un(struct ASTNode *node, struct PrimitiveArena *arena) {
  struct Primitive *val = eval_ast(node->v.un->child, arena);
  struct Primitive *res = arena_add(arena);
  if (node->v.un->value == TT_Exclamation) {
    if (val->type != PT_Boolean) {
      printf("eval_bin_un: erroneous value");
      exit(1);
    }
    res->type = PT_Boolean;
    res->v.b = !val->v.b;
  } else if (node->v.un->value == TT_Minus) {
    if (val->type != PT_Number) {
      printf("eval_bin_un: erroneous value");
      exit(1);
    }
    res->type = PT_Number;
    res->v.num = -val->v.num;
  } else {
    printf("eval_bin_un: erroneous op type");
    exit(1);
  }
  return res;
}

struct Primitive *eval_gr(struct ASTNode *node, struct PrimitiveArena *arena) {
  return eval_ast(node->v.gr->child, arena);
}

struct Primitive *eval_ifelse(struct ASTNode *node, struct PrimitiveArena *arena) { return NULL; }

struct Primitive *eval_prim(struct ASTNode *node, struct PrimitiveArena *arena) { return node->v.prim; }

char *eval_bin_str(const enum TokenType op, const struct Primitive *lhs,
                   const struct Primitive *rhs) {
  if (lhs->type != PT_String || rhs->type != PT_String || op != TT_Plus)
    return NULL;
  return concat(lhs->v.str, rhs->v.str);
}

float eval_bin_num(const enum TokenType op, const struct Primitive *lhs,
                   const struct Primitive *rhs) {
  switch (op) {
  case TT_Plus:
    return lhs->v.num + rhs->v.num;
  case TT_Percent:
    return fmod(lhs->v.num, rhs->v.num);
  case TT_Minus:
    return lhs->v.num - rhs->v.num;
  case TT_Slash:
    return lhs->v.num / rhs->v.num;
  case TT_Asterisk:
    return lhs->v.num * rhs->v.num;
  case TT_Circumflex:
    return pow(lhs->v.num, rhs->v.num);
  default:
    printf("eval_bin_num: erroneous op type");
    exit(1);
  }
}

int eval_bin_bool(const enum TokenType op, const struct Primitive *lhs,
                  const struct Primitive *rhs) {
  switch (op) {
  case TT_And:
    return lhs->v.b && rhs->v.b;
  case TT_Or:
    return lhs->v.b || rhs->v.b;
  default:
    printf("eval_bin_bool: erroneous op type");
    exit(1);
  }
}

int eval_bin_cmp(const enum TokenType op, const struct Primitive *lhs,
                 const struct Primitive *rhs) {
  switch (op) {
  case TT_Leq:
    return lhs->v.num <= rhs->v.num;
  case TT_Less:
    return lhs->v.num < rhs->v.num;
  case TT_Geq:
    return lhs->v.num >= rhs->v.num;
  case TT_Greater:
    return lhs->v.num > rhs->v.num;
  case TT_Eq:
    return lhs->v.num == rhs->v.num;
  case TT_Neq:
    return lhs->v.num != rhs->v.num;
  default:
    printf("eval_bin_cmp: erroneous op type");
    exit(1);
  }
}

struct Primitive *eval_bin(struct ASTNode *node, struct PrimitiveArena *arena) {
  const struct Primitive *lhs = eval_ast(node->v.bin->left, arena);
  const struct Primitive *rhs = eval_ast(node->v.bin->right, arena);
  struct Primitive *res = arena_add(arena);
  if (node->v.bin->value == TT_Plus) {
    char *str = eval_bin_str(TT_Plus, lhs, rhs);
    if (str != NULL) {
      res->type = PT_String;
      res->v.str = str;
      return res;
    }
  }
  switch (node->v.bin->value) {
  case TT_Plus:
  case TT_Percent:
  case TT_Minus:
  case TT_Slash:
  case TT_Asterisk:
  case TT_Circumflex:
    res->type = PT_Number;
    res->v.num = eval_bin_num(node->v.bin->value, lhs, rhs);
    return res;
  case TT_Leq:
  case TT_Less:
  case TT_Geq:
  case TT_Greater:
  case TT_Eq:
  case TT_Neq:
    res->type = PT_Boolean;
    res->v.num = eval_bin_cmp(node->v.bin->value, lhs, rhs);
    return res;
  case TT_And:
  case TT_Or:
    res->type = PT_Boolean;
    res->v.num = eval_bin_bool(node->v.bin->value, lhs, rhs);
    return res;
  default:
    return NULL;
  }
}

struct Primitive *eval_ast(struct ASTNode *ast, struct PrimitiveArena *arena) {
  switch (ast->type) {
  case NT_IfElse:
    return eval_ifelse(ast, arena);
  case NT_Binary:
    return eval_bin(ast, arena);
  case NT_Unary:
    return eval_un(ast, arena);
  case NT_Group:
    return eval_gr(ast, arena);
  case NT_Primitive:
    return eval_prim(ast, arena);
  default:
    return NULL;
  }
}

struct Primitive *evalolate(struct ASTNode *ast) {
  struct PrimitiveArena *arena = malloc(sizeof(struct PrimitiveArena));
  arena->len = 0;
  arena->cap = 8;
  arena->val = malloc((sizeof(struct Primitive) * arena->cap));
  struct Primitive *res = malloc(sizeof(struct Primitive));
  struct Primitive *val = eval_ast(ast, arena);
  res = memcpy(res, val, sizeof(struct Primitive));
/*
  if(res->type == PT_String) {
    res->v.str = malloc(strlen(val->v.str)+1);
    res->v.str = memcpy(res->v.str, val->v.str, strlen(val->v.str) );
  }
*/
  free(arena->val);
  free(arena);
  return res;
}
