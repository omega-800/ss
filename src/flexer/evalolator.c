#include "../lib.h"
#include "flexer.h"
#include "parser.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CTX_BUF 32

struct Context {
  struct Assignment **vars;
  size_t len;
  size_t cap;
};

struct PrimitiveArena {
  struct Primitive *val;
  size_t len;
  size_t cap;
};

struct Primitive *eval_un(struct ASTNode *node, struct PrimitiveArena *arena,
                          struct Context ctx);
struct Primitive *eval_call(struct ASTNode *node, struct PrimitiveArena *arena,
                            struct Context ctx);
struct Primitive *eval_letin(struct ASTNode *node, struct PrimitiveArena *arena,
                             struct Context ctx);
struct Primitive *eval_ifelse(struct ASTNode *node,
                              struct PrimitiveArena *arena, struct Context ctx);
struct Primitive *eval_gr(struct ASTNode *node, struct PrimitiveArena *arena,
                          struct Context ctx);
struct Primitive *eval_prim(struct ASTNode *node, struct PrimitiveArena *arena,
                            struct Context ctx);
struct Primitive *eval_bin(struct ASTNode *node, struct PrimitiveArena *arena,
                           struct Context ctx);
struct Primitive *eval_ast(struct ASTNode *node, struct PrimitiveArena *arena,
                           struct Context ctx);

void print_ctx(struct Context ctx) {
  printf("len: %lu, cap: %lu\n", ctx.len, ctx.cap);
  for (int i = 0; i < ctx.len; i++) {
    printf("%s->", ctx.vars[i]->name);
    for (int j = 0; j < ctx.vars[i]->len; j++)
      printf("%s->", ctx.vars[i]->args[j]);
    print_ast(ctx.vars[i]->val);
    printf("\n");
  }
  printf("\n");
}

struct Assignment *find_in_ctx(char *name, struct Context ctx) {
  for (int i = 0; i < ctx.len; i++)
    if (strcmp(ctx.vars[i]->name, name) == 0)
      return ctx.vars[i];
  return NULL;
}

struct Primitive *arena_add(struct PrimitiveArena *arena) {
  if (arena->len >= arena->cap) {
    arena->cap += 8;
    arena->val = realloc(arena->val, arena->cap * sizeof(struct Primitive));
    if (arena->val == NULL) {
      printf("error reallocating evaluation arena\n");
      exit(1);
    }
  }
  arena->len += 1;
  return arena->val + arena->len - 1;
}

struct Primitive *eval_un(struct ASTNode *node, struct PrimitiveArena *arena,
                          struct Context ctx) {
  struct Primitive *val = eval_ast(node->v.un.child, arena, ctx);
  struct Primitive *res = arena_add(arena);
  if (node->v.un.value == TT_Exclamation) {
    if (val->type != PT_Boolean) {
      printf("eval_bin_un: erroneous type ");
      print_primitive_type(val->type);
      printf(" for negation, expected Boolean");
      exit(1);
    }
    res->type = PT_Boolean;
    res->v.b = !val->v.b;
  } else if (node->v.un.value == TT_Minus) {
    if (val->type != PT_Number) {
      printf("eval_bin_un: erroneous type ");
      print_primitive_type(val->type);
      printf(" for negation, expected Number");
      exit(1);
    }
    res->type = PT_Number;
    res->v.num = -val->v.num;
  } else {
    printf("eval_bin_un: erroneous op type ");
    print_token_type(node->v.un.value);
    exit(1);
  }
  return res;
}

struct Primitive *eval_call(struct ASTNode *node, struct PrimitiveArena *arena,
                            struct Context ctx) {
  if (node->type != NT_Call)
    return NULL;

  struct Assignment *var = find_in_ctx(node->v.call.name, ctx);
  if (var == NULL) {
    printf("unbound variable %s\n", node->v.call.name);
    exit(1);
  }

  if (var->len < node->v.call.len)
    printf("expected %lu args in fn %s, got %lu. you fucked something up\n", var->len, var->name, node->v.call.len);

  size_t add = node->v.call.len;

  if (ctx.len + add >= ctx.cap) {
    ctx.cap += (((int)node->v.call.len / CTX_BUF) + 1) * CTX_BUF;
    ctx.vars = realloc(ctx.vars, sizeof(struct Assignment *) * ctx.cap);
  }

  for (int i = 0; i < node->v.call.len; i++) {
    struct Assignment *ass = malloc(sizeof(struct Assignment));

    ass->val = node->v.call.args[i];
    ass->name = var->args[i];
    ass->cap = 0;
    ass->len = 0;

    ctx.vars[ctx.len] = ass;
    ctx.len++;
  }

  struct Primitive *res = eval_ast(var->val, arena, ctx);

  for (int i = 0; i < add; i++) {
    free(ctx.vars[ctx.len - 1]);
    ctx.len--;
  }

  return res;
}

