#include "pin.H"
#include <iostream>
#include <signal.h>
#include <stdint.h>

#define STACK_MAX_DEPTH 1024

/* Maintain an array of shadow stack nodes */
typedef struct {
    ADDRINT routine;
    ADDRINT callsite;
    ADDRINT retaddr;
} ss_node_t;

int shadow_stack_idx = 0;
ss_node_t shadow_stack[STACK_MAX_DEPTH];

VOID processCall(ADDRINT routine, ADDRINT callsite, ADDRINT retaddr) {
    // GOAL: add a node to the shadow stack

    /* Hint: add a sanity check that you do not exceed the maximum stack depth */

    /*** TODO ***/
}

VOID processRet(ADDRINT* esp) {
    // GOAL: pop a node from the shadow stack

    /* Hints:
       - add a sanity check that an entry actually exists
       - check that the return address corresponds to the one in the node
    */

    /*** TODO ***/
}

void InstructionCallback(INS ins, void *v) {
    if (INS_IsCall(ins)) {
        // GOAL: pass the following run-time data
        //       - call site (instruction pointer)
        //       - destination address
        //       - return address (i.e., instruction after the call)
		if (INS_IsDirectCall(ins)) {
            /* Hint: you can use INS_DirectControlFlowTargetAddress() */

            /*** TODO ***/
		}
		else {
            /* Hint: you can use IARG_BRANCH_TARGET_ADDR */

            /*** TODO ***/
		}
	} else if (INS_IsRet(ins)) {
        // GOAL: pass the stack pointer value for inspecting the return address

        /* Hint: you can use REG_STACK_PTR */

        /*** TODO ***/
	}
}

VOID OnContextChange(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT *from, CONTEXT *to, INT32 info, VOID *v) {
    if (reason == CONTEXT_CHANGE_REASON_FATALSIGNAL) {
        ADDRINT eip;
        PIN_GetContextRegval(from, REG_INST_PTR, (UINT8*)&eip);
        std::cerr << "Unrecoverable error at instruction pointer: 0x" << std::hex << eip << std::endl;

        std::cerr << "Shadow stack:" << std::endl;
        int idx = shadow_stack_idx;
        while (idx) {
            // First frame sees a different treatment
            ADDRINT instr = (idx == shadow_stack_idx) ? eip : shadow_stack[idx].callsite;

            // GOAL: print stack entry for instr and any available debug symbol

            /* Hints:
               - use RTN_FindByAddress() for the symbol name
               - use PIN_UndecorateSymbolName() to demangle C++ symbols
               - use PIN_GetSourceLocation() to access debug information
            */

            /*** TODO ***/

            if (instr); // REMOVE THIS LINE (needed only to suppress warning)

            idx--;
        }
        std::cerr << "Exiting." << std::endl;
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize Pin
    PIN_Init(argc, argv);

    // Add instrumentation for instructions
    INS_AddInstrumentFunction(InstructionCallback, NULL);

    // Add callback for intercepting signals for the application
    PIN_AddContextChangeFunction(OnContextChange, NULL);

    // Start the program execution
    PIN_StartProgram();

    return 0;
}
