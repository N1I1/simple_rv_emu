#ifndef STATE_H
#define STATE_H

#include <stdint.h>

#define NUM_REGS 32

#define CSR_MSTATUS 0x300
#define CSR_MISA 0x301
#define CSR_MIE 0x304
#define CSR_MTVEC 0x305
#define CSR_MSCRATCH 0x340
#define CSR_MEPC 0x341
#define CSR_MCAUSE 0x342
#define CSR_MTVAL 0x343
#define CSR_MIP 0x344

typedef struct {
    uint64_t regs[NUM_REGS];
    uint64_t pc;
    uint64_t dnpc; // Next PC
    uint32_t csrs[4096];
} State;

#endif // STATE_H