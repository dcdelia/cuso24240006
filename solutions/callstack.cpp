#include "pin.H"
#include <iostream>
#include <signal.h>
#include <stdint.h>

#define STACK_MAX_DEPTH 1024

typedef struct {
    ADDRINT routine;    
    ADDRINT callsite;
    ADDRINT retaddr;
    ADDRINT* esp; // for debugging
} ss_node_t;

int shadow_stack_idx = 0;
ss_node_t shadow_stack[STACK_MAX_DEPTH];

VOID processCall(ADDRINT routine, ADDRINT callsite, ADDRINT retaddr, ADDRINT* esp) {
    ASSERT(shadow_stack_idx < STACK_MAX_DEPTH - 1, "Shadow stack is too small");
    shadow_stack_idx++;
    shadow_stack[shadow_stack_idx].routine = routine;
    shadow_stack[shadow_stack_idx].callsite = callsite;
    shadow_stack[shadow_stack_idx].retaddr = retaddr;
    shadow_stack[shadow_stack_idx].esp = esp - 1; // IPOINT_BEFORE and pointer arithmetics ;-)
}

VOID processRet(ADDRINT* esp) {
    ASSERT(shadow_stack_idx > 0, "Popping from an empty shadow stack");
    if (*esp != shadow_stack[shadow_stack_idx].retaddr || esp != shadow_stack[shadow_stack_idx].esp)
        std::cerr << "Misaligned stack? May need a stronger shadow stack implementation" << std::endl;
    shadow_stack_idx--;
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
            // First frame sees a different treatment
            ADDRINT instr = (idx == shadow_stack_idx) ? eip : shadow_stack[idx].callsite;
            
            // Access debug information if available
            INT32 column, line;
            std::string filename;
            RTN rtnCS = RTN_FindByAddress(instr);
            PIN_GetSourceLocation(instr, &column, &line, &filename);
            std::cerr << std::dec << "[" << shadow_stack_idx - idx << "] "
                      << std::hex << instr
                      << " " << (RTN_Valid(rtnCS) ? PIN_UndecorateSymbolName(RTN_Name(rtnCS), UNDECORATION_NAME_ONLY) : "UNK")
                      << " (" << filename << ":" << std::dec << line << ") "
                      << std::endl;
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