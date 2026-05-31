#include "maluch.h"

#include <assert.h>
#include <stdio.h>

char *regToText(const uint8_t reg_num) {
    assert(reg_num < 16 && "There are only 16 registers (r0-r15)");
    switch(reg_num) {
        case 0:
            return "r0";
        case 1:
            return "r1";
        case 2:
            return "r2";
        case 3:
            return "r3";
        case 4:
            return "r4";
        case 5:
            return "r5";
        case 6:
            return "r6";
        case 7:
            return "r7";
        case 8:
            return "r8";
        case 9:
            return "r9";
        case 10:
            return "r10";
        case 11:
            return "r11";
        case 12:
            return "r12";
        case 13:
            return "r13";
        case 14:
            return "r14";
        case 15:
            return "r15";
        default: __builtin_unreachable();
    }
}

void updateCpuFlagsArithmetic(Maluch *const maluch, const uint32_t res, const uint16_t dst_val, const uint16_t src_val, bool subtraction) {
    const uint16_t res_16 = (uint16_t) res;

    maluch->flags.carry = (res > 0xFFFF);
    maluch->flags.sign = (res_16 & 0x8000) != 0;
    maluch->flags.zero = (res_16 == 0);
    if(subtraction) {
        maluch->flags.overflow = ((dst_val ^ src_val) & (dst_val ^ res_16) & 0x8000) != 0;
    } else {
        maluch->flags.overflow = ((dst_val ^ res_16) & (src_val ^ res_16) & 0x8000) != 0;
    }
}

void updateCpuFlagsLogic(Maluch *const maluch, const uint16_t res) {
    maluch->flags.sign = (res & 0x8000) != 0;
    maluch->flags.zero = (res == 0);

    maluch->flags.carry = 0;
    maluch->flags.overflow = 0;
}

void pushToStack(Maluch *const maluch, const uint16_t val) {
    assert((maluch->reg_file.r2 > 0x8000 || maluch->reg_file.r2 == 0) && "unknown how to act for values 0x8000 and below");
    maluch->ram[--maluch->reg_file.r2 - 0x8000] = val;
}

uint16_t pullFromStack(Maluch *const maluch) {
    assert(maluch->reg_file.r2 >= 0x8000 && "unknown how to act for values below 0x8000");
    return maluch->ram[maluch->reg_file.r2++ - 0x8000];
}

void invalid(Maluch *const maluch) {
    printf("Invalid insn at IP = %d\n", maluch->ip);
}

void rsrvd(Maluch *const maluch) {
    printf("Reserved insn at IP = %d\n", maluch->ip);
}

void mov(Maluch *const maluch, const uint8_t dst, const uint16_t val) { 
    assert(dst > 0 && "Cannot modify r0.");
    assert(dst < 16 && "There are only 16 registers (r0-r15)");

    maluch->reg_file.regs[dst] = val;
}

void ldw(Maluch *const maluch, const uint8_t dst, const uint16_t addr) {
    assert(dst > 0 && "Cannot modify r0.");
    assert(dst < 16 && "There are only 16 registers (r0-r15)");

    if(addr < 0x8000) {
        maluch->reg_file.regs[dst] = maluch->rom[addr];
    } else {
        maluch->reg_file.regs[dst] = maluch->ram[addr - 0x8000];
    }
}

void stw(Maluch *const maluch, const uint16_t addr, const uint8_t src) {
    assert(src < 16 && "There are only 16 registers (r0-r15)");

    if(addr < 0x8000) {
        // y*cols +x 
        const uint8_t row = addr / 256;
        const uint8_t column = addr % 256;
        if(column > COLS || row > ROWS) return;
        maluch->vram[row * COLS + column] = maluch->reg_file.regs[src];
    } else {
        maluch->ram[addr - 0x8000] = maluch->reg_file.regs[src];
    }
}

void call(Maluch *const maluch, const uint16_t addr) {
    pushToStack(maluch, addr);
    maluch->ip = addr;
}

void ret(Maluch *const maluch) {
    maluch->ip = pullFromStack(maluch);
}

void iret(Maluch *const maluch) {
    fprintf(stderr, "Interrupts unsupported yet.");
    maluch->ip = pullFromStack(maluch);
}

void push(Maluch *const maluch, const uint16_t val) {
    pushToStack(maluch, val);
}

void pull(Maluch *const maluch, const uint8_t dst) {
    assert(dst > 0 && "Cannot modify r0.");
    assert(dst < 16 && "There are only 16 registers (r0-r15)");

    maluch->reg_file.regs[dst] = pullFromStack(maluch);
}

