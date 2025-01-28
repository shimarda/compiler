#include "f-cc.h"


LVar *find_lvar(Token *tok) {
	for (LVar *var = locals; var; var = var->next) {
		if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
			return var;
		}
	}
	return NULL;
}

int is_alnum(char c) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '_';
}

void err(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

//次のトークンが期待している記号の時にはトークンを進め、真を返す
bool consume(char *op) {
	if (token->kind != TK_RESERVED || 
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len)) return false;
	token = token->next;
	return true;
}

bool consume_ident() {
	if (token->kind != TK_IDENT) 
		return false;

	token = token->next;
	return true;
}

//次のトークンが期待している記号の時、トークンを進め、それ以外の時はエラーを報告
void expect(char *op) {
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len)) err_at(token->str, "'%c'ではありません", op);
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

//入力文字列pをトークナイズして返す
Token *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		//空白文字をスキップ
		if (isspace(*p)) {
			p++;
			continue;
		}
		// if (memcmp(p, "return", strlen("return")) == 0 && !is_alnum(p[strlen("return")])) {
		// 	tokens[i].ty = TK_RETURN;
		// 	tokens[i].str = p;
		// 	i++;
		// 	p += strlen("return");
		// 	continue;
		// }

		//if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")) {
		if (memcmp(p, "==", strlen("==")) == 0 || memcmp(p, "!=", strlen("!=")) == 0 || memcmp(p, ">=", strlen(">=")) == 0 || memcmp(p, "<=", strlen("<=")) == 0) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		if (strchr("+-*/()<>", *p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 0);
			char *q = p;
			cur->val = strtol(p,&p, 10);
			cur->len = p - q;
			continue;
		}

		if ('a' <= *p && *p <= 'z') {
			cur = new_token(TK_IDENT, cur, p++, 1);
			continue;
		}
		err_at(p, "トークナイズできません");
	}
	new_token(TK_EOF, cur, p, 0);
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

void program() {
	int i = 0;
	while (!at_eof())
		code[i++] = stmt();
	code[i] = NULL;
}

Node *stmt() {
	Node *node;

	if (consume(TK_RETURN)) {
		node = calloc(1, sizeof(Node));
		node->kind;
		node->lhs = expr();
	} else {
		node = expr();
	}

	if (!consume(";")) {
		err_at(token->str, "';'ではありません");
	}
	return node;
}

Node *expr() {
	return assign();
}

Node *assign() {
    Node *node = equality();

	if (consume("="))
		node = new_node(ND_ASSIGN, node, assign());
	return node;
}

Node *equality() {
	Node *node = relational();

	for (;;) {
		if (consume("=="))
			node = new_node(ND_EQ, node, relational());
		else if (consume("!=")) 
			node = new_node(ND_NE, node, relational());
		else 
			return node;
	}
}

Node *relational() {
	Node *node = add();

	for (;;) {
		if (consume("<"))
			node = new_node(ND_LT, node, add());
		else if (consume("<="))
			node = new_node(ND_LE, node, add());
		//>, >=は両辺を入れ替えて<, <=で対応
		else if (consume(">"))
			node = new_node(ND_LT, add(), node);
		else if (consume(">="))
			node = new_node(ND_LE, add(), node);
		else
			return node;
	}
}

Node *add() {
	Node *node = mul();
	
		for (;;) {
		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else
			return node;
	}
}

Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else
			return node;
	}
}

Node *primary() {
	//次のトークンが(なら、( expr ) のはず
	if (consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	} 

	Token *tok = consume_ident();
	if (tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

  LVar *lvar = find_lvar(tok);
  if (lvar) {
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
	//それ以外は数値
	return new_node_num(expect_num());
}

Node *unary() {
	if (consume("+"))
		return unary();
	if (consume("-"))
		return new_node(ND_SUB, new_node_num(0), unary());
	return
		primary();
}
