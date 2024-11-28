#include "pin.H"
#include <iostream>
#include <stdint.h>

static uint64_t instruction_count_executed;
static uint64_t instruction_count_instrumented;

void analyzeInstruction() {
    // Will be invoked every time an instruction is executed
    ++instruction_count_executed;
}

void InstructionCallback(INS ins, void *v) {
    // Will be invoked every time Pin is about to translate a new instruction

    #if 0   // Optional: print instruction address (noisy)
    std::cerr << "Instruction address: " << std::hex << INS_Address(ins) << std::endl;
    #endif

    INS_InsertCall(ins,
                  IPOINT_BEFORE,
                  (AFUNPTR)analyzeInstruction,
                  IARG_END);

    ++instruction_count_instrumented;
}

void FiniCallback(int code, void *v) {
    // Note: with some programs you will be unable to see this message 
    std::cerr << "Instructions instrumented: " << instruction_count_instrumented << std::endl;
    std::cerr << "Instructions executed:     " << instruction_count_executed << std::endl;
}

int main(int argc, char *argv[]) {
    // Initialize Pin
    PIN_Init(argc, argv);

    // Add instrumentation function for instructions
    INS_AddInstrumentFunction(InstructionCallback, NULL);

    // Call FiniCallback when the program finishes
    PIN_AddFiniFunction(FiniCallback, NULL);

    // Start the program execution
    PIN_StartProgram();

    return 0;
}