void add(Maluch *const maluch, const uint8_t dst, const uint16_t val) {
    assert(dst > 0 && "Cannot modify r0.");
    assert(dst < 16 && "There are only 16 registers (r0-r15)");

    const uint32_t res = maluch->reg_file.regs[dst] + val;
    updateCpuFlagsArithmetic(maluch, res, dst, val, false);
    maluch->reg_file.regs[dst] = (uint16_t)res;
}

void sub(Maluch *const maluch, const uint8_t dst, const uint16_t val) {
    assert(dst > 0 && "Cannot modify r0.");
    assert(dst < 16 && "There are only 16 registers (r0-r15)");

    const uint32_t res = maluch->reg_file.regs[dst] - val;
    updateCpuFlagsArithmetic(maluch, res, dst, val, true);
    maluch->reg_file.regs[dst] = (uint16_t)res;
}

void and(Maluch *const maluch, const uint8_t dst, const uint16_t val) {
    assert(dst > 0 && "Cannot modify r0.");
    assert(dst < 16 && "There are only 16 registers (r0-r15)");

    const uint32_t res = maluch->reg_file.regs[dst] & val;
    updateCpuFlagsLogic(maluch, res);
    maluch->reg_file.regs[dst] = (uint16_t)res;
}

void or(Maluch *const maluch, const uint8_t dst, const uint16_t val) {
    assert(dst > 0 && "Cannot modify r0.");
    assert(dst < 16 && "There are only 16 registers (r0-r15)");

    const uint32_t res = maluch->reg_file.regs[dst] | val;
    updateCpuFlagsLogic(maluch, res);
    maluch->reg_file.regs[dst] = (uint16_t)res;
}

void xor(Maluch *const maluch, const uint8_t dst, const uint16_t val) {
    assert(dst > 0 && "Cannot modify r0.");
    assert(dst < 16 && "There are only 16 registers (r0-r15)");

    const uint32_t res = maluch->reg_file.regs[dst] ^ val;
    updateCpuFlagsLogic(maluch, res);
    maluch->reg_file.regs[dst] = (uint16_t)res;
}

void lsl(Maluch *const maluch, const uint8_t dst, const uint16_t val) {
    assert(dst > 0 && "Cannot modify r0.");
    assert(dst < 16 && "There are only 16 registers (r0-r15)");

    const uint32_t res = maluch->reg_file.regs[dst] << (val % 16);
    updateCpuFlagsLogic(maluch, res);
    maluch->reg_file.regs[dst] = (uint16_t)res;
}

void lsr(Maluch *const maluch, const uint8_t dst, const uint16_t val) {
    assert(dst > 0 && "Cannot modify r0.");
    assert(dst < 16 && "There are only 16 registers (r0-r15)");

    const uint16_t res = maluch->reg_file.regs[dst] >> (val % 16);
    updateCpuFlagsLogic(maluch, res);
    maluch->reg_file.regs[dst] = (uint16_t)res;
}

void cmp(Maluch *const maluch, const uint8_t dst, const uint16_t val) {
    assert(dst > 0 && "Cannot modify r0.");
    assert(dst < 16 && "There are only 16 registers (r0-r15)");

    const uint32_t res = maluch->reg_file.regs[dst] - (val % 16);
    updateCpuFlagsArithmetic(maluch, res, dst, val, true);
}

void test(Maluch *const maluch, const uint8_t dst, const uint16_t val) {
    assert(dst > 0 && "Cannot modify r0.");
    assert(dst < 16 && "There are only 16 registers (r0-r15)");

    const uint32_t res = maluch->reg_file.regs[dst] & (val % 16);
    updateCpuFlagsLogic(maluch, res);
}

void not(Maluch *const maluch, const uint8_t dst) {
    assert(dst > 0 && "Cannot modify r0.");
    assert(dst < 16 && "There are only 16 registers (r0-r15)");

    maluch->reg_file.regs[dst] = ~maluch->reg_file.regs[dst];
    updateCpuFlagsLogic(maluch, maluch->reg_file.regs[dst]);
}

void jmp(Maluch *const maluch, const uint16_t target) {
    maluch->ip = target;
}

void bee(Maluch *const maluch, const uint16_t target) {
    if(maluch->flags.zero) maluch->ip = target;
}

void bne(Maluch *const maluch, const uint16_t target) {
    if(!maluch->flags.zero) maluch->ip = target;
}

void bge(Maluch *const maluch, const uint16_t target) {
    if(maluch->flags.sign == maluch->flags.overflow) maluch->ip = target;
}

void ble(Maluch *const maluch, const uint16_t target) {
    if(maluch->flags.zero || maluch->flags.sign != maluch->flags.overflow)
        maluch->ip = target;
}

void bgg(Maluch *const maluch, const uint16_t target) {
    if(maluch->flags.zero == false && maluch->flags.sign == maluch->flags.overflow)
        maluch->ip = target;
}

