#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "kcc.h"

/// エラーを報告 printfと同じ引数
void error(char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

/// エラー個所を報告する
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	
	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr,"\n");
	exit(1);
}

/**
次のトークンが期待している記号のときには、トークンを一つ読み進めて真を返す。
それ以外の場合には偽を返す。
*/
bool consume(char *op){
	if (token->kind != TK_RESERVED 
	|| strlen(op) != token->len
	|| memcmp(token->str, op, token->len))
		return false;
	token = token->next;
	return true;
}

Token *consume_ident() {
	if(token->kind != TK_IDENT) return NULL;
	Token *prev = token;
	token = token->next;
	return prev;
}

bool consume_tokenKind(TokenKind tk){
	if(token->kind != tk) return false;
	token = token->next;
	return true;
}

/**
次のトークンが期待している記号のときには、トークンを一つ読み進める。
それ以外の場合にはエラーを報告する。
*/
void expect(char *op){
	if (token->kind != TK_RESERVED 
	|| strlen(op) != token->len
	|| memcmp(token->str, op, token->len))
		error_at(token->str, "\"%s\"ではありません",op);
	token = token->next;
}

/**
次のトークンが期待している記号のときには、
トークンを1つ読み進めてその数値を返す。
それ以外の場合にはエラーを報告する。
*/
int expect_number() {
	if(token->kind != TK_NUM) 
		error_at(token->str, "数ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

/// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

bool startswith(char *p, char *q) {
	return memcmp(p,q,strlen(q)) == 0;
}

int is_alnum(char c) {
	return ('a' <= c && c <= 'z') ||
		('A' <= c && c <= 'Z') ||
		('0' <= c && c <= '9') ||
		(c == '_');
}

/// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;
	
	while(*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}
		
		if(startswith(p, "==")
		|| startswith(p, "!=")
		|| startswith(p, ">=")
		|| startswith(p, "<=")) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}
		
		if (strchr("+-*/()<>=;",*p)) {
			cur = new_token(TK_RESERVED, cur, p++,1);
			continue;
		}
		
		if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
			cur = new_token(TK_RETURN, cur, p, 6);
			p += 6;
			continue;
		}
		
		if(strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
			cur = new_token(TK_IF, cur, p, 2);
			p += 2;
			continue;
		}
		
		char *ident=p;
		int ident_count=0;
		while(('a' <= *ident && *ident <= 'z') || 
			('A' <= *ident && *ident <= 'Z')) {
			ident_count++;
			ident++;
		}
		if(ident_count!=0){
			cur = new_token(TK_IDENT, cur, p ,ident_count);
			p += ident_count;
			continue;
		}
		
		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 0);
			char *q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue;
		}
		
		error_at(p,"トークナイズできません");
	}
	
	new_token(TK_EOF, cur, p, 0);
	return head.next;
}

LVar *find_lvar(Token *tok) {
	for (LVar *var = locals; var; var = var->next) {
		if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)){
			return var;
		}
	}
	return NULL;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

void program() {
	LVar lvar_head;
	lvar_head.next=NULL;
	lvar_head.name="";
	lvar_head.offset=0;
	locals = &lvar_head;
	
	int i=0;
	while (!at_eof()) {
		code[i++] = statement();
	}
	code[i] = NULL;
}

Node *statement() {
	Node *node;
	
	if(consume_tokenKind(TK_IF)) {
		node = calloc(1, sizeof(Node));
		node->kind = ND_IF;
		expect("(");
		node->cond = expr();
		expect(")");
		node->then = statement();
		return node;
	}
	
	if (consume_tokenKind(TK_RETURN)) {
		node = calloc(1, sizeof(Node));
		node->kind = ND_RETURN;
		node->lhs = expr();
	}else{
		node = expr();
	}
	
	expect(";");
	return node;
}

Node *expr() {
	return assign();
}

Node *assign() {
	Node *node = equality();
	
	if(consume("=")) {
		node = new_node(ND_ASSIGN, node, assign());
	}
	return node;
}

Node *equality() {
	Node *node = relational();
	
	for(;;) {
		if(consume("=="))
			node = new_node(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_node(ND_NE, node, relational());
		else return node;
	}
}

Node *relational() {
	Node *node = add();
	
	for(;;) {
		if(consume("<"))
			node = new_node(ND_LT, node, add());
		else if (consume("<="))
			node = new_node(ND_LE, node, add());
		else if (consume(">"))
			node = new_node(ND_LT, add(), node);
		else if (consume(">="))
			node = new_node(ND_LE, add(), node);
		else return node;
	}
}

Node *add() {
	Node *node = mul();
	
	for(;;) {
		if(consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if(consume("-"))
			node = new_node(ND_SUB, node, mul());
		else return node;
	}
}

Node *mul() {
	Node *node = unary();
	
	for(;;) {
		if(consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if(consume("/"))
			node = new_node(ND_DIV, node, unary());
		else return node;
	}
}

Node *unary() {
	if (consume("+"))
		return primary();
	if (consume("-"))
		return new_node(ND_SUB, new_node_num(0), primary());
	return primary();
}

Node *primary() {
	if (consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}
	
	Token *tok = consume_ident();
	if(tok!=NULL) {
		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_LVAR;
		
		LVar *lvar = find_lvar(tok);
		if(lvar!=NULL) {
			node->offset = lvar->offset;
		} else {
			lvar = calloc(1, sizeof(LVar));
			lvar->next = locals;
			lvar->name = tok->str;
			lvar->len = tok->len;
			lvar->offset = locals->offset + 8;
			node->offset = lvar->offset;
			locals = lvar;
		}
		return node;
	}
	
	return new_node_num(expect_number());
}
