#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emulator.h"
#include "state.h"

void init_emulator(Emulator *emu, const char *hex_file, uint64_t start_pc, size_t num_instrs, const char *log_file_name) {
    memset(emu, 0, sizeof(Emulator));
    emu->log_enabled = true;
    emu->state.pc = start_pc;
    emu->state.dnpc = start_pc + 4;

    FILE *file = fopen(hex_file, "r");
    if (!file) {
        perror("Failed to open hex file");
        exit(1);
    }

    char line[256];
    size_t count = 0;
    while (fgets(line, sizeof(line), file) && count < num_instrs) {
        if (line[0] == '#') {
            continue; // Ignore comment lines
        }
        uint32_t value;
        if (sscanf(line, "%x", &value) == 1) {
            if (emu->state.pc + count * 4 < MEMORY_SIZE) {
                memcpy(&emu->memory[emu->state.pc + count * 4], &value, sizeof(value));
                count++;
            }
        }
    }

    fclose(file);

    if (log_file_name == NULL) {
        log_file_name = LOG_FILE;
    }
    emu->log_file = fopen(log_file_name, "w");
    if (!emu->log_file) {
        perror("Failed to open log file");
        exit(1);
    }
}

uint32_t fetch(Emulator *emu) {
    uint32_t raw_instr;
    memcpy(&raw_instr, &emu->memory[PC], sizeof(raw_instr));
    return raw_instr;
}

bool execute(Emulator *emu, Instruction instr) {
    switch (instr.opcode) {
        case 0x33: // R-type instructions
            execute_r_type(emu, instr);
            break;
        case 0x13: // I-type instructions
            execute_i_type(emu, instr);
            break;
        case 0x63: // B-type instructions
            execute_b_type(emu, instr);
            break;
        case 0x6F: // JAL
            execute_jal(emu, instr);
            break;
        case 0x67: // JALR
            execute_jalr(emu, instr);
            break;
        case 0x17: // AUIPC
            execute_auipc(emu, instr);
            break;
        case 0x37: // LUI
            execute_lui(emu, instr);
            break;
        case 0x03: // Load instructions
            execute_load(emu, instr);
            break;
        case 0x23: // Store instructions
            execute_store(emu, instr);
            break;
        case 0x3B: // W-type instructions
            execute_addw(emu, instr);
            break;
        case 0x73: // CSR and system instructions
            if (instr.funct3 == 0) {
                if (instr.imm_i == 0) {
                    execute_ecall(emu);
                } else if (instr.imm_i == 1) {
                    execute_ebreak(emu);
                } else if (instr.imm_i == 0x302) {
                    execute_mret(emu);
                }
            } else {
                execute_csr(emu, instr);
            }
            break;
        // ...other instruction types...
        default:
            return false;
    }
    return true;
}

bool fetch_and_execute(Emulator *emu) {
    static size_t executed_instrs = 0;

    if (PC >= MEMORY_SIZE || executed_instrs >= MAX_EXEC_INSTRS) {
        fprintf(stderr, "Maximum instruction limit reached or memory overflow.\n");
        return false;
    }

    uint32_t raw_instr = fetch(emu);
    if (raw_instr == 0xFFFFFFFF) {
        return false;
    }
    
    Instruction instr = {
        .opcode = raw_instr & 0x7F,
        .rd = (raw_instr >> 7) & 0x1F,
        .funct3 = (raw_instr >> 12) & 0x07,
        .rs1 = (raw_instr >> 15) & 0x1F,
        .rs2 = (raw_instr >> 20) & 0x1F,
        .funct7 = (raw_instr >> 25) & 0x7F,
        .imm_i = (int32_t)(raw_instr & 0xFFF00000) >> 20,
        .imm_b = ((raw_instr >> 7) & 0x1E) | ((raw_instr >> 20) & 0x7E0) | ((raw_instr << 4) & 0x800) | ((raw_instr >> 19) & 0x1000),
        .imm_j = ((raw_instr >> 12) & 0xFF) | ((raw_instr >> 20) & 0x1) << 11 | ((raw_instr >> 21) & 0x3FF) << 1 | ((raw_instr >> 31) & 0x1) << 20,
        .imm_u = (int32_t)(raw_instr & 0xFFFFF000),
        .imm_jalr = (int32_t)(raw_instr & 0xFFF00000) >> 20,
        .imm_s = ((raw_instr >> 7) & 0x1F) | ((raw_instr >> 25) & 0x7F) << 5,
        .imm_l = (int32_t)(raw_instr & 0xFFF00000) >> 20
    };
    bool success = execute(emu, instr);
    if (!success) {
        fprintf(stderr, "Failed to execute instruction: 0x%08x\n", raw_instr);
        return false;
    }

    if (emu->log_enabled) {
        log_state(emu, raw_instr);
    }

    PC = DNPC;
    DNPC = PC + 4;
    executed_instrs++;
    return true;
}