struct Primitive *eval_letin(struct ASTNode *node, struct PrimitiveArena *arena,
                             struct Context ctx) {
  if (node->type != NT_LetIn)
    return NULL;

  if (ctx.len + node->v.letin.len >= ctx.cap) {
    ctx.cap += (((int)node->v.letin.len / CTX_BUF) + 1) * CTX_BUF;
    ctx.vars = realloc(ctx.vars, sizeof(struct Assignment *) * ctx.cap);
  }

  for (int i = 0; i < node->v.letin.len; i++) {
    ctx.vars[ctx.len] = node->v.letin.let[i];
    ctx.len++;
  }

  struct Primitive *res = eval_ast(node->v.letin.in, arena, ctx);

  ctx.len = ctx.len - node->v.letin.len;

  return res;
}

struct Primitive *eval_gr(struct ASTNode *node, struct PrimitiveArena *arena,
                          struct Context ctx) {
  return eval_ast(node->v.gr.child, arena, ctx);
}

struct Primitive *eval_ifelse(struct ASTNode *node,
                              struct PrimitiveArena *arena,
                              struct Context ctx) {
  const struct Primitive *condition =
      eval_ast(node->v.ifelse.condition, arena, ctx);
  if (condition->type != PT_Boolean) {
    printf("eval_bin_un: erroneous condition type ");
    print_primitive_type(condition->type);
    printf(" expected Boolean");
    exit(1);
  }
  if (condition->v.b)
    return eval_ast(node->v.ifelse.truthy, arena, ctx);
  return eval_ast(node->v.ifelse.falsy, arena, ctx);
}

struct Primitive *eval_prim(struct ASTNode *node, struct PrimitiveArena *arena,
                            struct Context ctx) {
  return &node->v.prim;
}

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
    printf("eval_bin_num: erroneous op type ");
    print_token_type(op);
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
    printf("eval_bin_bool: erroneous op type ");
    print_token_type(op);
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
    printf("eval_bin_cmp: erroneous op type ");
    print_token_type(op);
    exit(1);
  }
}

struct Primitive *eval_bin(struct ASTNode *node, struct PrimitiveArena *arena,
                           struct Context ctx) {
  const struct Primitive *lhs = eval_ast(node->v.bin.left, arena, ctx);
  const struct Primitive *rhs = eval_ast(node->v.bin.right, arena, ctx);
  struct Primitive *res = arena_add(arena);
  if (node->v.bin.value == TT_Plus) {
    char *str = eval_bin_str(TT_Plus, lhs, rhs);
    if (str != NULL) {
      res->type = PT_String;
      res->v.str = str;
      return res;
    }
  }
  switch (node->v.bin.value) {
  case TT_Plus:
  case TT_Percent:
  case TT_Minus:
  case TT_Slash:
  case TT_Asterisk:
  case TT_Circumflex:
    res->type = PT_Number;
    res->v.num = eval_bin_num(node->v.bin.value, lhs, rhs);
    return res;
  case TT_Leq:
  case TT_Less:
  case TT_Geq:
  case TT_Greater:
  case TT_Eq:
  case TT_Neq:
    res->type = PT_Boolean;
    res->v.num = eval_bin_cmp(node->v.bin.value, lhs, rhs);
    return res;
  case TT_And:
  case TT_Or:
    res->type = PT_Boolean;
    res->v.num = eval_bin_bool(node->v.bin.value, lhs, rhs);
    return res;
  default:
    return NULL;
  }
}

struct Primitive *eval_ast(struct ASTNode *ast, struct PrimitiveArena *arena,
                           struct Context ctx) {
  switch (ast->type) {
  case NT_Call:
    return eval_call(ast, arena, ctx);
  case NT_LetIn:
    return eval_letin(ast, arena, ctx);
  case NT_IfElse:
    return eval_ifelse(ast, arena, ctx);
  case NT_Binary:
    return eval_bin(ast, arena, ctx);
  case NT_Unary:
    return eval_un(ast, arena, ctx);
  case NT_Group:
    return eval_gr(ast, arena, ctx);
  case NT_Primitive:
    return eval_prim(ast, arena, ctx);
  default:
    return NULL;
  }
}

struct Primitive *evalolate(struct ASTNode *ast) {
  struct PrimitiveArena *arena = malloc(sizeof(struct PrimitiveArena));
  struct Context context = {.cap = CTX_BUF, .len = 0};
  context.vars = malloc(sizeof(struct Assignment *) * context.cap);
  arena->len = 0;
  arena->cap = 8;
  arena->val = malloc((sizeof(struct Primitive) * arena->cap));
  struct Primitive *res = malloc(sizeof(struct Primitive));
  const struct Primitive *val = eval_ast(ast, arena, context);
  res = memcpy(res, val, sizeof(struct Primitive));
  /*
    if(res->type == PT_String) {
      res->v.str = malloc(strlen(val->v.str)+1);
      res->v.str = memcpy(res->v.str, val->v.str, strlen(val->v.str) );
    }
  */
  free(context.vars);
  free(arena->val);
  free(arena);
  return res;
}
