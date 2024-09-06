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
} TokenKind;

typedef struct Token Token;

//トークン型
struct Token {
	TokenKind kind;	//トークンの型
	Token *next;	//次の入力トークン
	int val;		//kindがTK_NUMの場合その数値
	char *str;		//トークン文字列
};

//抽象構文木のノードの種類
typedef enum {
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NUM,
} NodeKind;

typedef struct Node Node;

//抽象構文木のノードの型
struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

extern bool consume(char);
extern void expect(char );
extern int expect_num();
extern bool at_eof();
extern Token *new_token(TokenKind , Token *, char *);
extern Token *tokenize(char *p);
extern void err_at(char *, char *, ...);
extern Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
extern Node *new_node_num(int val);
extern Node *expr();
extern Node *mul();
extern Node *primary();
extern void gen(Node *node);

//現在着目しているトークン
Token *token;
char *usr_input;

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}
	//入力プログラム
	//トークナイズ
	usr_input = argv[1];
	token = tokenize(usr_input);
	Node *node = expr();

	//アセンブリの前半部分
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	//抽象構文木を下りながらコード生成
	gen(node);

	//スタックトップをraxへロードし関数の返り値とする
	printf("	pop rax\n");
	printf("	ret\n");
	return 0;
	}

//エラーを示す関数
//printfと同じ引数
void err(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

//次のトークンが期待している記号の時にはトークンを進め、真を返す
bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op) return false;
	token = token->next;
	return true;
}

//次のトークンが期待している記号の時、トークンを進め、それ以外の時はエラーを報告
void expect(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
	err_at(token->str, "'%c'ではありません", op);
	token = token->next;
}

//次のトークンが数値の場合、トークンを進め、それ以外はエラー
int expect_num() {
	if (token->kind != TK_NUM) err_at(token->str, "数ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

//トークン終了
bool at_eof() {
	return token->kind == TK_EOF;
}

//新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

//入力文字列pをトークナイズして返す
Token *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;
	char lst[6] = "+-*/()";

	while (*p) {
		//空白文字をスキップ
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' | *p == ')') {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p,&p, 10);
			continue;
		}
		err_at(p, "トークナイズできません");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}

//エラーの箇所を報告する
void err_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - usr_input;
	fprintf(stderr, "%s\n", usr_input);
	fprintf(stderr, "%*s", pos, "");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
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

Node *expr() {
	Node *node = mul();

	for (;;) {
		if (consume('+')) node = new_node(ND_ADD, node, mul());
		else if (consume('-')) node = new_node(ND_SUB, node, mul());
		else return node;
	}
}

Node *mul() {
	Node *node = primary();

	for (;;) {
		if (consume('*')) node = new_node(ND_MUL, node, primary());
		else if (consume('/')) node = new_node(ND_DIV, node, primary());
		else return node;
	}
}

Node *primary() {
	//次のトークンが(なら、( expr ) のはず
	if (consume('(')) {
		Node *node = expr();
		expect(')');
		return node;
	} 

	//それ以外は数値
	return new_node_num(expect_num());
}

void gen(Node *node) {
	if (node->kind == ND_NUM) {
		printf("	push %d\n", node->val);
		return;
	}
	gen(node->lhs);
	gen(node->rhs);

	printf("	pop rdi\n");
	printf("	pop rax\n");

	switch(node->kind) {
	case ND_ADD:
		printf("	add rax, rdi\n");
		break;
	case ND_SUB:
		printf("	sub rax, rdi\n");
		break;
	case ND_MUL:
		printf("	imul rax, rdi\n");
		break;
	case ND_DIV:
		printf("	cqo\n");
		printf("	idiv rdi\n");
		break;
	}
	printf("	push rax\n");
}