void log_state(const Emulator *emu, const uint32_t instr) {
    fprintf(emu->log_file, "PC: 0x%016lx\t", emu->state.pc);  // Print PC as 64-bit hex
    fprintf(emu->log_file, "Instr: 0x%08x\n", instr);
    for (int i = 0; i < NUM_REGS; i++) {
        if (i % 4 == 3) {
            fprintf(emu->log_file, "x%d: 0x%016lx\n", i, emu->state.regs[i]);  // Print registers as 64-bit hex
        } else {
            fprintf(emu->log_file, "x%d: 0x%016lx ", i, emu->state.regs[i]);  // Print registers as 64-bit hex
        }
    }
    fprintf(emu->log_file, "\n");
    // Log CSRs if needed
    // for (int i = 0; i < 4096; i++) {
    //     fprintf(emu->log_file, "CSR[%d]: 0x%08x\n", i, emu->state.csrs[i]);
    // }
}

void execute_r_type(Emulator *emu, Instruction instr) {
    if (instr.rd == 0) return; // Skip if rd is x0

    switch (instr.funct3) {
        case 0x0:
            if (instr.funct7 == 0x00) {
                // ADD
                RD = RS1 + RS2;
            } else if (instr.funct7 == 0x20) {
                // SUB
                RD = RS1 - RS2;
            }
            break;
        case 0x1:
            if (instr.funct7 == 0x00) {
                // SLL
                RD = RS1 << (RS2 & 0x3F);
            }
            break;
        case 0x2:
            if (instr.funct7 == 0x00) {
                // SLT
                RD = (int64_t)RS1 < (int64_t)RS2 ? 1 : 0;
            }
            break;
        case 0x3:
            if (instr.funct7 == 0x00) {
                // SLTU
                RD = RS1 < RS2 ? 1 : 0;
            }
            break;
        case 0x4:
            if (instr.funct7 == 0x00) {
                // XOR
                RD = RS1 ^ RS2;
            } else if (instr.funct7 == 0x01) {
                // MUL
                RD = RS1 * RS2;
            }
            break;
        case 0x5:
            if (instr.funct7 == 0x00) {
                // SRL
                RD = RS1 >> (RS2 & 0x3F);
            } else if (instr.funct7 == 0x20) {
                // SRA
                RD = (int64_t)RS1 >> (RS2 & 0x3F);
            } else if (instr.funct7 == 0x01) {
                // DIV
                RD = RS1 / RS2;
            }
            break;
        case 0x6:
            if (instr.funct7 == 0x00) {
                // OR
                RD = RS1 | RS2;
            } else if (instr.funct7 == 0x01) {
                // REM
                RD = RS1 % RS2;
            }
            break;
        case 0x7:
            if (instr.funct7 == 0x00) {
                // AND
                RD = RS1 & RS2;
            }
            break;
        // ...other R-type instructions...
    }
}

