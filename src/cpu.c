#include <stdlib.h>

#include "utils.h"
#include "cpu.h"

#define CPU_NMI_ADDR_LO 0xFFFA
#define CPU_NMI_ADDR_HI 0xFFFB
#define CPU_RES_ADDR_LO 0xFFFC
#define CPU_RES_ADDR_HI 0xFFFD
#define CPU_IRQ_ADDR_LO 0xFFFE
#define CPU_IRQ_ADDR_HI 0xFFFF

#define CPU_STATUS_GET(cpu, flag) (!!((cpu)->p & (flag)))
#define CPU_STATUS_SET(cpu, flag) (cpu)->p |= (uint8_t)(flag)
#define CPU_STATUS_CLEAR(cpu, flag) (cpu)->p &= (uint8_t)(~(flag))
#define CPU_STATUS_UPDATE(cpu, flag, cond) (cpu)->p ^= (uint8_t)((-(!!(cond)) ^ (cpu)->p) & (flag))

typedef struct CpuInstructionEncoding CpuInstructionEncoding;
struct CpuInstructionEncoding
{
    CpuInstructionCode code;
    CpuAddressingMode addrMode;
    int32_t baseCyclesCount;
};

global CpuInstructionEncoding instructionEncodings[256] = {
        {BRK, IMP, 7}, {ORA, IDX, 6}, {JAM, IMP, 0}, {SLO, IDX, 8}, // $00-$03
        {NOP, ZPG, 3}, {ORA, ZPG, 3}, {ASL, ZPG, 5}, {SLO, ZPG, 5}, // $04-$07
        {PHP, IMP, 3}, {ORA, IMM, 2}, {ASL, ACC, 2}, {ANC, IMM, 2}, // $08-$0B
        {NOP, ABS, 4}, {ORA, ABS, 4}, {ASL, ABS, 6}, {SLO, ABS, 6}, // $0C-$0F
        {BPL, REL, 2}, {ORA, IDY, 5}, {JAM, IMP, 0}, {SLO, IDY, 8}, // $10-$13
        {NOP, ZPX, 4}, {ORA, ZPX, 4}, {ASL, ZPX, 6}, {SLO, ZPX, 6}, // $14-$17
        {CLC, IMP, 2}, {ORA, ABY, 4}, {NOP, IMP, 2}, {SLO, ABY, 7}, // $18-$1B
        {NOP, ABX, 4}, {ORA, ABX, 4}, {ASL, ABX, 7}, {SLO, ABX, 7}, // $1C-$1F
        {JSR, ABS, 6}, {AND, IDX, 6}, {JAM, IMP, 0}, {RLA, IDX, 8}, // $20-$23
        {BIT, ZPG, 3}, {AND, ZPG, 3}, {ROL, ZPG, 5}, {RLA, ZPG, 5}, // $24-$27
        {PLP, IMP, 4}, {AND, IMM, 2}, {ROL, ACC, 2}, {ANC, IMM, 2}, // $28-$2B
        {BIT, ABS, 4}, {AND, ABS, 4}, {ROL, ABS, 6}, {RLA, ABS, 6}, // $2C-$2F
        {BMI, REL, 2}, {AND, IDY, 5}, {JAM, IMP, 0}, {RLA, IDY, 8}, // $30-$33
        {NOP, ZPX, 4}, {AND, ZPX, 4}, {ROL, ZPX, 6}, {RLA, ZPX, 6}, // $34-$37
        {SEC, IMP, 2}, {AND, ABY, 4}, {NOP, IMP, 2}, {RLA, ABY, 7}, // $38-$3B
        {NOP, ABX, 4}, {AND, ABX, 4}, {ROL, ABX, 7}, {RLA, ABX, 7}, // $3C-$3F
        {RTI, IMP, 6}, {EOR, IDX, 6}, {JAM, IMP, 0}, {SRE, IDX, 8}, // $40-$43
        {NOP, ZPG, 3}, {EOR, ZPG, 3}, {LSR, ZPG, 5}, {SRE, ZPG, 5}, // $44-$47
        {PHA, IMP, 3}, {EOR, IMM, 2}, {LSR, ACC, 2}, {ALR, IMM, 2}, // $48-$4B
        {JMP, ABS, 3}, {EOR, ABS, 4}, {LSR, ABS, 6}, {SRE, ABS, 6}, // $4C-$4F
        {BVC, REL, 2}, {EOR, IDY, 5}, {JAM, IMP, 0}, {SRE, IDY, 8}, // $50-$53
        {NOP, ZPX, 4}, {EOR, ZPX, 4}, {LSR, ZPX, 6}, {SRE, ZPX, 6}, // $54-$57
        {CLI, IMP, 2}, {EOR, ABY, 4}, {NOP, IMP, 2}, {SRE, ABY, 7}, // $58-$5B
        {NOP, ABX, 4}, {EOR, ABX, 4}, {LSR, ABX, 7}, {SRE, ABX, 7}, // $5C-$5F
        {RTS, IMP, 6}, {ADC, IDX, 6}, {JAM, IMP, 0}, {RRA, IDX, 8}, // $60-$63
        {NOP, ZPG, 3}, {ADC, ZPG, 3}, {ROR, ZPG, 5}, {RRA, ZPG, 5}, // $64-$67
        {PLA, IMP, 4}, {ADC, IMM, 2}, {ROR, ACC, 2}, {ARR, IMM, 2}, // $68-$6B
        {JMP, IDR, 5}, {ADC, ABS, 4}, {ROR, ABS, 6}, {RRA, ABS, 6}, // $6C-$6F
        {BVS, REL, 2}, {ADC, IDY, 5}, {JAM, IMP, 0}, {RRA, IDY, 8}, // $70-$73
        {NOP, ZPX, 4}, {ADC, ZPX, 4}, {ROR, ZPX, 6}, {RRA, ZPX, 6}, // $74-$77
        {SEI, IMP, 2}, {ADC, ABY, 4}, {NOP, IMP, 2}, {RRA, ABY, 7}, // $78-$7B
        {NOP, ABX, 4}, {ADC, ABX, 4}, {ROR, ABX, 7}, {RRA, ABX, 7}, // $7C-$7F
        {NOP, IMM, 2}, {STA, IDX, 6}, {NOP, IMM, 2}, {SAX, IDX, 6}, // $80-$83
        {STY, ZPG, 3}, {STA, ZPG, 3}, {STX, ZPG, 3}, {SAX, ZPG, 3}, // $84-$87
        {DEY, IMP, 2}, {NOP, IMM, 2}, {TXA, IMP, 2}, {ANE, IMM, 2}, // $88-$8B
        {STY, ABS, 4}, {STA, ABS, 4}, {STX, ABS, 4}, {SAX, ABS, 4}, // $8C-$8F
        {BCC, REL, 2}, {STA, IDY, 6}, {JAM, IMP, 0}, {SHA, IDY, 6}, // $90-$93
        {STY, ZPX, 4}, {STA, ZPX, 4}, {STX, ZPY, 4}, {SAX, ZPY, 4}, // $94-$97
        {TYA, IMP, 2}, {STA, ABY, 5}, {TXS, IMP, 2}, {TAS, ABY, 5}, // $98-$9B
        {SHY, ABX, 5}, {STA, ABX, 5}, {SHX, ABY, 5}, {SHA, ABY, 5}, // $9C-$9F
        {LDY, IMM, 2}, {LDA, IDX, 6}, {LDX, IMM, 2}, {LAX, IDX, 6}, // $A0-$A3
        {LDY, ZPG, 3}, {LDA, ZPG, 3}, {LDX, ZPG, 3}, {LAX, ZPG, 3}, // $A4-$A7
        {TAY, IMP, 2}, {LDA, IMM, 2}, {TAX, IMP, 2}, {LXA, IMM, 2}, // $A8-$AB
        {LDY, ABS, 4}, {LDA, ABS, 4}, {LDX, ABS, 4}, {LAX, ABS, 4}, // $AC-$AF
        {BCS, REL, 2}, {LDA, IDY, 5}, {JAM, IMP, 0}, {LAX, IDY, 5}, // $B0-$B3
        {LDY, ZPX, 4}, {LDA, ZPX, 4}, {LDX, ZPY, 4}, {LAX, ZPY, 4}, // $B4-$B7
        {CLV, IMP, 2}, {LDA, ABY, 4}, {TSX, IMP, 2}, {LAS, ABY, 4}, // $B8-$BB
        {LDY, ABX, 4}, {LDA, ABX, 4}, {LDX, ABY, 4}, {LAX, ABY, 4}, // $BC-$BF
        {CPY, IMM, 2}, {CMP, IDX, 6}, {NOP, IMM, 2}, {DCP, IDX, 8}, // $C0-$C3
        {CPY, ZPG, 3}, {CMP, ZPG, 3}, {DEC, ZPG, 5}, {DCP, ZPG, 5}, // $C4-$C7
        {INY, IMP, 2}, {CMP, IMM, 2}, {DEX, IMP, 2}, {SBX, IMM, 2}, // $C8-$CB
        {CPY, ABS, 4}, {CMP, ABS, 4}, {DEC, ABS, 6}, {DCP, ABS, 6}, // $CC-$CF
        {BNE, REL, 2}, {CMP, IDY, 5}, {JAM, IMP, 0}, {DCP, IDY, 8}, // $D0-$D3
        {NOP, ZPX, 4}, {CMP, ZPX, 4}, {DEC, ZPX, 6}, {DCP, ZPX, 6}, // $D4-$D7
        {CLD, IMP, 2}, {CMP, ABY, 4}, {NOP, IMP, 2}, {DCP, ABY, 7}, // $D8-$DB
        {NOP, ABX, 4}, {CMP, ABX, 4}, {DEC, ABX, 7}, {DCP, ABX, 7}, // $DC-$DF
        {CPX, IMM, 2}, {SBC, IDX, 6}, {NOP, IMM, 2}, {ISB, IDX, 8}, // $E0-$E3
        {CPX, ZPG, 3}, {SBC, ZPG, 3}, {INC, ZPG, 5}, {ISB, ZPG, 5}, // $E4-$E7
        {INX, IMP, 2}, {SBC, IMM, 2}, {NOP, IMP, 2}, {USB, IMM, 2}, // $E8-$EB
        {CPX, ABS, 4}, {SBC, ABS, 4}, {INC, ABS, 6}, {ISB, ABS, 6}, // $EC-$EF
        {BEQ, REL, 2}, {SBC, IDY, 5}, {JAM, IMP, 0}, {ISB, IDY, 8}, // $F0-$F3
        {NOP, ZPX, 4}, {SBC, ZPX, 4}, {INC, ZPX, 6}, {ISB, ZPX, 6}, // $F4-$F7
        {SED, IMP, 2}, {SBC, ABY, 4}, {NOP, IMP, 2}, {ISB, ABY, 7}, // $F8-$FB
        {NOP, ABX, 4}, {SBC, ABX, 4}, {INC, ABX, 7}, {ISB, ABX, 7}, // $FC-$FF
};

