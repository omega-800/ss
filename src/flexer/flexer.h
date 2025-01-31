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

enum TokenType2 { 
  // Primitives
  TT_Number,
  TT_String,
  // keywords
  TT_If,
  TT_Then,
  TT_Else,
  TT_True,
  TT_False,
  TT_Each,
  TT_Map,
  TT_Filter,
  TT_List,
  // identifier
  TT_Identifier,
  // Symbols
  TT_Leq,           // <=
  TT_Less,          // <
  TT_Equals,        // ==
  TT_Neq,           // !=
  TT_Assign,        // =
  TT_Geq,           // >=
  TT_Greater,       // >
  TT_Exclamation,   // !
  TT_Dollar,        // $
  TT_Percent,       // %
  TT_And,           // &&
  TT_Ampersand,     // &
  TT_LParen,        // (
  TT_RParen,        // )
  TT_Asterisk,      // *
  TT_Plus,          // +
  TT_Comma,         // ,
  TT_Arrow,         // ->
  TT_Minus,         // -
  TT_Dot,           // .
  TT_Slash,         // /
  TT_Colon,         // :
  TT_Semicolon,     // ;
  TT_Question,      // ?
  TT_At,            // @
  TT_LBracket,      // [
  TT_RBracket,      // ]
  TT_Backslash,     // \
  TT_Circumflex,    // ^
  TT_GraveAccent,   // `
  TT_LBrace,        // {
  TT_RBrace,        // }
  TT_APipe,         // |>
  TT_Or,            // ||
  TT_Pipe,          // |
  TT_Tilde,         // ~
  TT_Pound,         // #
};

