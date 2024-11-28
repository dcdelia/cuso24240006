#include "pin.H"
#include <fstream>
#include <iostream>
#include <stdint.h>

static std::ofstream traceFile;
static uint32_t indCallCount;

static ADDRINT mainModuleBegin, mainModuleEnd;

VOID analyzeIndirectCall(ADDRINT src, ADDRINT dest) {
    // GOAL: log call site and destination, possibly showing symbols for both

    /* Hints:
       - use RTN_FindByAddress to locate the RTN enclosing the instruction address
       - pad RTN manipulations with a PIN_LockClient() - PIN_UnlockClient() sandwich
       - use RTN_Valid() to check if a RTN is valid before getting its name with RTN_Name()
       - use traceFile with output streams (as if it were std::cerr)
    */
    ++indCallCount;

    /*** TODO ***/
}

void ImageCallback(IMG img, void* v) {
    // Collect lowest and highest address for main binary
    if (IMG_IsMainExecutable(img)) {
        mainModuleBegin = IMG_LowAddress(img);
        mainModuleEnd = IMG_HighAddress(img);
    }
}

void InstructionCallback(INS ins, void *v) {
     if (INS_IsIndirectControlFlow(ins)) { // transfer target from memory or register
        if (INS_IsCall(ins)) { // through a call
            // GOAL: check if instruction is from main program and register
            //       a callback that traces the call site and the callee

            /* Hints
               - you can inspect EIP by passing IARG_INST_PTR to the callback
               - IARG_BRANCH_TARGET_ADDR gives you the call target at run-time
            */

            /*** TODO ***/
        }
     }
}

void FiniCallback(int code, void *v) {
    // Beware: with many programs you better write data to a file as the
    //         standard I/O descriptors may be closed by when we get here 
    std::cerr << "Total icalls executed: " << indCallCount << std::endl;
}

int main(int argc, char *argv[]) {
    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize Pin
    PIN_Init(argc, argv);

    // Add instrumentation for instructions
    INS_AddInstrumentFunction(InstructionCallback, NULL);

    // Add callback for image loading
    IMG_AddInstrumentFunction(ImageCallback, NULL);

    // Call FiniCallback when the program finishes
    PIN_AddFiniFunction(FiniCallback, NULL);

    // Set up log file
    traceFile.open("icalls.log");

    // Start the program execution
    PIN_StartProgram();

    return 0;
}
