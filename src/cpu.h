#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#include "utils.h"
#include "mmu.h"
#include "str8.h"

#define CPU_STACK_ADDR_OFFSET 0x0100

typedef int32_t CpuStatusFlag;
enum CpuStatusFlag
{
    CARRY             = (1 << 0),
    ZERO              = (1 << 1),
    INTERRUPT_INHIBIT = (1 << 2),
    DECIMAL_MODE      = (1 << 3),
    BREAK             = (1 << 4),
    UNUSED            = (1 << 5),
    OVERFLOW          = (1 << 6),
    NEGATIVE          = (1 << 7),
};

typedef int32_t CpuInterruptType;
enum CpuInterruptType
{
    NOI, // No Interrupt
    RES, // Reset
    NMI, // Non-Maskable Interrupt
    IRQ, // Interrupt Request
};

typedef struct Cpu Cpu;
struct Cpu
{
    Mmu *mmu;

    uint16_t pc; // Program Counter
    uint8_t a;   // ACC
    uint8_t x;   // Register X
    uint8_t y;   // Register Y
    uint8_t p;   // Status register
    uint8_t sp;  // Stack Pointer

    CpuInterruptType interrupt;
    uint64_t cyclesCount;
    uint64_t pendingCycleCount;
};

typedef int32_t CpuInstructionCode;
enum CpuInstructionCode
{
    // official
    ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS, CLC,
    CLD, CLI, CLV, CMP, CPX, CPY, DEC, DEX, DEY, EOR, INC, INX, INY, JMP,
    JSR, LDA, LDX, LDY, LSR, NOP, ORA, PHA, PHP, PLA, PLP, ROL, ROR, RTI,
    RTS, SBC, SEC, SED, SEI, STA, STX, STY, TAX, TAY, TSX, TXA, TXS, TYA,

    // unofficial
    ALR, ANC, ANE, ARR, DCP, ISB, JAM, LAS, LAX, LXA, RLA, RRA, SAX, SBX,
    SHA, SHX, SHY, SLO, SRE, TAS, USB,

    CPU_INSTRUCTION_CODE_COUNT
};

typedef int32_t CpuAddressingMode;
enum CpuAddressingMode
{
    IMP,
    ACC, // rA
    IMM, // #$BB
    ZPG, // $LL
    ZPX, // $LL,X
    ZPY, // $LL,Y
    REL, // $BB
    ABS, // $LLHH
    ABX, // $LLHH,X
    ABY, // $LLHH,Y
    IDR, // ($LLHH)
    IDX, // ($LL,X)
    IDY, // ($LL),Y

    CPU_ADDRESSING_MODE_COUNT
};

bool cpu_init(Cpu *cpu);
void cpu_tick(Cpu *cpu);
void cpu_interrupt(Cpu *cpu, CpuInterruptType type);
Str8 cpu_sprint(Arena *arena, Cpu *cpu);

#endif //CPU_H
