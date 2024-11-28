#include "pin.H"
#include <iostream>
#include <signal.h>
#include <stdint.h>

#define STACK_MAX_DEPTH 1024

typedef struct {
    ADDRINT routine;    
    ADDRINT callsite;
    ADDRINT retaddr;
    ADDRINT* esp;
} ss_node_t;

int shadow_stack_idx = 0;
ss_node_t shadow_stack[STACK_MAX_DEPTH];

VOID processCall(ADDRINT routine, ADDRINT callsite, ADDRINT retaddr, ADDRINT* esp) {
    // TODO
    ASSERT(shadow_stack_idx < STACK_MAX_DEPTH - 1, "Shadow stack is too small");
    shadow_stack_idx++;
    shadow_stack[shadow_stack_idx].routine = routine;
    shadow_stack[shadow_stack_idx].callsite = callsite;
    shadow_stack[shadow_stack_idx].retaddr = retaddr;
    shadow_stack[shadow_stack_idx].esp = esp - 1; // watch out for pointer arithmetics ;-)
}

VOID processRet(ADDRINT* esp) {
    ASSERT(shadow_stack_idx > 0, "Underflow on shadow stack");
    /*while (shadow_stack_idx > 1 && esp > shadow_stack[shadow_stack_idx].esp) {
        std::cerr << "Popping stale frame" << std::endl;
        --shadow_stack_idx;
        ASSERT(shadow_stack_idx>0, "Underflow two on shadow stack");
    }*/
    if (*esp != shadow_stack[shadow_stack_idx].retaddr || esp != shadow_stack[shadow_stack_idx].esp)
        std::cerr << "Misaligned stack? " << std::endl;
    shadow_stack_idx--;
    // TODO
}

void InstructionCallback(INS ins, void *v) {
    if (INS_IsCall(ins)) {
		if (INS_IsDirectCall(ins)) {
			INS_InsertCall(ins,
				IPOINT_BEFORE,
				(AFUNPTR)processCall,
                IARG_ADDRINT, INS_DirectControlFlowTargetAddress(ins), // Target routine
				IARG_INST_PTR, // Call site
				IARG_ADDRINT, INS_NextAddress(ins), // Return address
				IARG_REG_VALUE, REG_STACK_PTR, // SP before call executes
				IARG_END);
		}
		else {
			INS_InsertCall(
				ins,
				IPOINT_BEFORE,
				(AFUNPTR)processCall,
                IARG_BRANCH_TARGET_ADDR, // Target routine
				IARG_INST_PTR, // Call site
				IARG_ADDRINT, INS_NextAddress(ins), // Return address
				IARG_REG_VALUE, REG_STACK_PTR, // SP before call executes
				IARG_END);
		}
	} else if (INS_IsRet(ins)) {
		INS_InsertCall(ins,
			IPOINT_BEFORE,
			(AFUNPTR)processRet,
			IARG_REG_VALUE, REG_STACK_PTR, // SP before ret execution
			IARG_END);
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
            ADDRINT instr, fun;
            if (idx == shadow_stack_idx) {
                instr = eip;
                fun = shadow_stack[idx].routine;
            } else {
                instr = shadow_stack[idx].callsite;
                fun = shadow_stack[idx+1].routine;
            }
            INT32 column, line;
            std::string filename;
            RTN rtnSrc = RTN_FindByAddress(fun);
            PIN_GetSourceLocation(instr, &column, &line, &filename);
            std::cerr << std::dec << "[" << shadow_stack_idx - idx << "] "
                      << std::hex << instr
                      << " " << (RTN_Valid(rtnSrc) ? RTN_Name(rtnSrc) : "UNK")
                      << " (" << filename
                      << ":" << std::dec << line
                      << ")" << std::endl;
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