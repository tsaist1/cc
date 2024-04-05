#include "tcc.h"

char *user_input;
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

Token *consume_ident() {
    if (token->kind != TK_IDENTIFIER)
        return NULL;
    Token *t = token;
    token = token->next;
    return t;
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

bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}
bool is_alpha(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool is_alnum(char c)
{
    return is_alpha(c) || ('0' <= c && c <= '9');
}

Token *tokenize()
{
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // Skip whitespace chars
        if (isspace(*p)) {
            p++;
            continue;
        }
        
        // Keyword
        if (startswith(p, "return") && !is_alnum(p[6])) {
            cur = new_token(TK_RESERVED, cur, p, 6);
            p += 6;
            continue;
        }

        // Multi-letter punctuator
        if (startswith(p, "==") || startswith(p, "!=") ||
            startswith(p, "<=") || startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        // Single-letter punctuator
        if (ispunct(*p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        if ('a' <= *p && *p <= 'z') {
            cur = new_token(TK_IDENTIFIER, cur, p++, 1);
            // cur->len = 1;
            continue;
        }
        
        error_at(p, "cannot tokenize");
    }
        
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}
