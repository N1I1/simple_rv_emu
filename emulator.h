#ifndef EMULATOR_H
#define EMULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include "state.h"

#define MEMORY_SIZE 65536
#define PC_START 0
#define NUM_INSTRS 100
#define LOG_FILE "build/ref.log"
#define MAX_EXEC_INSTRS 1000

#define RS1 ((emu)->state.regs[(instr).rs1])
#define RS2 ((emu)->state.regs[(instr).rs2])
#define RD  ((emu)->state.regs[(instr).rd])
#define PC  ((emu)->state.pc)
#define DNPC ((emu)->state.dnpc)

typedef struct {
    State state;
    uint8_t memory[MEMORY_SIZE];
    bool log_enabled;
    FILE *log_file;
} Emulator;

typedef struct {
    uint32_t opcode;
    uint32_t rd;
    uint32_t funct3;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t funct7;
    int32_t imm_i; // Immediate for I-type instructions
    int32_t imm_b; // Immediate for B-type instructions
    int32_t imm_j; // Immediate for JAL instructions
    int32_t imm_u; // Immediate for U-type instructions
    int32_t imm_jalr; // Immediate for JALR instructions
    int32_t imm_s; // Immediate for store instructions
    int32_t imm_l; // Immediate for load instructions
    // ...other immediate types if needed...
} Instruction;

void init_emulator(Emulator *emu, const char *hex_file, uint64_t start_pc, size_t num_instrs, const char *log_file_name);
bool fetch_and_execute(Emulator *emu);
void log_state(const Emulator *emu, const uint32_t raw_instr);
uint32_t fetch(Emulator *emu);
bool execute(Emulator *emu, Instruction instr);
void execute_r_type(Emulator *emu, Instruction instr);
void execute_i_type(Emulator *emu, Instruction instr);
void execute_b_type(Emulator *emu, Instruction instr);
void execute_jal(Emulator *emu, Instruction instr);
void execute_jalr(Emulator *emu, Instruction instr);
void execute_auipc(Emulator *emu, Instruction instr);
void execute_lui(Emulator *emu, Instruction instr);
void execute_load(Emulator *emu, Instruction instr);
void execute_store(Emulator *emu, Instruction instr);
void execute_addw(Emulator *emu, Instruction instr);
void execute_csr(Emulator *emu, Instruction instr);
void execute_ecall(Emulator *emu);
void execute_ebreak(Emulator *emu);
void execute_mret(Emulator *emu);
// ...other function declarations for different instruction types...

#endif // EMULATOR_H