global const char *cpuInstructionCodeNames[CPU_INSTRUCTION_CODE_COUNT] = {
    // official
    "ADC", "AND", "ASL", "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL", "BRK", "BVC", "BVS", "CLC",
    "CLD", "CLI", "CLV", "CMP", "CPX", "CPY", "DEC", "DEX", "DEY", "EOR", "INC", "INX", "INY", "JMP",
    "JSR", "LDA", "LDX", "LDY", "LSR", "NOP", "ORA", "PHA", "PHP", "PLA", "PLP", "ROL", "ROR", "RTI",
    "RTS", "SBC", "SEC", "SED", "SEI", "STA", "STX", "STY", "TAX", "TAY", "TSX", "TXA", "TXS", "TYA",

    // unofficial
    "ALR", "ANC", "ANE", "ARR", "DCP", "ISB", "JAM", "LAS", "LAX", "LXA", "RLA", "RRA", "SAX", "SBX",
    "SHA", "SHX", "SHY", "SLO", "SRE", "TAS", "SBC",
};

internal int32_t
branch(Cpu *cpu, uint16_t newAddr)
{
    bool pageCrossed = ((cpu->pc & 0xFF00) != (newAddr & 0xFF00));

    cpu->pc = newAddr;

    int32_t cycleCount = pageCrossed ? 2 : 1;
    return cycleCount;
}

