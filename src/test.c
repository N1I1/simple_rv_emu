#include <stdio.h>
#include <assert.h>
#include "emulator.h"
// Remove the conflicting include
#include "state.h"

Instruction int_to_instruction(uint32_t raw_instr) {
    return (Instruction) {
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
}

void run_tests() {
    Emulator emu;
    init_emulator(&emu, "assets/instr.hex", PC_START, NUM_INSTRS, "build/test.log");

    // Test ADD
    emu.state.regs[1] = 5;
    emu.state.regs[2] = 10;
    execute_r_type(&emu, int_to_instruction(0x002080b3)); // ADD x1, x1, x2
    assert(emu.state.regs[1] == 15);

    emu.state.regs[1] = 0xffffffffffffffff;
    emu.state.regs[2] = 1;
    execute_r_type(&emu, int_to_instruction(0x002080b3)); // ADD x1, x1, x2
    assert(emu.state.regs[1] == 0);

    execute_r_type(&emu, int_to_instruction(0x00208033)); // ADD x0, x1, x2
    assert(emu.state.regs[0] == 0);

    printf("\033[0;32mADD\t PASSED\n");

    // Test SUB
    emu.state.regs[1] = 10;
    emu.state.regs[2] = 5;
    execute_r_type(&emu, int_to_instruction(0x402080b3)); // SUB x1, x1, x2
    assert(emu.state.regs[1] == 5);

    emu.state.regs[1] = 0;
    emu.state.regs[2] = 1;
    execute_r_type(&emu, int_to_instruction(0x402080b3)); // SUB x1, x1, x2
    assert(emu.state.regs[1] == (uint64_t)-1);

    printf("\033[0;32mSUB\t PASSED\n");
    // Test SLL
    emu.state.regs[1] = 1;
    emu.state.regs[2] = 3;
    execute_r_type(&emu, int_to_instruction(0x002090b3)); // SLL x1, x1, x2
    assert(emu.state.regs[1] == 8);
    printf("\033[0;32mSLL\t PASSED\n");
    
    // Test SRL
    emu.state.regs[1] = 16;
    emu.state.regs[2] = 2;
    execute_r_type(&emu, int_to_instruction(0x0020d0b3)); // SRL x1, x1, x2
    assert(emu.state.regs[1] == 4);
    printf("\033[0;32mSRL\t PASSED\n");

    // Test SRA
    emu.state.regs[1] = -16;
    emu.state.regs[2] = 2;
    execute_r_type(&emu, int_to_instruction(0x4020d0b3)); // SRA x1, x1, x2
    assert(emu.state.regs[1] == (uint64_t)-4);
    printf("\033[0;32mSRA\t PASSED\n");

    // Test ADDI
    emu.state.regs[1] = 5;
    execute_i_type(&emu, int_to_instruction(0x00508093)); // ADDI x1, x1, 5
    assert(emu.state.regs[1] == 10);
    printf("\033[0;32mADDI\t PASSED\n");

    // Test SLTI
    emu.state.regs[1] = 5;
    execute_i_type(&emu, int_to_instruction(0x0060a093)); // SLTI x1, x1, 6
    assert(emu.state.regs[1] == 1);

    emu.state.regs[1] = 7;
    execute_i_type(&emu, int_to_instruction(0x0060a093)); // SLTI x1, x1, 6
    assert(emu.state.regs[1] == 0);
    printf("\033[0;32mSLTI\t PASSED\n");

    // Test SLTIU
    emu.state.regs[1] = 5;
    execute_i_type(&emu, int_to_instruction(0x0060a093)); // SLTIU x1, x1, 6
    assert(emu.state.regs[1] == 1);

    emu.state.regs[1] = 7;
    execute_i_type(&emu, int_to_instruction(0x0060b093)); // SLTIU x1, x1, 6
    assert(emu.state.regs[1] == 0);
    printf("\033[0;32mSLTIU\t PASSED\n");

    // Test XORI
    emu.state.regs[1] = 5;
    execute_i_type(&emu, int_to_instruction(0x00f0c093)); // XORI x1, x1, 15
    assert(emu.state.regs[1] == 10);
    printf("\033[0;32mXORI\t PASSED\n");

    // Test ORI
    emu.state.regs[1] = 5;
    execute_i_type(&emu, int_to_instruction(0x00f0e093)); // ORI x1, x1, 15
    assert(emu.state.regs[1] == 15);
    printf("\033[0;32mORI\t PASSED\n");

    // Test ANDI
    emu.state.regs[1] = 5;
    execute_i_type(&emu, int_to_instruction(0x00f0f093)); // ANDI x1, x1, 15
    assert(emu.state.regs[1] == 5);
    printf("\033[0;32mANDI\t PASSED\n");

    // Test SLLI
    emu.state.regs[1] = 1;
    execute_i_type(&emu, int_to_instruction(0x00309093)); // SLLI x1, x1, 3
    assert(emu.state.regs[1] == 8);
    printf("\033[0;32mSLLI\t PASSED\n");

    // Test SRLI
    emu.state.regs[1] = 16;
    execute_i_type(&emu, int_to_instruction(0x0020d093)); // SRLI x1, x1, 2
    assert(emu.state.regs[1] == 4);
    printf("\033[0;32mSRLI\t PASSED\n");

    // Test SRAI
    emu.state.regs[1] = -16;
    execute_i_type(&emu, int_to_instruction(0x4020d093)); // SRAI x1, x1, 2
    assert(emu.state.regs[1] == (uint64_t)-4);
    printf("\033[0;32mSRAI\t PASSED\n");

    emu.state.pc = 0x100; // Set PC to 0x100
    // Test BEQ
    emu.state.regs[1] = 5;
    emu.state.regs[2] = 5;
    execute_b_type(&emu, int_to_instruction(0x00208063)); // BEQ x1, x2, 0
    assert(emu.state.dnpc == emu.state.pc);

    emu.state.dnpc = 0x100 + 4;
    emu.state.regs[1] = 5;
    emu.state.regs[2] = 10;
    execute_b_type(&emu, int_to_instruction(0x00208063)); // BEQ x1, x2, 0
    assert(emu.state.dnpc == emu.state.pc + 4);
    printf("\033[0;32mBEQ\t PASSED\n");

    // Test BNE
    emu.state.dnpc = 0x100 + 4;
    emu.state.regs[1] = 5;
    emu.state.regs[2] = 10;
    execute_b_type(&emu, int_to_instruction(0x00209063)); // BNE x1, x2, 0
    assert(emu.state.dnpc ==  emu.state.pc);

    emu.state.dnpc = 0x100 + 4;
    emu.state.regs[1] = 5;
    emu.state.regs[2] = 5;
    execute_b_type(&emu, int_to_instruction(0x00209063)); // BNE x1, x2, 0
    assert(emu.state.dnpc == emu.state.pc + 4);
    printf("\033[0;32mBNE\t PASSED\n");

    // Test BLT
    emu.state.dnpc = 0x100 + 4;
    emu.state.regs[1] = 5;
    emu.state.regs[2] = 10;
    execute_b_type(&emu, int_to_instruction(0x0020c063)); // BLT x1, x2, 0
    assert(emu.state.dnpc == emu.state.pc);

    emu.state.dnpc = 0x100 + 4;
    emu.state.regs[1] = 10;
    emu.state.regs[2] = 5;
    execute_b_type(&emu, int_to_instruction(0x0020c063)); // BLT x1, x2, 0
    assert(emu.state.dnpc == emu.state.pc + 4);
    printf("\033[0;32mBLT\t PASSED\n");

    // Test BGE
    emu.state.dnpc = 0x100 + 4;
    emu.state.regs[1] = 10;
    emu.state.regs[2] = 5;
    execute_b_type(&emu, int_to_instruction(0x0020d063)); // BGE x1, x2, 0
    assert(emu.state.dnpc == emu.state.pc);

    emu.state.dnpc = 0x100 + 4;
    emu.state.regs[1] = 5;
    emu.state.regs[2] = 10;
    execute_b_type(&emu, int_to_instruction(0x0020d063)); // BGE x1, x2, 0
    assert(emu.state.dnpc == emu.state.pc + 4);
    printf("\033[0;32mBGE\t PASSED\n");

    // Test BLTU
    emu.state.dnpc = 0x100 + 4;
    emu.state.regs[1] = 5;
    emu.state.regs[2] = 10;
    execute_b_type(&emu, int_to_instruction(0x0020e063)); // BLTU x1, x2, 0
    assert(emu.state.dnpc == emu.state.pc);

    emu.state.dnpc = 0x100 + 4;
    emu.state.regs[1] = 10;
    emu.state.regs[2] = 5;
    execute_b_type(&emu, int_to_instruction(0x0020e063)); // BLTU x1, x2, 0
    assert(emu.state.dnpc == emu.state.pc + 4);
    printf("\033[0;32mBLTU\t PASSED\n");

    // Test BGEU
    emu.state.dnpc = 0x100 + 4;
    emu.state.regs[1] = 10;
    emu.state.regs[2] = 5;
    execute_b_type(&emu, int_to_instruction(0x0020f063)); // BGEU x1, x2, 0
    assert(emu.state.dnpc == emu.state.pc);

    emu.state.dnpc = 0x100 + 4;
    emu.state.regs[1] = 5;
    emu.state.regs[2] = 10;
    execute_b_type(&emu, int_to_instruction(0x0020f063)); // BGEU x1, x2, 0
    assert(emu.state.dnpc == emu.state.pc + 4);
    printf("\033[0;32mBGEU\t PASSED\n");

    emu.state.regs[1] = 0;
    // Test JAL
    emu.state.pc = 0x100;
    emu.state.dnpc = 0x100 + 4;
    execute_jal(&emu, int_to_instruction(0x0000006F)); // JAL x0, 0
    assert(emu.state.dnpc == emu.state.pc);

    emu.state.pc = 0x100;
    emu.state.dnpc = 0x100 + 4;
    execute_jal(&emu, int_to_instruction(0x0040006F)); // JAL x0, 4
    assert(emu.state.dnpc == emu.state.pc + 4);

    emu.state.pc = 0x100;
    emu.state.dnpc = 0x100 + 4;
    execute_jal(&emu, int_to_instruction(0x004000EF)); // JAL x1, 4
    assert(emu.state.dnpc == emu.state.pc + 4);
    assert(emu.state.regs[1] == emu.state.pc + 4);
    printf("\033[0;32mJAL\t PASSED\n");

    // Test JALR
    emu.state.pc = 0x100;
    emu.state.regs[1] = 0x200;
    execute_jalr(&emu, int_to_instruction(0x00008067)); // JALR x0, 0(x1)
    assert(emu.state.dnpc == ((emu.state.regs[1] + 0) & ~1));

    emu.state.pc = 0x100;
    emu.state.regs[1] = 0x200;
    execute_jalr(&emu, int_to_instruction(0x00408067)); // JALR x0, 4(x1)
    assert(emu.state.dnpc == ((emu.state.regs[1] + 4) & ~1));

    emu.state.pc = 0x100;
    emu.state.regs[1] = 0x200;
    execute_jalr(&emu, int_to_instruction(0x004080E7)); // JALR x1, 4(x1)
    assert(emu.state.dnpc == ((emu.state.regs[1] + 4) & ~1));
    assert(emu.state.regs[1] == emu.state.pc + 4);
    printf("\033[0;32mJALR\t PASSED\n");

    // Test AUIPC
    emu.state.pc = 0x100;
    execute_auipc(&emu, int_to_instruction(0x00000017)); // AUIPC x0, 0
    assert(emu.state.regs[0] == 0);

    emu.state.pc = 0x100;
    execute_auipc(&emu, int_to_instruction(0x00400017)); // AUIPC x0, 4
    assert(emu.state.regs[0] == 0);

    emu.state.pc = 0x100;
    execute_auipc(&emu, int_to_instruction(0x00400097)); // AUIPC x1, 4
    assert(emu.state.regs[1] == emu.state.pc + (4 << 12));
    printf("\033[0;32mAUIPC\t PASSED\n");

    // Test LUI
    emu.state.pc = 0x100;
    execute_lui(&emu, int_to_instruction(0x00000037)); // LUI x0, 0
    assert(emu.state.regs[0] == 0);

    emu.state.pc = 0x100;
    execute_lui(&emu, int_to_instruction(0x00400037)); // LUI x0, 4
    assert(emu.state.regs[0] == 0);

    emu.state.pc = 0x100;
    execute_lui(&emu, int_to_instruction(0x004000B7)); // LUI x1, 4
    assert(emu.state.regs[1] == (4 << 12));
    printf("\033[0;32mLUI\t PASSED\n");

    // Test LB
    emu.state.regs[1] = 0x100;
    emu.memory[0x100] = 0xFF;
    execute_load(&emu, int_to_instruction(0x00008083)); // LB x1, 0(x1)
    assert((int8_t)emu.state.regs[1] == -1);
    printf("\033[0;32mLB\t PASSED\n");

    // Test LH
    emu.state.regs[1] = 0x100;
    emu.memory[0x100] = 0xFF;
    emu.memory[0x101] = 0xFF;
    execute_load(&emu, int_to_instruction(0x00009083)); // LH x1, 0(x1)
    assert((int16_t)emu.state.regs[1] == -1);
    printf("\033[0;32mLH\t PASSED\n");

    // Test LW
    emu.state.regs[1] = 0x100;
    emu.memory[0x100] = 0xFF;
    emu.memory[0x101] = 0xFF;
    emu.memory[0x102] = 0xFF;
    emu.memory[0x103] = 0xFF;
    execute_load(&emu, int_to_instruction(0x0000a083)); // LW x1, 0(x1)
    assert((int32_t)emu.state.regs[1] == -1);
    printf("\033[0;32mLW\t PASSED\n");

    // Test LD
    emu.state.regs[1] = 0x100;
    emu.memory[0x100] = 0xFF;
    emu.memory[0x101] = 0xFF;
    emu.memory[0x102] = 0xFF;
    emu.memory[0x103] = 0xFF;
    emu.memory[0x104] = 0xFF;
    emu.memory[0x105] = 0xFF;
    emu.memory[0x106] = 0xFF;
    emu.memory[0x107] = 0xFF;
    execute_load(&emu, int_to_instruction(0x0000b083)); // LD x1, 0(x1)
    assert((int64_t)emu.state.regs[1] == -1);
    printf("\033[0;32mLD\t PASSED\n");

    // Test LBU
    emu.state.regs[1] = 0x100;
    emu.memory[0x100] = 0xFF;
    execute_load(&emu, int_to_instruction(0x0000c083)); // LBU x1, 0(x1)
    assert(emu.state.regs[1] == 0xFF);
    printf("\033[0;32mLBU\t PASSED\n");

    // Test LHU
    emu.state.regs[1] = 0x100;
    emu.memory[0x100] = 0xFF;
    emu.memory[0x101] = 0xFF;
    execute_load(&emu, int_to_instruction(0x0000d083)); // LHU x1, 0(x1)
    assert(emu.state.regs[1] == 0xFFFF);
    printf("\033[0;32mLHU\t PASSED\n");

    // Test LWU
    emu.state.regs[1] = 0x100;
    emu.memory[0x100] = 0xFF;
    emu.memory[0x101] = 0xFF;
    emu.memory[0x102] = 0xFF;
    emu.memory[0x103] = 0xFF;
    execute_load(&emu, int_to_instruction(0x0000e083)); // LWU x1, 0(x1)
    assert(emu.state.regs[1] == 0xFFFFFFFF);
    printf("\033[0;32mLWU\t PASSED\n");

    // Test SB
    emu.memory[0x100] = 0;
    emu.state.regs[1] = 0x100;
    emu.state.regs[2] = 0xFF;
    execute_store(&emu, int_to_instruction(0x00208023)); // SB x2, 0(x1)
    assert(emu.memory[0x100] == 0xFF);
    printf("\033[0;32mSB\t PASSED\n");

    // Test SH
    emu.memory[0x100] = 0;
    emu.memory[0x101] = 0;
    emu.state.regs[1] = 0x100;
    emu.state.regs[2] = 0xFFFF;
    execute_store(&emu, int_to_instruction(0x00209023)); // SH x2, 0(x1)
    assert(emu.memory[0x100] == 0xFF && emu.memory[0x101] == 0xFF);
    printf("\033[0;32mSH\t PASSED\n");

    // Test SW
    emu.memory[0x100] = 0;
    emu.memory[0x101] = 0;
    emu.memory[0x102] = 0;
    emu.memory[0x103] = 0;
    emu.state.regs[1] = 0x100;
    emu.state.regs[2] = 0xFFFFFFFF;
    execute_store(&emu, int_to_instruction(0x0020A023)); // SW x2, 0(x1)
    assert(emu.memory[0x100] == 0xFF && emu.memory[0x101] == 0xFF && emu.memory[0x102] == 0xFF && emu.memory[0x103] == 0xFF);
    printf("\033[0;32mSW\t PASSED\n");

    // Test SD
    emu.memory[0x100] = 0;
    emu.memory[0x101] = 0;
    emu.memory[0x102] = 0;
    emu.memory[0x103] = 0;
    emu.memory[0x104] = 0;
    emu.memory[0x105] = 0;
    emu.memory[0x106] = 0;
    emu.memory[0x107] = 0;
    emu.state.regs[1] = 0x100;
    emu.state.regs[2] = 0xFFFFFFFFFFFFFFFF;
    execute_store(&emu, int_to_instruction(0x0020B023)); // SD x2, 0(x1)
    assert(emu.memory[0x100] == 0xFF && emu.memory[0x101] == 0xFF && emu.memory[0x102] == 0xFF && emu.memory[0x103] == 0xFF &&
           emu.memory[0x104] == 0xFF && emu.memory[0x105] == 0xFF && emu.memory[0x106] == 0xFF && emu.memory[0x107] == 0xFF);
    printf("\033[0;32mSD\t PASSED\n");

    // Test ADDW
    emu.state.regs[1] = 0xFFFFFFFF;
    emu.state.regs[2] = 1;
    execute_addw(&emu, int_to_instruction(0x002080BB)); // ADDW x1, x1, x2
    assert((int32_t)emu.state.regs[1] == 0);
    printf("\033[0;32mADDW\t PASSED\n");

    // Test SUBW
    emu.state.regs[1] = 0;
    emu.state.regs[2] = 1;
    execute_addw(&emu, int_to_instruction(0x402080BB)); // SUBW x1, x1, x2
    assert((int32_t)emu.state.regs[1] == -1);
    printf("\033[0;32mSUBW\t PASSED\n");

    // Test CSR instructions
    emu.state.regs[1] = 0x1234;
    emu.state.csrs[CSR_MSTATUS] = 0x0;
    execute_csr(&emu, int_to_instruction(0x300090f3)); // CSRRW x1, mstatus, x1
    assert(emu.state.csrs[CSR_MSTATUS] == 0x1234);
    assert(emu.state.regs[1] == 0x0);
    printf("\033[0;32mCSRRW\t PASSED\n");

    emu.state.regs[1] = 0x1;
    execute_csr(&emu, int_to_instruction(0x3000a0fe)); // CSRRS x1, mstatus, x1
    assert(emu.state.csrs[CSR_MSTATUS] == 0x1235);
    assert(emu.state.regs[1] == 0x1234);
    printf("\033[0;32mCSRRS\t PASSED\n");

    emu.state.regs[1] = 0x1;
    execute_csr(&emu, int_to_instruction(0x3000b0f3)); // CSRRC x1, mstatus, x1
    assert(emu.state.csrs[CSR_MSTATUS] == 0x1234);
    assert(emu.state.regs[1] == 0x1235);
    printf("\033[0;32mCSRRC\t PASSED\n");

    execute_csr(&emu, int_to_instruction(0x3000d0f3)); // CSRRWI x1, mstatus, 1
    assert(emu.state.csrs[CSR_MSTATUS] == 0x1);
    assert(emu.state.regs[1] == 0x1234);
    printf("\033[0;32mCSRRWI\t PASSED\n");

    execute_csr(&emu, int_to_instruction(0x300160f3)); // CSRRSI x1, mstatus, 2
    assert(emu.state.csrs[CSR_MSTATUS] == 0x3);
    assert(emu.state.regs[1] == 0x1);
    printf("\033[0;32mCSRRSI\t PASSED\n");

    execute_csr(&emu, int_to_instruction(0x3001f0f3)); // CSRRCI x1, mstatus, 3
    assert(emu.state.csrs[CSR_MSTATUS] == 0x0);
    assert(emu.state.regs[1] == 0x3);
    printf("\033[0;32mCSRRCI\t PASSED\n");

    // Test ECALL
    emu.state.pc = 0x100;
    execute_ecall(&emu);
    assert(emu.state.csrs[CSR_MEPC] == 0x100);
    assert(emu.state.csrs[CSR_MCAUSE] == 11);
    assert(emu.state.dnpc == emu.state.csrs[CSR_MTVEC]);
    printf("\033[0;32mECALL\t PASSED\n");

    // Test EBREAK
    emu.state.pc = 0x100;
    execute_ebreak(&emu);
    assert(emu.state.csrs[CSR_MEPC] == 0x100);
    assert(emu.state.csrs[CSR_MCAUSE] == 3);
    assert(emu.state.dnpc == emu.state.csrs[CSR_MTVEC]);
    printf("\033[0;32mEBREAK\t PASSED\n");

    // Test MRET
    emu.state.csrs[CSR_MEPC] = 0x200;
    emu.state.csrs[CSR_MSTATUS] = 0x1800;
    execute_mret(&emu);
    assert(emu.state.dnpc == 0x200);
    assert((emu.state.csrs[CSR_MSTATUS] & 0x1800) == 0x0);
    printf("\033[0;32mMRET\t PASSED\n");

    fclose(emu.log_file);
}

int main() {
    run_tests();
    printf("\033[38;5;206m");
    printf("\033[38;5;206mAll tests passed.\033[0m\n");
    return 0;
}