void execute_i_type(Emulator *emu, Instruction instr) {
    if (instr.rd == 0) return; // Skip if rd is x0

    switch (instr.funct3) {
        case 0x0:
            // ADDI
            RD = RS1 + instr.imm_i;
            break;
        case 0x2:
            // SLTI
            RD = (int64_t)RS1 < instr.imm_i ? 1 : 0;
            break;
        case 0x3:
            // SLTIU
            RD = RS1 < (uint64_t)instr.imm_i ? 1 : 0;
            break;
        case 0x4:
            // XORI
            RD = RS1 ^ instr.imm_i;
            break;
        case 0x6:
            // ORI
            RD = RS1 | instr.imm_i;
            break;
        case 0x7:
            // ANDI
            RD = RS1 & instr.imm_i;
            break;
        case 0x1:
            // SLLI
            RD = RS1 << (instr.imm_i & 0x3F);
            break;
        case 0x5:
            if ((instr.imm_i & 0x400) == 0) {
                // SRLI
                RD = RS1 >> (instr.imm_i & 0x3F);
            } else {
                // SRAI
                RD = (int64_t)RS1 >> (instr.imm_i & 0x3F);
            }
            break;
        // ...other I-type instructions...
    }
}

void execute_b_type(Emulator *emu, Instruction instr) {
    int32_t imm = (instr.imm_b << 19) >> 19; // Sign-extend the immediate

    switch (instr.funct3) {
        case 0x0:
            // BEQ
            if (RS1 == RS2) {
                DNPC = PC + imm;
            }
            break;
        case 0x1:
            // BNE
            if (RS1 != RS2) {
                DNPC = PC + imm;
            }
            break;
        case 0x4:
            // BLT
            if ((int64_t)RS1 < (int64_t)RS2) {
                DNPC = PC + imm;
            }
            break;
        case 0x5:
            // BGE
            if ((int64_t)RS1 >= (int64_t)RS2) {
                DNPC = PC + imm;
            }
            break;
        case 0x6:
            // BLTU
            if (RS1 < RS2) {
                DNPC = PC + imm;
            }
            break;
        case 0x7:
            // BGEU
            if (RS1 >= RS2) {
                DNPC = PC + imm;
            }
            break;
        // ...other B-type instructions...
    }
}

void execute_jal(Emulator *emu, Instruction instr) {
    if (instr.rd != 0) {
        RD = PC + 4;
    }
    int32_t imm = (instr.imm_j << 11) >> 11; // Sign-extend the immediate
    DNPC = PC + imm;
}

void execute_jalr(Emulator *emu, Instruction instr) {
    if (instr.rd != 0) {
        RD = PC + 4;
    }
    DNPC = (RS1 + instr.imm_jalr) & ~1;
}

void execute_auipc(Emulator *emu, Instruction instr) {
    if (instr.rd != 0) {
        RD = PC + (instr.imm_i << 12);
    }
}

void execute_lui(Emulator *emu, Instruction instr) {
    if (instr.rd != 0) {
        RD = instr.imm_i << 12;
    }
}

void execute_load(Emulator *emu, Instruction instr) {
    uint64_t address = RS1 + instr.imm_l;
    switch (instr.funct3) {
        case 0x0: // LB
            RD = (int8_t)emu->memory[address];
            break;
        case 0x1: // LH
            RD = (int16_t)(emu->memory[address] | 
                (emu->memory[address + 1] << 8));
            break;
        case 0x2: // LW
            RD = (int32_t)(emu->memory[address] | 
                          (emu->memory[address + 1] << 8) | 
                          (emu->memory[address + 2] << 16) | 
                          (emu->memory[address + 3] << 24));
            break;
        case 0x3: // LD
            RD = (uint64_t)(emu->memory[address] | 
                           (emu->memory[address + 1] << 8) | 
                           (emu->memory[address + 2] << 16) | 
                           (emu->memory[address + 3] << 24) |
                           ((uint64_t)emu->memory[address + 4] << 32) | 
                           ((uint64_t)emu->memory[address + 5] << 40) | 
                           ((uint64_t)emu->memory[address + 6] << 48) | 
                           ((uint64_t)emu->memory[address + 7] << 56));
            break;
        case 0x4: // LBU
            RD = emu->memory[address];
            break;
        case 0x5: // LHU
            RD = (uint16_t)(emu->memory[address] | (emu->memory[address + 1] << 8));
            break;
        case 0x6: // LWU
            RD = (uint32_t)(emu->memory[address] | (emu->memory[address + 1] << 8) | (emu->memory[address + 2] << 16) | (emu->memory[address + 3] << 24));
            break;
        // ...other load instructions...
    }
}

