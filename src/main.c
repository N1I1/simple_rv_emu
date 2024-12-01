#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "emulator.h"
#include "state.h"

int main(int argc, char *argv[]) {
    const char *hex_file = (argc > 1) ? argv[1] : "assets/instr.hex";
    uint64_t start_pc = (argc > 2) ? strtoull(argv[2], NULL, 0) : PC_START;
    size_t num_instrs = (argc > 3) ? strtoul(argv[3], NULL, 0) : NUM_INSTRS;
    const char *log_file = (argc > 4) ? argv[4] : LOG_FILE;
    bool log_enabled = (argc > 5) ? (strcmp(argv[5], "true") == 0) : true;

    Emulator emu;
    init_emulator(&emu, hex_file, start_pc, num_instrs, log_file);
    emu.log_enabled = log_enabled;

    while (fetch_and_execute(&emu));

    fclose(emu.log_file);
    return 0;
}