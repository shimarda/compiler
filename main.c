#include "f-cc.h"

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