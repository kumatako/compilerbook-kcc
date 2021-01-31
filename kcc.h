// トークンの種類
typedef enum {
	TK_RESERVED, // 記号
	TK_NUM, // 整数
	TK_EOF, // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;
struct Token {
	TokenKind kind; // トークンの種類
	Token *next; // 次の入力トークンを表すポインタ
	int val; // kindがTK_NUMの場合、数値
	char *str; // トークン文字列
	int len; // トークンの長さ
};

Token *token; // 現在着目しているトークン
char *user_input;

typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_NUM, // integer
	ND_EQ, // ==
	ND_NE, // !=
	ND_LT, // <
	ND_LE, // <=
} NodeKind;

typedef struct Node Node;

struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

Token *tokenize(char *p);
void gen(Node *node);
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();