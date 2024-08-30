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

extern bool consume(char);
extern void expect(char );
extern int expect_num();
extern bool at_eof();
extern Token *new_token(TokenKind , Token *, char *);
extern Token *tokenize();
extern void err_at(char *, char *, ...);

//現在着目しているトークン
Token *token;
char *usr_input;

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}
	//入力プログラム
	usr_input = argv[1];

	//トークナイズ
	token = tokenize();

	char *p = argv[1];

	//アセンブリの前半部分
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");


	//式の最初は数でなければないのでチェック
	//最初のmov
	printf("	mov rax, %d\n", expect_num());

	//トークンの並びを消費しアセンブリを出力
	while (!at_eof()) {
		if (consume('+')) {
			printf("	add rax, %d\n", expect_num());
			continue;
		}

		expect('-');
		printf("	sub rax, %d\n", expect_num());
		}
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
Token *tokenize() {
	char *p = usr_input;
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		//空白文字をスキップ
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (*p == '+' || *p == '-') {
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