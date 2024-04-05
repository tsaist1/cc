#include "tcc.h"

/*
x86_64 asm
lea -- Load Effective Address
[address] -- Memory Address
*/

//  Pushes given node's address onto the stack
void gen_addr(Node *node)
{
    if (node->kind == ND_LVAR) {
        int offset = (node->name - 'a' + 1) * 8;
        printf("    lea rax, [rbp-%d]\n", offset);
        printf("    push rax\n");
    }
    error("not an lvalue");
}

void load()
{
    printf("    pop rax\n");
    printf("    mov rax, [rax]\n");
    printf("    push rax\n");
}

void store()
{
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    mov [rax], rdi\n");
    printf("    push rdi\n");
}

void gen(Node *node)
{
    switch (node->kind) {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_EXPR_STMT:
        gen(node->lhs);
        printf("	add rsp, 8\n");
        return;
    case ND_LVAR:
        gen_addr(node);
        load();
    case ND_ASSIGN:
        gen_addr(node->lhs);
        gen(node->rhs);
        store();
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("	pop rax\n");
        printf("	jmp .Lreturn\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("	pop rdi\n");
    printf("	pop rax\n");

    switch (node->kind) {
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
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }
    printf("	push rax\n");
}

void codegen(Node *node) {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // Prologue
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rbp, 208\n");
    for (Node *n = node; n; n = n->next) {
        gen(n);
    }

    // Epilogue
    printf(".Lreturn:\n");
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
}