internal void
push(Cpu *cpu, uint8_t value)
{
    mmu_cpu_write(cpu->mmu, CPU_STACK_ADDR_OFFSET + cpu->sp, value);
    cpu->sp--;
}

internal void
push16(Cpu *cpu, uint16_t value)
{
    uint8_t hi = (uint8_t)(value >> 8);
    uint8_t lo = (uint8_t)(value & 0xFF);

    push(cpu, hi);
    push(cpu, lo);
}

internal uint8_t
pop(Cpu *cpu)
{
    cpu->sp++;
    uint8_t result = mmu_cpu_read(cpu->mmu, CPU_STACK_ADDR_OFFSET + cpu->sp);
    return result;
}

internal uint16_t
pop16(Cpu *cpu)
{
    uint8_t lo = pop(cpu);
    uint8_t hi = pop(cpu);
    uint16_t result = (uint16_t)((hi << 8) | lo);
    return result;
}

internal uint8_t
handle_interrupt(Cpu *cpu)
{
    push16(cpu, cpu->pc);
    push(cpu, cpu->p | UNUSED);

    CPU_STATUS_SET(cpu, INTERRUPT_INHIBIT);

    uint16_t addr;
    switch (cpu->interrupt) {
        case RES: {
            cpu->sp -= 3;
            addr = CPU_RES_ADDR_LO;
        } break;
        case NMI: {
            addr = CPU_NMI_ADDR_LO;
        } break;
        case IRQ: {
            addr = CPU_IRQ_ADDR_LO;
        } break;
        default: {
            UNREACHABLE();
        }
    }
    cpu->pc = mmu_cpu_read16(cpu->mmu, addr);

    cpu->interrupt = NOI;

    return 7;
}

