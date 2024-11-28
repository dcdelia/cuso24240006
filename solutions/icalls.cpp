#include "pin.H"
#include <fstream>
#include <iostream>
#include <stdint.h>

static BOOL trackLibs = false;

static std::ofstream traceFile;
static uint32_t indCallCount;
static ADDRINT mainModuleBegin, mainModuleEnd;

VOID analyzeIndirectCall(ADDRINT src, ADDRINT dest) {
    ++indCallCount;
    traceFile << std::hex << src << " -> " << dest;
    
    PIN_LockClient(); // needed for some RTN manipulations
    RTN rtnSrc = RTN_FindByAddress(src);
    RTN rtnDest = RTN_FindByAddress(dest);
    traceFile << " (" << (RTN_Valid(rtnSrc) ? RTN_Name(rtnSrc) : "UNK")
              << " -> " << (RTN_Valid(rtnDest) ? RTN_Name(rtnDest) : "UNK") << ")" << std::endl;
    PIN_UnlockClient();
}

void ImageCallback(IMG img, void* v) {
    if (IMG_IsMainExecutable(img)) {
        mainModuleBegin = IMG_LowAddress(img);
        mainModuleEnd = IMG_HighAddress(img);
    }
}

void InstructionCallback(INS ins, void *v) {
     if (INS_IsIndirectControlFlow(ins)) { // transfer target from memory or register
        if (INS_IsCall(ins)) { // through a call
            ADDRINT addr = INS_Address(ins);
            if (trackLibs || (addr >= mainModuleBegin && addr < mainModuleEnd))
                INS_InsertCall(ins,
                            IPOINT_BEFORE,
                            (AFUNPTR)analyzeIndirectCall,
                            IARG_INST_PTR,
                            IARG_BRANCH_TARGET_ADDR,
                            IARG_END);
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