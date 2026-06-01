#ifndef MALUCH_H
#define MALUCH_H

#include <stdint.h>
#include <stdbool.h>

#define COLS 80
#define ROWS 40

typedef struct Maluch {
    union {
        struct {
            uint16_t r0;
            uint16_t r1;
            uint16_t r2;
            uint16_t r3;
            uint16_t r4;
            uint16_t r5;
            uint16_t r6;
            uint16_t r7;
            uint16_t r8;
            uint16_t r9;
            uint16_t r10;
            uint16_t r11;
            uint16_t r12;
            uint16_t r13;
            uint16_t r14;
            uint16_t r15;
        };
        uint16_t regs[16];
    } reg_file;
    uint16_t ip;
    uint16_t rom[0x8000];
    uint16_t ram[0x8000];
    uint16_t vram[COLS * ROWS];
    struct {
        uint8_t fg;
        uint8_t bg;
    } colors[COLS * ROWS];
    struct {
        uint8_t sign : 1;
        uint8_t overflow : 1;
        uint8_t carry : 1;
        uint8_t zero : 1;
    } flags;
} Maluch;

typedef struct Insn {
    uint16_t src_reg : 4;
    uint16_t dst_reg : 4;
    uint16_t functor : 3;
    uint16_t immediate : 1;
    uint16_t opcode : 4;
} Insn;


void maluchInit(Maluch *const maluch);

// MISC
char *regToText(const uint8_t reg_num);

// STACK
void pushToStack(Maluch *const maluch, const uint16_t val);
uint16_t pullFromStack(Maluch *const maluch);

// OPCODES
void invalid(Maluch *const maluch);
void rsrvd(Maluch *const maluch);
void mov(Maluch *const maluch, const uint8_t dst, const uint16_t val);
void ldw(Maluch *const maluch, const uint8_t dst, const uint16_t addr);
void stw(Maluch *const maluch, const uint16_t addr, const uint8_t src);
void call(Maluch *const maluch, const uint16_t addr);
void ret(Maluch *const maluch);
void iret(Maluch *const maluch);
void push(Maluch *const maluch, const uint16_t val);
void pull(Maluch *const maluch, const uint8_t dst);
void add(Maluch *const maluch, const uint8_t dst, const uint16_t val);
void sub(Maluch *const maluch, const uint8_t dst, const uint16_t val);
void and(Maluch *const maluch, const uint8_t dst, const uint16_t val);
void or(Maluch *const maluch, const uint8_t dst, const uint16_t val);
void xor(Maluch *const maluch, const uint8_t dst, const uint16_t val);
void lsl(Maluch *const maluch, const uint8_t dst, const uint16_t val);
void lsr(Maluch *const maluch, const uint8_t dst, const uint16_t val);
void cmp(Maluch *const maluch, const uint8_t dst, const uint16_t val);
void test(Maluch *const maluch, const uint8_t dst, const uint16_t val);
void not(Maluch *const maluch, const uint8_t dst);
void jmp(Maluch *const maluch, const uint16_t target);
void bee(Maluch *const maluch, const uint16_t target);
void bne(Maluch *const maluch, const uint16_t target);
void bge(Maluch *const maluch, const uint16_t target);
void ble(Maluch *const maluch, const uint16_t target);
void bgg(Maluch *const maluch, const uint16_t target);
void bll(Maluch *const maluch, const uint16_t target);
void boo(Maluch *const maluch, const uint16_t target);
void bbs(Maluch *const maluch, const uint16_t target);
void bss(Maluch *const maluch, const uint16_t target);
void bns(Maluch *const maluch, const uint16_t target);
void bae(Maluch *const maluch, const uint16_t target);
void bbe(Maluch *const maluch, const uint16_t target);
void baa(Maluch *const maluch, const uint16_t target);
void bbb(Maluch *const maluch, const uint16_t target);
void bno(Maluch *const maluch, const uint16_t target);

// stepping function
void step(Maluch *const maluch);
#endif