internal int32_t
handle_opcode(Cpu *cpu, uint8_t opcode)
{
    CpuInstructionEncoding enc = instructionEncodings[opcode];

    int32_t cycleCount = enc.baseCyclesCount;
    uint16_t addr = 0;
    bool pageCrossed = false;
    switch (enc.addrMode) {
        case IMP:
        case ACC: {
        } break;
        case IMM: {
            addr = cpu->pc++;
        } break;
        case ZPG: {
            addr = mmu_cpu_read(cpu->mmu, cpu->pc++);
        } break;
        case ZPX: {
            uint8_t arg = mmu_cpu_read(cpu->mmu, cpu->pc++);
            addr = (arg + cpu->x) & 0xFF;
        } break;
        case ZPY: {
            uint8_t arg = mmu_cpu_read(cpu->mmu, cpu->pc++);
            addr = (arg + cpu->y) & 0xFF;
        } break;
        case REL: {
            uint8_t arg = mmu_cpu_read(cpu->mmu, cpu->pc++);
            int32_t offset = (int32_t)(*((int8_t *)&arg));
            addr = (uint16_t)((int32_t)cpu->pc + offset);
        } break;
        case ABS: {
            addr = mmu_cpu_read16(cpu->mmu, cpu->pc);
            cpu->pc += 2;
        } break;
        case ABX: {
            uint16_t arg = mmu_cpu_read16(cpu->mmu, cpu->pc);
            cpu->pc += 2;
            addr = arg + cpu->x;
            pageCrossed = ((arg & 0xFF00) != (addr & 0xFF00));
        } break;
        case ABY: {
            uint16_t arg = mmu_cpu_read16(cpu->mmu, cpu->pc);
            cpu->pc += 2;
            addr = arg + cpu->y;
            pageCrossed = ((arg & 0xFF00) != (addr & 0xFF00));
        } break;
        case IDR: {
            uint16_t arg = mmu_cpu_read16(cpu->mmu, cpu->pc);
            cpu->pc += 2;

            if ((arg & 0x00FF) == 0x00FF) {
                // HW bug when the page boundary is crossed
                addr = (uint16_t)((mmu_cpu_read(cpu->mmu, arg & 0xFF00) << 8) | mmu_cpu_read(cpu->mmu, arg));
            }
            else {
                addr = mmu_cpu_read16(cpu->mmu, arg);
            }
        } break;
        case IDX: {
            uint8_t arg = mmu_cpu_read(cpu->mmu, cpu->pc++);

            uint16_t loAddr = (arg + cpu->x) & 0xFF;
            uint8_t loAddrAbs = mmu_cpu_read(cpu->mmu, loAddr);

            uint16_t hiAddr = (arg + cpu->x + 1) & 0xFF;
            uint8_t hiAddrAbs = mmu_cpu_read(cpu->mmu, hiAddr);

            addr = (uint16_t)((hiAddrAbs << 8) | loAddrAbs);
        } break;
        case IDY: {
            uint8_t arg = mmu_cpu_read(cpu->mmu, cpu->pc++);

            uint16_t loAddr = arg;
            uint8_t loAddrTmp = mmu_cpu_read(cpu->mmu, loAddr);

            uint16_t hiAddr1 = (arg + 1) & 0xFF;
            uint8_t hiAddrTmp = mmu_cpu_read(cpu->mmu, hiAddr1);

            uint16_t addrTmp = (uint16_t)((hiAddrTmp << 8) | loAddrTmp);
            addr = addrTmp + cpu->y;
            pageCrossed = ((addrTmp & 0xFF00) != (addr & 0xFF00));
        } break;
        default: {
            UNREACHABLE();
        }
    }

    switch (enc.code) {
        // official
        case ADC: {
            uint16_t a = cpu->a;
            uint16_t m = mmu_cpu_read(cpu->mmu, addr);
            uint16_t r = (uint16_t)(a + m + (uint16_t)CPU_STATUS_GET(cpu, CARRY));

            CPU_STATUS_UPDATE(cpu, CARRY, r & 0xFF00);
            CPU_STATUS_UPDATE(cpu, ZERO, !(r & 0xFF));
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);
            //   A + M = R
            //   +   +   +
            //   +   +   -  <- overflow
            //   +   -   +
            //   +   -   -
            //   -   +   +
            //   -   +   -
            //   -   -   +  <- overflow
            //   -   -   -
            CPU_STATUS_UPDATE(cpu, OVERFLOW, (a ^ r) & (m ^ r) & 0x80);

            cpu->a = (uint8_t)(r & 0xFF);

            if (pageCrossed) {
                cycleCount++;
            }
        } break;
        case AND: {
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = cpu->a & m;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->a = r;

            if (pageCrossed) {
                cycleCount++;
            }
        } break;
        case ASL: {
            if (enc.addrMode == ACC) {
                uint16_t a = cpu->a;
                uint16_t r = a << 1;

                CPU_STATUS_UPDATE(cpu, CARRY, r & 0xFF00);
                CPU_STATUS_UPDATE(cpu, ZERO, !(r & 0xFF));
                CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

                cpu->a = (uint8_t)(r & 0xFF);
            }
            else {
                uint16_t m = mmu_cpu_read(cpu->mmu, addr);
                uint16_t r = m << 1;

                CPU_STATUS_UPDATE(cpu, CARRY, r & 0xFF00);
                CPU_STATUS_UPDATE(cpu, ZERO, !(r & 0xFF));
                CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

                mmu_cpu_write(cpu->mmu, addr, (uint8_t)(r & 0xFF));
            }
        } break;
        case BCC: {
            if (!CPU_STATUS_GET(cpu, CARRY)) {
                cycleCount += branch(cpu, addr);
            }
        } break;
        case BCS: {
            if (CPU_STATUS_GET(cpu, CARRY)) {
                cycleCount += branch(cpu, addr);
            }
        } break;
        case BEQ: {
            if (CPU_STATUS_GET(cpu, ZERO)) {
                cycleCount += branch(cpu, addr);
            }
        } break;
        case BIT: {
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = cpu->a & m;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, OVERFLOW, m & OVERFLOW);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, m & NEGATIVE);
        } break;
        case BMI: {
            if (CPU_STATUS_GET(cpu, NEGATIVE)) {
                cycleCount += branch(cpu, addr);
            }
        } break;
        case BNE: {
            if (!CPU_STATUS_GET(cpu, ZERO)) {
                cycleCount += branch(cpu, addr);
            }
        } break;
        case BPL: {
            if (!CPU_STATUS_GET(cpu, NEGATIVE)) {
                cycleCount += branch(cpu, addr);
            }
        } break;
        case BRK: {
            push16(cpu,  cpu->pc + 1);
            push(cpu, cpu->p | BREAK | UNUSED);

            CPU_STATUS_SET(cpu, INTERRUPT_INHIBIT);

            cpu->pc = mmu_cpu_read16(cpu->mmu, CPU_IRQ_ADDR_LO);
        } break;
        case BVC: {
            if (!CPU_STATUS_GET(cpu, OVERFLOW)) {
                cycleCount += branch(cpu, addr);
            }
        } break;
        case BVS: {
            if (CPU_STATUS_GET(cpu, OVERFLOW)) {
                cycleCount += branch(cpu, addr);
            }
        } break;
        case CLC: {
            CPU_STATUS_CLEAR(cpu, CARRY);
        } break;
        case CLD: {
            CPU_STATUS_CLEAR(cpu, DECIMAL_MODE);
        } break;
        case CLI: {
            CPU_STATUS_CLEAR(cpu, INTERRUPT_INHIBIT);
        } break;
        case CLV: {
            CPU_STATUS_CLEAR(cpu, OVERFLOW);
        } break;
        case CMP: {
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = cpu->a - m;

            CPU_STATUS_UPDATE(cpu, CARRY, cpu->a >= m);
            CPU_STATUS_UPDATE(cpu, ZERO, cpu->a == m);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            if (pageCrossed) {
                cycleCount++;
            }
        } break;
        case CPX: {
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = cpu->x - m;

            CPU_STATUS_UPDATE(cpu, CARRY, cpu->x >= m);
            CPU_STATUS_UPDATE(cpu, ZERO, cpu->x == m);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);
        } break;
        case CPY: {
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = cpu->y - m;

            CPU_STATUS_UPDATE(cpu, CARRY, cpu->y >= m);
            CPU_STATUS_UPDATE(cpu, ZERO, cpu->y == m);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);
        } break;
        case DEC: {
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = (uint8_t)(m - 1);

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            mmu_cpu_write(cpu->mmu, addr, r);
        } break;
        case DEX: {
            uint8_t r = cpu->x - 1;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->x = r;
        } break;
        case DEY: {
            uint8_t r = cpu->y - 1;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->y = r;
        } break;
        case EOR: {
            uint8_t a = cpu->a;
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = a ^ m;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->a = r;

            if (pageCrossed) {
                cycleCount++;
            }
        } break;
        case INC: {
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = m + 1;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            mmu_cpu_write(cpu->mmu, addr, r);
        } break;
        case INX: {
            uint8_t r = cpu->x + 1;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->x = r;
        } break;
        case INY: {
            uint8_t r = cpu->y + 1;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->y = r;
        } break;
        case JMP: {
            cpu->pc = addr;
        } break;
        case JSR: {
            push16(cpu, cpu->pc - 1);
            cpu->pc = addr;
        } break;
        case LDA: {
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);

            CPU_STATUS_UPDATE(cpu, ZERO, !m);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, m & 0x80);

            cpu->a = m;

            if (pageCrossed) {
                cycleCount++;
            }
        } break;
        case LDX: {
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);

            CPU_STATUS_UPDATE(cpu, ZERO, !m);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, m & 0x80);

            cpu->x = m;

            if (pageCrossed) {
                cycleCount++;
            }
        } break;
        case LDY: {
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);

            CPU_STATUS_UPDATE(cpu, ZERO, !m);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, m & 0x80);

            cpu->y = m;

            if (pageCrossed) {
                cycleCount++;
            }
        } break;
        case LSR: {
            if (enc.addrMode == ACC) {
                uint8_t a = cpu->a;
                uint8_t r = a >> 1;

                CPU_STATUS_UPDATE(cpu, CARRY, a & 1);
                CPU_STATUS_UPDATE(cpu, ZERO, !r);
                CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

                cpu->a = r;
            }
            else {
                uint8_t m = mmu_cpu_read(cpu->mmu, addr);
                uint8_t r = m >> 1;

                CPU_STATUS_UPDATE(cpu, CARRY, m & 1);
                CPU_STATUS_UPDATE(cpu, ZERO, !r);
                CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

                mmu_cpu_write(cpu->mmu, addr, r);
            }
        } break;
        case NOP: {
            if (pageCrossed) {
                cycleCount++;
            }
        } break;
        case ORA: {
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = cpu->a | m;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->a = r;

            if (pageCrossed) {
                cycleCount++;
            }
        } break;
        case PHA: {
            push(cpu, cpu->a);
        } break;
        case PHP: {
            push(cpu, cpu->p | BREAK | UNUSED);
        } break;
        case PLA: {
            uint8_t r = pop(cpu);

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->a = r;
        } break;
        case PLP: {
            cpu->p = pop(cpu) | UNUSED;
            CPU_STATUS_CLEAR(cpu, BREAK);
        } break;
        case ROL: {
            if (enc.addrMode == ACC) {
                uint8_t a = cpu->a;
                uint8_t r = (uint8_t)((a << 1) | (uint8_t)CPU_STATUS_GET(cpu, CARRY));

                CPU_STATUS_UPDATE(cpu, CARRY, a & 0x80);
                CPU_STATUS_UPDATE(cpu, ZERO, !r);
                CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

                cpu->a = r;
            }
            else {
                uint8_t m = mmu_cpu_read(cpu->mmu, addr);
                uint8_t r = (uint8_t)((m << 1) | (uint8_t)CPU_STATUS_GET(cpu, CARRY));

                CPU_STATUS_UPDATE(cpu, CARRY, m & 0x80);
                CPU_STATUS_UPDATE(cpu, ZERO, !r);
                CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

                mmu_cpu_write(cpu->mmu, addr, r);
            }
        } break;
        case ROR: {
            if (enc.addrMode == ACC) {
                uint8_t a = cpu->a;
                uint8_t r = (uint8_t)((a >> 1) | ((uint8_t)CPU_STATUS_GET(cpu, CARRY) << 7));

                CPU_STATUS_UPDATE(cpu, CARRY, a & 1);
                CPU_STATUS_UPDATE(cpu, ZERO, !r);
                CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

                cpu->a = r;
            }
            else {
                uint8_t m = mmu_cpu_read(cpu->mmu, addr);
                uint8_t r = (uint8_t)((m >> 1) | ((uint8_t)CPU_STATUS_GET(cpu, CARRY) << 7));

                CPU_STATUS_UPDATE(cpu, CARRY, m & 1);
                CPU_STATUS_UPDATE(cpu, ZERO, !r);
                CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

                mmu_cpu_write(cpu->mmu, addr, r);
            }
        } break;
        case RTI: {
            cpu->p = pop(cpu) | UNUSED;
            cpu->pc = pop16(cpu);
            CPU_STATUS_CLEAR(cpu, BREAK);
        } break;
        case RTS: {
            cpu->pc = pop16(cpu) + 1;
        } break;
        case SBC: {
            uint16_t a = cpu->a;
            uint16_t m = mmu_cpu_read(cpu->mmu, addr);
            uint16_t r = (uint16_t)(a - m - (uint16_t)!CPU_STATUS_GET(cpu, CARRY));

            CPU_STATUS_UPDATE(cpu, CARRY, !(r & 0xFF00));
            CPU_STATUS_UPDATE(cpu, ZERO, !(r & 0xFF));
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);
            //   A - M = R
            //   +   +   +
            //   +   +   -
            //   +   -   +
            //   +   -   -  <- overflow
            //   -   +   +  <- overflow
            //   -   +   -
            //   -   -   +
            //   -   -   -
            CPU_STATUS_UPDATE(cpu, OVERFLOW, (a ^ m) & (a ^ r) & 0x80);

            cpu->a = (uint8_t)(r & 0xFF);

            if (pageCrossed) {
                cycleCount++;
            }
        } break;
        case SEC: {
            CPU_STATUS_SET(cpu, CARRY);
        } break;
        case SED: {
            CPU_STATUS_SET(cpu, DECIMAL_MODE);
        } break;
        case SEI: {
            CPU_STATUS_SET(cpu, INTERRUPT_INHIBIT);
        } break;
        case STA: {
            mmu_cpu_write(cpu->mmu, addr, cpu->a);
        } break;
        case STX: {
            mmu_cpu_write(cpu->mmu, addr, cpu->x);
        } break;
        case STY: {
            mmu_cpu_write(cpu->mmu, addr, cpu->y);
        } break;
        case TAX: {
            uint8_t r = cpu->a;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->x = r;
        } break;
        case TAY: {
            uint8_t r = cpu->a;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->y = r;
        } break;
        case TSX: {
            uint8_t r = cpu->sp;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->x = r;
        } break;
        case TXA: {
            uint8_t r = cpu->x;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->a = r;
        } break;
        case TXS: {
            uint8_t r = cpu->x;
            cpu->sp = r;
        } break;
        case TYA: {
            uint8_t r = cpu->y;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->a = r;
        } break;

        // unofficial
        case ALR: {
            uint8_t a = cpu->a;
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = (uint8_t)(a & m);

            CPU_STATUS_UPDATE(cpu, CARRY, r & 1);

            r >>= 1;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_CLEAR(cpu, NEGATIVE);

            cpu->a = r;
        } break;
        case ANC: {
            uint8_t a = cpu->a;
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = (uint8_t)(a & m);

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, CARRY | NEGATIVE, r & NEGATIVE);

            cpu->a = r;
        } break;
        case ANE: {
            uint8_t a = cpu->a;
            uint8_t v = 0xFF; // could be 0x00, 0xEE, 0xEF, 0xFE or 0xFF
            uint8_t x = cpu->x;
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = (uint8_t)((a | v) & x & m);

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->a = r;
        } break;
        case ARR: {
            uint8_t a = cpu->a;
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = ((uint8_t)((a & m) >> 1)) | ((uint8_t)CPU_STATUS_GET(cpu, CARRY) << 7);

            CPU_STATUS_UPDATE(cpu, CARRY, r & 0x40);
            CPU_STATUS_UPDATE(cpu, OVERFLOW, ((r >> 6) ^ (r >> 5)) & 1);
            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->a = r;
        } break;
        case DCP: {
            uint8_t a = cpu->a;
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = (uint8_t)(m - 1);
            uint8_t d = (uint8_t)(a - r);

            CPU_STATUS_UPDATE(cpu, CARRY, a >= r);
            CPU_STATUS_UPDATE(cpu, ZERO, a == r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, d & NEGATIVE);

            mmu_cpu_write(cpu->mmu, addr, r);
        } break;
        case ISB: {
            uint16_t a = cpu->a;
            uint16_t m = mmu_cpu_read(cpu->mmu, addr);
            uint16_t m1 = (uint16_t)((m + 1) & 0xFF);
            uint16_t r = (uint16_t)(a - m1 - (uint16_t)!CPU_STATUS_GET(cpu, CARRY));

            CPU_STATUS_UPDATE(cpu, CARRY, !(r & 0xFF00));
            CPU_STATUS_UPDATE(cpu, ZERO, !(r & 0xFF));
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);
            //   A - M = R
            //   +   +   +
            //   +   +   -
            //   +   -   +
            //   +   -   -  <- overflow
            //   -   +   +  <- overflow
            //   -   +   -
            //   -   -   +
            //   -   -   -
            CPU_STATUS_UPDATE(cpu, OVERFLOW, (a ^ m1) & (a ^ r) & 0x80);

            mmu_cpu_write(cpu->mmu, addr, (uint8_t)(m1 & 0xFF));
            cpu->a = (uint8_t)(r & 0xFF);
        } break;
        case JAM: {
            // ignore
        } break;
        case LAS: {
            uint8_t sp = cpu->sp;
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = sp & m;

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->sp = cpu->a = cpu->x = r;

            if (pageCrossed) {
                cycleCount++;
            }
        } break;
        case LAX: {
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);

            CPU_STATUS_UPDATE(cpu, ZERO, !m);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, m & NEGATIVE);

            cpu->a = cpu->x = m;

            if (pageCrossed) {
                cycleCount++;
            }
        } break;
        case LXA: {
            uint8_t a = cpu->a;
            uint8_t v = 0xFF; // could be 0x00, 0xEE, 0xEF, 0xFE or 0xFF
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = (uint8_t)((a | v) & m);

            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->a = cpu->x = r;

            if (pageCrossed) {
                cycleCount++;
            }
        } break;
        case RLA: {
            uint8_t a = cpu->a;
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t m1 = (uint8_t)((m << 1) | (uint8_t)CPU_STATUS_GET(cpu, CARRY));
            uint8_t r = (uint8_t)(a & m1);

            CPU_STATUS_UPDATE(cpu, CARRY, m & 0x80);
            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            mmu_cpu_write(cpu->mmu, addr, m1);
            cpu->a = r;
        } break;
        case RRA: {
            uint16_t a = cpu->a;
            uint16_t m = mmu_cpu_read(cpu->mmu, addr);
            uint16_t m1 = (uint16_t)((m >> 1) | ((uint16_t)CPU_STATUS_GET(cpu, CARRY) << 7));

            CPU_STATUS_UPDATE(cpu, CARRY, m & 1);

            uint16_t r = (uint16_t)(a + m1 + (uint16_t)CPU_STATUS_GET(cpu, CARRY));

            CPU_STATUS_UPDATE(cpu, CARRY, r & 0xFF00);
            CPU_STATUS_UPDATE(cpu, ZERO, !(r & 0xFF));
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);
            //   A + M = R
            //   +   +   +
            //   +   +   -  <- overflow
            //   +   -   +
            //   +   -   -
            //   -   +   +
            //   -   +   -
            //   -   -   +  <- overflow
            //   -   -   -
            CPU_STATUS_UPDATE(cpu, OVERFLOW, (a ^ r) & (m1 ^ r) & 0x80);

            mmu_cpu_write(cpu->mmu, addr, (uint8_t)(m1 & 0xFF));
            cpu->a = (uint8_t)(r & 0xFF);
        } break;
        case SAX: {
            uint8_t a = cpu->a;
            uint8_t x = cpu->x;
            uint8_t r = (uint8_t)(a & x);
            mmu_cpu_write(cpu->mmu, addr, r);
        } break;
        case SBX: {
            uint8_t a = cpu->a;
            uint8_t x = cpu->x;
            uint8_t b = (uint8_t)(a & x);
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t r = b - m;

            CPU_STATUS_UPDATE(cpu, CARRY, b >= m);
            CPU_STATUS_UPDATE(cpu, ZERO, b == m);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            cpu->x = r;
        } break;
        case SHA: {
            uint8_t a = cpu->a;
            uint8_t x = cpu->x;
            uint8_t v = (uint8_t)(((addr & 0xFF00) >> 8) + 1);
            uint8_t r = (uint8_t)(a & x & v);

            mmu_cpu_write(cpu->mmu, addr, r);
        } break;
        case SHX: {
            uint8_t x = cpu->x;
            uint8_t v = (uint8_t)(((addr & 0xFF00) >> 8) + 1);
            uint8_t r = (uint8_t)(x & v);

            mmu_cpu_write(cpu->mmu, addr, r);
        } break;
        case SHY: {
            uint8_t y = cpu->y;
            uint8_t v = (uint8_t)(((addr & 0xFF00) >> 8) + 1);
            uint8_t r = (uint8_t)(y & v);

            mmu_cpu_write(cpu->mmu, addr, r);
        } break;
        case SLO: {
            uint8_t a = cpu->a;
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t m1 = (uint8_t)(m << 1);
            uint8_t r = (uint8_t)(a | m1);

            CPU_STATUS_UPDATE(cpu, CARRY, m & 0x80);
            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            mmu_cpu_write(cpu->mmu, addr, m1);
            cpu->a = r;
        } break;
        case SRE: {
            uint8_t a = cpu->a;
            uint8_t m = mmu_cpu_read(cpu->mmu, addr);
            uint8_t m1 = (uint8_t)(m >> 1);
            uint8_t r = (uint8_t)(a ^ m1);

            CPU_STATUS_UPDATE(cpu, CARRY, m & 1);
            CPU_STATUS_UPDATE(cpu, ZERO, !r);
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);

            mmu_cpu_write(cpu->mmu, addr, m1);
            cpu->a = r;
        } break;
        case TAS: {
            uint8_t a = cpu->a;
            uint8_t x = cpu->x;
            uint8_t sp = (uint8_t)(a & x);
            uint8_t v = (uint8_t)(((addr & 0xFF00) >> 8) + 1);
            uint8_t r = (uint8_t)(sp & v);

            cpu->sp = sp;
            mmu_cpu_write(cpu->mmu, addr, r);
        } break;
        case USB: {
            uint16_t a = cpu->a;
            uint16_t m = mmu_cpu_read(cpu->mmu, addr);
            uint16_t r = (uint16_t)(a - m - (uint16_t)!CPU_STATUS_GET(cpu, CARRY));

            CPU_STATUS_UPDATE(cpu, CARRY, !(r & 0xFF00));
            CPU_STATUS_UPDATE(cpu, ZERO, !(r & 0xFF));
            CPU_STATUS_UPDATE(cpu, NEGATIVE, r & NEGATIVE);
            //   A - M = R
            //   +   +   +
            //   +   +   -
            //   +   -   +
            //   +   -   -  <- overflow
            //   -   +   +  <- overflow
            //   -   +   -
            //   -   -   +
            //   -   -   -
            CPU_STATUS_UPDATE(cpu, OVERFLOW, (a ^ m) & (a ^ r) & 0x80);

            cpu->a = (uint8_t)(r & 0xFF);
        } break;
        default: {
            UNREACHABLE();
        }
    }

    return cycleCount;
}

