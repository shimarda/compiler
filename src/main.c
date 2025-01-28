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
	Node *node = program();
	// Node *node = expr();

	//アセンブリの前半部分
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	//プロローグ
	printf("	push rbp\n");
	printf("	mov rbp, rsp\n");
	printf("	sub rsp, 208\n");

	//抽象構文木を下りながらコード生成
	for (int i = 0; code[i]; i++) {
		gen(code[i]);

		//式の評価結果としてスタックに一つの値が残っているはずなので
		//スタックが溢れないようにpopしておく
		printf("	pop rax\n");
	}


	gen(node);

	//スタックトップをraxへロードし関数の返り値とする
	printf("	mov rsp, rbp\n");
	printf("	pop rbp\n");
	printf("	ret\n");
	return 0;
	}