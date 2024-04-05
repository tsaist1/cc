#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ** Tokenizer *********************************************************
typedef enum
{
    TK_RESERVED,   // Keywords or punctuators
    TK_IDENTIFIER, // Identifier
    TK_NUM,        // Integer
    TK_EOF,		   // End of file markers
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
Token *consume_ident();
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
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_EQ,        // ==
    ND_NE,        // !=
    ND_LT,        // <
    ND_LE,        // <=
    ND_ASSIGN,    // = 
    ND_RETURN,    // "return"
    ND_EXPR_STMT, // Expression statement
    ND_LVAR,      // local variable
    ND_NUM,       // integer
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind; // Node type
    Node *next;    // Next node
    Node *lhs;
    Node *rhs;
    char name;     // Used if kind == ND_LVAR
    int val;	   // Used if kind == ND_NUM
    int offset;
     
};

Node *program();

void codegen(Node *node);