bool
cpu_init(Cpu *cpu)
{
    cpu->a = 0;
    cpu->x = 0;
    cpu->y = 0;
    cpu->p = 0 | INTERRUPT_INHIBIT | UNUSED;
    cpu->interrupt = NOI;
    cpu->cyclesCount = 0;
    cpu->pendingCycleCount = 0;

    cpu->pc = mmu_cpu_read16(cpu->mmu, CPU_RES_ADDR_LO);
    cpu->sp = 0xFD; // As if the stack was initialized to $00, and then a RESET was performed (e.g. $00 - 3 = $FD).

    return true;
}

void
cpu_tick(Cpu *cpu)
{
    if (cpu->pendingCycleCount == 0) {
        if (cpu->interrupt != NOI) {
            cpu->pendingCycleCount = handle_interrupt(cpu);
        }
        else {
            uint8_t opcode = mmu_cpu_read(cpu->mmu, cpu->pc++);
            cpu->pendingCycleCount = handle_opcode(cpu, opcode);
        }
        cpu->cyclesCount += cpu->pendingCycleCount;
    }
    cpu->pendingCycleCount--;
}

void
cpu_interrupt(Cpu *cpu, CpuInterruptType interrupt)
{
    if ((interrupt != IRQ) || !CPU_STATUS_GET(cpu, INTERRUPT_INHIBIT)) {
        cpu->interrupt = interrupt;
    }
}