void bll(Maluch *const maluch, const uint16_t target) {
    if(maluch->flags.sign != maluch->flags.overflow) maluch->ip = target;
}

void boo(Maluch *const maluch, const uint16_t target) {
    if(maluch->flags.overflow) maluch->ip = target;
}

void bbs(Maluch *const maluch, const uint16_t target) {
    if(maluch->reg_file.r0 & 0xF) maluch->ip = target;
}

void bss(Maluch *const maluch, const uint16_t target) {
    if(maluch->flags.sign) maluch->ip = target;
}

void bns(Maluch *const maluch, const uint16_t target) {
    if(!maluch->flags.sign) maluch->ip = target;
}

void bae(Maluch *const maluch, const uint16_t target) {
    if(!maluch->flags.carry) maluch->ip = target;
}

void bbe(Maluch *const maluch, const uint16_t target) {
    if(maluch->flags.carry || maluch->flags.zero) maluch->ip = target;
}

void baa(Maluch *const maluch, const uint16_t target) {
    if(!maluch->flags.carry && !maluch->flags.zero) maluch->ip = target;
}

void bbb(Maluch *const maluch, const uint16_t target) {
    if(maluch->flags.carry) maluch->ip = target;
}

void bno(Maluch *const maluch, const uint16_t target) {
    if(!maluch->flags.overflow) maluch->ip = target;
}

static void advanceIp(Maluch *const maluch, const bool imm) {
    if(imm) {
        maluch->ip+=2;
    } else {
        maluch->ip++;
    }
}

void step(Maluch *const maluch) {

    const Insn *insn;
    if(maluch->ip < 0x8000) {
        insn = (Insn*) &maluch->rom[maluch->ip];
    } else {
        insn = (Insn*) &maluch->ram[maluch->ip - 0x8000];
    }

    const bool immediate = insn->immediate;
    const uint8_t opcode = insn->opcode;

    const uint16_t usually_dst = insn->dst_reg;
    const uint16_t val = insn->immediate?
        *(uint16_t*)(insn+1) : maluch->reg_file.regs[insn->src_reg];

    advanceIp(maluch, immediate);
    switch(opcode) {
        case 0x0: // invalid
            invalid(maluch);
            break;
        case 0x1: // mov
            mov(maluch, usually_dst, val);
            break;
        case 0x6: // in
            printf("ERROR: opcode in is not supported yet.");
            break;
        case 0x7: // out
            printf("ERROR: opcode out is not supported yet.");
            break;
        case 0x8: // ldw
            ldw(maluch, usually_dst, val);
            break;
        case 0x9: // stw
            stw(maluch, val, usually_dst);
            break;
        case 0xA: // call
            call(maluch, val);
            break;
        case 0xC: // push
            push(maluch, val);
            break;
        case 0xD: // pull
            pull(maluch, usually_dst);
            break;
        case 0xE:
        case 0xF:
            rsrvd(maluch);
            break;

        case 0x2: // FUNCTOR
            switch(insn->functor) {
                case 0: add(maluch, usually_dst, val); break;
                case 1: sub(maluch, usually_dst, val); break;
                case 2: and(maluch, usually_dst, val); break;
                case 3:  or(maluch, usually_dst, val); break;
                case 4: xor(maluch, usually_dst, val); break;
                case 5: not(maluch, usually_dst);      break;
                case 6: lsl(maluch, usually_dst, val); break;
                case 7: lsr(maluch, usually_dst, val); break;
            }
            break;
        case 0x3:
            switch(insn->functor) {
                case 1: cmp(maluch, usually_dst, val); break;
                case 2: test(maluch, usually_dst, val); break;
                default: rsrvd(maluch); break;
            }
            break;
        case 0x4:
            switch(insn->functor) {
                case 0: jmp(maluch, val); break;
                case 1: bee(maluch, val); break;
                case 2: bne(maluch, val); break;
                case 3: bge(maluch, val); break;
                case 4: ble(maluch, val); break;
                case 5: bgg(maluch, val); break;
                case 6: bll(maluch, val); break;
                case 7: boo(maluch, val); break;
            }
            break;
        case 0x5:
            switch(insn->functor) {
                case 0: bbs(maluch, val); break;
                case 1: bss(maluch, val); break;
                case 2: bns(maluch, val); break;
                case 3: bae(maluch, val); break;
                case 4: bbe(maluch, val); break;
                case 5: baa(maluch, val); break;
                case 6: bbb(maluch, val); break;
                case 7: bno(maluch, val); break;
            }
            break;
        case 0xB: // FUNCTOR
            switch(insn->functor) {
                case 0:  ret(maluch); break;
                case 1: iret(maluch); break;
                default: rsrvd(maluch); break;
            }
            break;
        default:
            __builtin_unreachable();
    }
}
