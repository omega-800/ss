/**
 * Program is ALWAYS only ONE statement, ok? get your imperative ass outta here
 * <Program>      ::= <Statement>
 * <Statement>    ::= <IfElse> | <Binary> | <Unary> | <Group> | <Primitive> | <LetIn> | <Call>
 * <IfElse>       ::= 'if' <Statement> 'then' <Statement> 'else' <Statement>
 * <LetIn>        ::= 'let' <Assignments> 'in' <Statement>
 * <Call>         ::= <Identifier> <Expression>*
 * <Assignments>  ::= <Assignment>+
 * <Assignment>   ::= <Identifier>+ = <Statement> ';'
 * <Binary>       ::= <Expression> ('%'|'^'|'*'|'/'|'+'|'-'|'=='|'>'|'>='|'<'|'<='|'!='|'&&'|'||') <Expression>
 * <Expression>   ::= (<Group> | <Unary> | <Primitive> | <Call>)
 * <Unary>        ::= ('!'|'-') <Expression>
 * <Group>        ::= '(' <Statement> ')'
 * <Primitive>    ::= <String> | <Number> | <Boolean>
 * <String>       ::= STR 
 * <Number>       ::= NUM
 * <Boolean>      ::= BIN
 * <Identifier>   ::= ID
 *
 * ID  = [^ ]*
 * STR = ".*"
 * NUM = \d+
 * BIN = (true|false)
 */
#include <stddef.h>

enum PrimitiveType {
  PT_Number,
  PT_String,
  PT_Boolean,
};

enum ASTNodeType {
  NT_IfElse,
  NT_Binary,
  NT_Unary,
  NT_Group,
  NT_Primitive,
  NT_Identifier,
  NT_LetIn,
  NT_Call,
};

enum ASTBinPrecedence {
  P_Unary,    // ! -
  P_Paren,    // ( )
  P_Exponent, // ^
  P_Bin1,     // * /
  P_Bin2,     // + - == >= <= &&
};

typedef struct ASTNode ASTNode;

struct Assignment {
  char *name;
  char **args;
  size_t len;
  size_t cap;
  struct ASTNode *val;
};

struct LetIn {
  struct Assignment **let;
  size_t len;
  size_t cap;
  struct ASTNode *in;
};

struct Call {
  char *name;
  struct ASTNode **args;
  size_t len;
  size_t cap;
};

struct Unary {
  int value;
  ASTNode *child;
};

struct IfElse {
  ASTNode *condition;
  ASTNode *truthy;
  ASTNode *falsy;
};

struct Binary {
  int value;
  ASTNode *left;
  ASTNode *right;
};

struct Group {
  ASTNode *child;
};

struct Primitive {
  enum PrimitiveType type;
  union {
    char *str;
    float num;
    int b;
  } v;
};

struct ASTNode {
  enum ASTNodeType type;
  union {
    struct IfElse ifelse;
    struct Binary bin;
    struct Unary un;
    struct Group gr;
    struct Primitive prim;
    struct LetIn letin;
    struct Call call;
  } v;
};

struct TokenArray;
struct ASTNode *parse_ast(const struct TokenArray tokens);
void free_ast(struct ASTNode *ast);
void print_ast(struct ASTNode *ast);
void print_primitive(const struct Primitive *prim);
void print_primitive_type(enum PrimitiveType);
void free_primitive(struct Primitive *prim);
