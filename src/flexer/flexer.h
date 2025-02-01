enum TokenType { 
  TT_If,
  TT_Then,
  TT_Else,
  TT_True,
  TT_False,
  TT_Identifier,
  TT_String,
  TT_Number,
  TT_Leq,           // <=
  TT_Less,          // <
  TT_Eq,            // ==
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
  TT_Backslash,     /* \ */
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
void print_tokens(struct TokenArray toks);
void print_type(enum TokenType type);
