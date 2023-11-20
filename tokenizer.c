#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ** Tokenizer *********************************************************
typedef enum
{
    TK_RESERVED, // symbols
    TK_NUM,
    TK_EOF,
} TokenKind;

// Token type
typedef struct Token Token;
struct Token
{
    TokenKind kind; // token type
    Token *next;    // next input token
    int val;
    char *str; // token string
    int len;   // token length
};

bool startswith(char *p, char *q)
{
    return memcpy(p, q, strlen(q)) == 0;
}

static char *user_input;

// current token
Token *token;

void error(char *fmt, ...)
{
    va_list ap; // argument pointer
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// report error msg
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// if next token is expected read forward next token
bool consume(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;

    token = token->next;
    return true;
}

void expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "expected \"%s\"", op);
    token = token->next;
}

int expect_number()
{
    if (token->kind != TK_NUM)
        error("not a number");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token)); // calloc zeroes out memory
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (startswith(p, "==") || startswith(p, "!=") ||
            startswith(p, "<=") || startswith(p, ">="))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
        }

        if (ispunct(*p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "cannot tokenize");
    }
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

// ** Parser *********************************************************
typedef enum
{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // integer
    ND_NEG, // unary -
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=

} NodeKind;

// AST node type
typedef struct Node Node;
struct Node
{
    NodeKind kind; // node type
    Node *lhs;
    Node *rhs;
    int val;
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}
// AST using EBNF (Extended Backus-Naur form)
// recursive descending parsing
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// expr = equality
Node *expr()
{
    return equality();
}

// eqaulity = relational ("==" relational | "!=" relational)*
Node *equality()
{
    Node *node = relational();
    for (;;)
    {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational()
{
    Node *node = add();
    for (;;)
    {
        if (consume("<"))
            node = new_node(ND_LT, node, add());
        else if (consume("<="))
            node = new_node(ND_LE, node, add());
        else if (consume(">"))
            node = new_node(ND_LT, add(), node);
        else if (consume(">="))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}

// add = mul ("+" mul | "-" mul)*
Node *add()
{
    Node *node = mul();
    for (;;)
    {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul()
{
    Node *node = unary();
    for (;;)
    {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary = unary ("+" | "-")? primary
Node *unary()
{
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

// primary = num | "(" expr ")"
Node *primary()
{
    // if next token is "(", it should be "(" expr ")"
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }
    return new_node_num(expect_number());
}

// ** Code generator *********************************************************
void gen(Node *node)
{
    if (node->kind == ND_NUM)
    {
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("	pop rdi\n");
    printf("	pop rax\n");

    switch (node->kind)
    {
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
    case ND_EQ:
        printf("    cmp rax, rdi");
        printf("    sete al");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi");
        printf("    setne al");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi");
        printf("    setl al");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi");
        printf("    setle al");
        printf("    movzb rax, al\n");
        break;
    }
    printf("	push rax\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("invalid number of arguments");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(user_input);
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    gen(node);

    printf("	pop rax\n");
    printf(" 	ret\n");

    return 0;
}