Str8
cpu_sprint(Arena *arena, Cpu *cpu)
{
    uint8_t opcode = mmu_cpu_read(cpu->mmu, cpu->pc);
    CpuInstructionEncoding enc = instructionEncodings[opcode];
    const char *codeName = cpuInstructionCodeNames[enc.code];

    Str8List list = {};

    str8_list_appendf(arena, &list, "%04X", cpu->pc);

    switch (enc.addrMode) {
        case IMP: {
            str8_list_appendf(arena, &list, "%02X      ", opcode);
            str8_list_appendf(arena, &list, "%s        ", codeName);
        } break;
        case ACC: {
            str8_list_appendf(arena, &list, "%02X      ", opcode);
            str8_list_appendf(arena, &list, "%s A      ", codeName);
        } break;
        case IMM: {
            uint8_t arg = mmu_cpu_read(cpu->mmu, cpu->pc + 1);
            str8_list_appendf(arena, &list, "%02X %02X   ", opcode, arg);
            str8_list_appendf(arena, &list, "%s #$%02X   ", codeName, arg);
        } break;
        case ZPG: {
            uint8_t arg = mmu_cpu_read(cpu->mmu, cpu->pc + 1);
            str8_list_appendf(arena, &list, "%02X %02X   ", opcode, arg);
            str8_list_appendf(arena, &list, "%s $%02X    ", codeName, arg);
        } break;
        case ZPX: {
            uint8_t arg = mmu_cpu_read(cpu->mmu, cpu->pc + 1);
            str8_list_appendf(arena, &list, "%02X %02X   ", opcode, arg);
            str8_list_appendf(arena, &list, "%s $%02X,X  ", codeName, arg);
        } break;
        case ZPY: {
            uint8_t arg = mmu_cpu_read(cpu->mmu, cpu->pc + 1);
            str8_list_appendf(arena, &list, "%02X %02X   ", opcode, arg);
            str8_list_appendf(arena, &list, "%s $%02X,Y  ", codeName, arg);
        } break;
        case REL: {
            uint8_t rawOffset = mmu_cpu_read(cpu->mmu, cpu->pc + 1);
            int32_t offset = (int32_t)(*((int8_t *)&rawOffset));
            uint16_t arg = (uint16_t)((int32_t)(cpu->pc + 2) + offset);
            str8_list_appendf(arena, &list, "%02X %02X   ", opcode, rawOffset);
            str8_list_appendf(arena, &list, "%s $%04X  ", codeName, arg);
        } break;
        case ABS: {
            uint8_t lo = mmu_cpu_read(cpu->mmu, cpu->pc + 1);
            uint8_t hi = mmu_cpu_read(cpu->mmu, cpu->pc + 2);
            uint16_t arg = (uint16_t)((hi << 8) | lo);
            str8_list_appendf(arena, &list, "%02X %02X %02X", opcode, lo, hi);
            str8_list_appendf(arena, &list, "%s $%04X  ", codeName, arg);
        } break;
        case ABX: {
            uint8_t lo = mmu_cpu_read(cpu->mmu, cpu->pc + 1);
            uint8_t hi = mmu_cpu_read(cpu->mmu, cpu->pc + 2);
            uint16_t arg = (uint16_t)((hi << 8) | lo);
            str8_list_appendf(arena, &list, "%02X %02X %02X", opcode, lo, hi);
            str8_list_appendf(arena, &list, "%s $%04X,X", codeName, arg);
        } break;
        case ABY: {
            uint8_t lo = mmu_cpu_read(cpu->mmu, cpu->pc + 1);
            uint8_t hi = mmu_cpu_read(cpu->mmu, cpu->pc + 2);
            uint16_t arg = (uint16_t)((hi << 8) | lo);
            str8_list_appendf(arena, &list, "%02X %02X %02X", opcode, lo, hi);
            str8_list_appendf(arena, &list, "%s $%04X,Y", codeName, arg);
        } break;
        case IDR: {
            uint8_t lo = mmu_cpu_read(cpu->mmu, cpu->pc + 1);
            uint8_t hi = mmu_cpu_read(cpu->mmu, cpu->pc + 2);
            uint16_t arg = (uint16_t)((hi << 8) | lo);
            str8_list_appendf(arena, &list, "%02X %02X %02X", opcode, lo, hi);
            str8_list_appendf(arena, &list, "%s ($%04X)", codeName, arg);
        } break;
        case IDX: {
            uint8_t arg = mmu_cpu_read(cpu->mmu, cpu->pc + 1);
            str8_list_appendf(arena, &list, "%02X %02X   ", opcode, arg);
            str8_list_appendf(arena, &list, "%s ($%02X,X)", codeName, arg);
        } break;
        case IDY: {
            uint8_t arg = mmu_cpu_read(cpu->mmu, cpu->pc + 1);
            str8_list_appendf(arena, &list, "%02X %02X   ", opcode, arg);
            str8_list_appendf(arena, &list, "%s ($%02X),Y", codeName, arg);
        } break;
        default: {
            UNREACHABLE();
        }
    }

    str8_list_appendf(
        arena,
        &list,
        "A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%lu",
        cpu->a,
        cpu->x,
        cpu->y,
        cpu->p,
        cpu->sp,
        cpu->cyclesCount
    );

    return str8_list_join(arena, list, '\t');
}
