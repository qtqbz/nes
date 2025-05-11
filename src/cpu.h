#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#include "bus.h"

#define CPU_STACK_ADDR_OFFSET 0x0100

typedef struct CpuStatus CpuStatus;
struct CpuStatus
{
    bool carry;
    bool zero;
    bool interruptInhibit;
    //bool decimalMode;
    bool breakCommand;
    //bool unused;
    bool overflow;
    bool negative;
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
    Bus *bus;

    uint16_t pc; // Program Counter
    uint8_t a;   // ACC
    uint8_t x;   // Register X
    uint8_t y;   // Register Y
    uint8_t sp;  // Stack Pointer

    CpuStatus status;
    CpuInterruptType interrupt;
    uint64_t cyclesCount;
};

typedef int32_t CpuInstructionCode;
enum CpuInstructionCode
{
    // legal
    ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS, CLC,
    CLD, CLI, CLV, CMP, CPX, CPY, DEC, DEX, DEY, EOR, INC, INX, INY, JMP,
    JSR, LDA, LDX, LDY, LSR, NOP, ORA, PHA, PHP, PLA, PLP, ROL, ROR, RTI,
    RTS, SBC, SEC, SED, SEI, STA, STX, STY, TAX, TAY, TSX, TXA, TXS, TYA,

    // illegal
    ALR, ANC, ANE, ARR, DCP, ISC, JAM, LAS, LAX, LXA, RLA, RRA, SAX, SBX,
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

void cpu_reset(Cpu *cpu);

#endif //CPU_H