void execute_store(Emulator *emu, Instruction instr) {
    uint64_t address = RS1 + instr.imm_s;
    switch (instr.funct3) {
        case 0x0: // SB
            emu->memory[address] = RS2 & 0xFF;
            break;
        case 0x1: // SH
            emu->memory[address] = RS2 & 0xFF;
            emu->memory[address + 1] = (RS2 >> 8) & 0xFF;
            break;
        case 0x2: // SW
            emu->memory[address] = RS2 & 0xFF;
            emu->memory[address + 1] = (RS2 >> 8) & 0xFF;
            emu->memory[address + 2] = (RS2 >> 16) & 0xFF;
            emu->memory[address + 3] = (RS2 >> 24) & 0xFF;
            break;
        case 0x3: // SD
            emu->memory[address] = RS2 & 0xFF;
            emu->memory[address + 1] = (RS2 >> 8) & 0xFF;
            emu->memory[address + 2] = (RS2 >> 16) & 0xFF;
            emu->memory[address + 3] = (RS2 >> 24) & 0xFF;
            emu->memory[address + 4] = (RS2 >> 32) & 0xFF;
            emu->memory[address + 5] = (RS2 >> 40) & 0xFF;
            emu->memory[address + 6] = (RS2 >> 48) & 0xFF;
            emu->memory[address + 7] = (RS2 >> 56) & 0xFF;
            break;
        // ...other store instructions...
    }
}

void execute_addw(Emulator *emu, Instruction instr) {
    if (instr.rd == 0) return; // Skip if rd is x0

    switch (instr.funct3) {
        case 0x0:
            if (instr.funct7 == 0x00) {
                // ADDW
                RD = (int32_t)(RS1 + RS2);
            } else if (instr.funct7 == 0x20) {
                // SUBW
                RD = (int32_t)(RS1 - RS2);
            }
            break;
        // ...other W-type instructions...
    }
}

void execute_csr(Emulator *emu, Instruction instr) {
    uint32_t csr = instr.imm_i & 0xFFF;
    uint64_t value = emu->state.csrs[csr];

    switch (instr.funct3) {
        case 0x1: // CSRRW
            emu->state.csrs[csr] = RS1;
            RD = value;
            break;
        case 0x2: // CSRRS
            emu->state.csrs[csr] |= RS1;
            RD = value;
            break;
        case 0x3: // CSRRC
            emu->state.csrs[csr] &= ~RS1;
            RD = value;
            break;
        case 0x5: // CSRRWI
            emu->state.csrs[csr] = instr.rs1;
            RD = value;
            break;
        case 0x6: // CSRRSI
            emu->state.csrs[csr] |= instr.rs1;
            RD = value;
            break;
        case 0x7: // CSRRCI
            emu->state.csrs[csr] &= ~instr.rs1;
            RD = value;
            break;
        // ...other CSR instructions...
    }
}

void execute_ecall(Emulator *emu) {
    // Handle system call
    // For simplicity, we just print a message and set the appropriate CSRs
    printf("ECALL at PC: 0x%016lx\n", emu->state.pc);
    emu->state.csrs[CSR_MEPC] = emu->state.pc;
    emu->state.csrs[CSR_MCAUSE] = 11; // Environment call from M-mode
    emu->state.csrs[CSR_MTVAL] = 0;
    DNPC = emu->state.csrs[CSR_MTVEC];
}

void execute_ebreak(Emulator *emu) {
    // Handle breakpoint
    // For simplicity, we just print a message and set the appropriate CSRs
    printf("EBREAK at PC: 0x%016lx\n", emu->state.pc);
    emu->state.csrs[CSR_MEPC] = emu->state.pc;
    emu->state.csrs[CSR_MCAUSE] = 3; // Breakpoint
    emu->state.csrs[CSR_MTVAL] = 0;
    DNPC = emu->state.csrs[CSR_MTVEC];
}

void execute_mret(Emulator *emu) {
    // Set the PC to the value of the MEPC CSR
    DNPC = emu->state.csrs[CSR_MEPC];
    // Restore the previous privilege mode from the MSTATUS CSR
    emu->state.csrs[CSR_MSTATUS] = (emu->state.csrs[CSR_MSTATUS] & ~0x1800) | ((emu->state.csrs[CSR_MSTATUS] >> 4) & 0x1800);
}