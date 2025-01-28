enum TokenType { 
  Keyword,
  Identifier,
  String, 
  Number,
  Operator,
};

struct Token {
  enum TokenType type;
  char *value;
};

struct TokenArray {
  struct Token **tokens;
  int len;
};

struct TokenArray flex(const char *str);
void free_tokens(struct TokenArray toks);
