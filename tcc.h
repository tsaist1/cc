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
    TokenKind kind; // Token type
    Token *next;    // Next input token
    int val;		// If kind is TK_NUM, its value
    char *str; 		// Token string
    int len;		// Token length
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();

extern char *user_input;
extern Token *token;

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
    NodeKind kind; // Node type
    Node *lhs;
    Node *rhs;
    int val;	   // Used if kind == ND_NUM
};

Node *expr();

void codegen(Node *node);
