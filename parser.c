#include "tcc.h"

/* ***** Parsing Grammar *****
 program    = stmt*
 stmt       = expr ";"
 expr       = assign
 assign     = equality ("=" assign)?
 equality   = relational ("==" relational | "!=" relational)*
 relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 add        = mul ("+" mul | "-" mul)*
 mul        = unary ("*" unary | "/" unary)*
 unary      = ("+" | "-")? primary
 primary    = num | ident | "(" expr ")"
*/

Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_unary(NodeKind kind, Node *expr)
{
    Node *node = new_node(kind);
    node->lhs = expr;
    return node;
}

Node *new_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

/* AST using EBNF (Extended Backus-Naur form)
   recursive descent parser */ 
// forward declarations for mutually recursive procedures
Node *stmt();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *program() 
{
    Node head;
    head.next = NULL;
    Node *cur = &head;

    while (!at_eof()) {
        cur->next = stmt();
        cur = cur->next;
    }
    return head.next;
}

Node *assign() 
{
    Node *node = equality();
    if (consume("="))
        node = new_binary(ND_ASSIGN, node, assign());
    return node;
}

Node *expr()
{
    return equality();
}

Node *stmt()
{
    if (consume("return")) {
        Node *node = new_unary(ND_RETURN, expr());
        expect(";");
        return node;
    }
    Node *node = expr();
    expect(";");
    return node;
}

Node *equality()
{
    Node *node = relational();
    for (;;) {
        if (consume("=="))
            node = new_binary(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_binary(ND_NE, node, relational());
        else
            return node;
    }
}

Node *relational()
{
    Node *node = add();
    for (;;) {
        if (consume("<"))
            node = new_binary(ND_LT, node, add());
        else if (consume("<="))
            node = new_binary(ND_LE, node, add());
        else if (consume(">"))
            node = new_binary(ND_LT, add(), node);
        else if (consume(">="))
            node = new_binary(ND_LE, add(), node);
        else
            return node;
    }
}

Node *add()
{
    Node *node = mul();
    for (;;) {
        if (consume("+"))
            node = new_binary(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_binary(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul()
{
    Node *node = unary();
    for (;;) {
        if (consume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_binary(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary()
{
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_binary(ND_SUB, new_num(0), primary());
    return primary();
}

Node *primary()
{
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    
    return new_num(expect_number());
}
