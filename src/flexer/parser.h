enum ASTNodeType {
  Statement,
  Declaration,
  Expression,
  Number, 
  String,
};

struct ASTNode {
  enum ASTNodeType type;
  char *value;
  struct ASTNode *left;
  struct ASTNode *right;
};
