#include "pin.H"
#include <iostream>
#include <stdint.h>

void InstructionCallback(INS ins, void *v) {
    // GOAL: register instrumentation for tracking each executed instruction

    /* Hints:
        - check INS_InsertCall instrumentation API
        - with INS_Address(ins) you can display the address for debugging
    */

    /*** TODO ***/
}

void FiniCallback(int code, void *v) {
    // GOAL: print how many instructions the program executed

    /*** TODO ***/
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