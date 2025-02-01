/**
 * Program is ALWAYS only ONE statement, ok? get your imperative ass outta here
 * <Program>    ::= <Statement>
 * <Statement>  ::= <IfElse> | <Binary> | <Unary> | <Group> | <Primitive>
 * <IfElse>     ::= 'if' <Statement> 'then' <Statement> 'else' <Statement>
 * <Binary>     ::= <Expression> ('%'|'^'|'*'|'/'|'+'|'-'|'=='|'>'|'>='|'<'|'<='|'!='|'&&'|'||') <Expression>
 * <Expression> ::= (<Group> | <Primitive>)
 * <Unary>      ::= ('!'|'-') <Expression>
 * <Group>      ::= '(' <Statement> ')'
 * <Primitive>  ::= <String> | <Number> | <Boolean>
 * <String>     ::= STR 
 * <Number>     ::= NUM
 * <Boolean>    ::= BIN
 *
 * STR = ".*"
 * NUM = \d+
 * BIN = (true|false)
 */

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
};

enum ASTBinPrecedence {
  P_Unary,    // ! -
  P_Paren,    // ( ... )
  P_Exponent, // ^
  P_Bin1,     // * /
  P_Bin2,     // + - == >= <= &&
};

typedef struct ASTNode ASTNode;

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

struct Unary {
  int value;
  ASTNode *child;
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
    struct IfElse *ifelse;
    struct Binary *bin;
    struct Unary *un;
    struct Group *gr;
    struct Primitive *prim;
  } v;
};

struct TokenArray;
struct ASTNode *parse_ast(const struct TokenArray tokens);
void free_ast(struct ASTNode *ast);
void print_ast(struct ASTNode *ast);
void print_primitive(const struct Primitive *prim);
void free_primitive(struct Primitive *prim);
