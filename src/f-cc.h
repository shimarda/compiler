#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//トークンの種類
typedef enum {
	TK_RESERVED,	//記号
	TK_NUM,			//整数トークン
	TK_EOF,			//入力の終わり
	TK_IDENT,		//識別子
} TokenKind;

typedef struct Token Token;

//トークン型
struct Token {
	TokenKind kind;	//トークンの型
	Token *next;	//次の入力トークン
	int val;		//kindがTK_NUMの場合その数値
	char *str;		//トークン文字列
	int len;
};

//抽象構文木のノードの種類
typedef enum {
	ND_ADD,	// +
	ND_SUB,	// -
	ND_MUL,	// *
	ND_DIV,	// /
	ND_NUM,	//num
	ND_EQ,	//==
	ND_NE,	//!=
	ND_LT, 	// <
	ND_LE,	// <=
	ND_GT,	//>
	ND_GE, // >=
	ND_LVAR, // ローカル変数
	ND_NUM // 整数
} NodeKind;

typedef struct Node Node;

//抽象構文木のノードの型
struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
	int offset; //kindがND_LVARの時に使う
};

extern Token *token;
extern char *usr_input;

bool consume(char *);
bool consume_ident(char *);
void expect(char *);
int expect_num();
bool at_eof();
Token *new_token(TokenKind , Token *, char *, int);
Token *tokenize(char *p);
void err_at(char *, char *, ...);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);


Node *program();
Node *stmt();
Node *expr();
Node *assign();
Node *mul();
Node *primary();
Node *unary();
Node *equality();
Node *relational();
Node *add();

void gen(Node *node);
void gen_lval(Node *node);