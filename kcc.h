// トークンの種類
typedef enum {
	TK_RESERVED, // 記号
	TK_IDENT, // 識別子
	TK_RETURN, // return
	TK_NUM, // 整数
	TK_EOF, // 入力の終わりを表すトークン
	TK_IF, // if
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
	ND_ASSIGN, // =
	ND_LVAR, // ローカル変数
	ND_NUM, // integer
	ND_EQ, // ==
	ND_NE, // !=
	ND_LT, // <
	ND_LE, // <=
	ND_RETURN, // return
	ND_IF, // if
} NodeKind;

typedef struct Node Node;
struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val; // kindがND_NUMの場合のみ使う
	int offset; // kindがND＿LVARの場合のみ使う
	
	// "if" (cond) then "else" els
	Node *cond;
	Node *then;
	Node *els;
};

Node *code[100];

typedef struct LVar LVar;
struct LVar {
	LVar *next;
	char *name;
	int len;
	int offset;
};

LVar *locals;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

Token *tokenize(char *p);
void gen(Node *node);

void program();
Node *statement();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();