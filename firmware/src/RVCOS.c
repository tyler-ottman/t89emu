#include <stdint.h>
#include "RVCOS.h"
#include <stdlib.h>

__attribute__((always_inline)) inline void enable_interrupts(void){
    asm volatile ("csrsi mstatus, 0x8");
}

__attribute__((always_inline)) inline void disable_interrupts(void){
    asm volatile ("csrci mstatus, 0x8");
}

volatile uint32_t* application_global;
volatile int cursor = 0;
volatile char *VIDEO_MEM = (volatile char *)(0x50000000 + 0xFE800);
volatile int mainThreadInitialized = 0;
uint32_t *InitStack(uint32_t *sp,TThreadEntry fun,void* param, TThreadID tp); // Discussion Code 10/22/21
//int get_tp(void);
void set_tp(uint32_t);
uint32_t call_on_other_gp(void *param, TThreadEntry entry, volatile uint32_t *gp);
void ContextSwitch(volatile uint32_t **oldsp, volatile uint32_t *newsp);
int removeThread(TThreadID tid); // Added here because function called above definition
extern volatile int global;

// Priority Queue For Each Type of Thread
volatile int HighPriorityQueue[256];
volatile int MedPriorityQueue[256];
volatile int LowPriorityQueue[256];

// Length of Each Priority Queue For Each Thread
volatile int HighPriorityQueueLength = 0;
volatile int MedPriorityQueueLength = 0;
volatile int LowPriorityQueueLength = 0;

// TCB Datastructure to store thread info
volatile struct TCB TCB_list[256];

int findEmptyTCB() {
    for(int i = 2; i < 256; i++) {
        if(TCB_list[i].isEmpty == 1) {
            return i;
        }
    }
    // Max threads reached
    return -1;
}

void *spin() {
    // Enable interrupts so timer interrupts can exit out of spin
    enable_interrupts();
    int temp = 0;
    while(1) {
        // spin while no threads to run
        if(temp) {
            temp = 0;
        } else {
            temp = 1;
        }
    }
}

void skeleton(void* param, TThreadEntry entry, TThreadID tid) {
    // switch back to application gp @640
    // call_on_other_gp(param, entry, application_global);

    enable_interrupts();

    // set return value of entry function if it returns to skeleton
    // TCB_list[tid].returnValue = entry(param);
    // switch back to application gp @640
    TCB_list[tid].returnValue = call_on_other_gp(param, entry, application_global);

    disable_interrupts();

    // Terminate if entry returns
    RVCThreadTerminate(tid, TCB_list[tid].returnValue);
}

TThreadID popThread(TThreadPriority prio) {
    // Pop first thread in given priority queue
    TThreadID poppedThread;
    switch(prio) {
        case(RVCOS_THREAD_PRIORITY_HIGH):
            poppedThread = HighPriorityQueue[0];
            break;
        case(RVCOS_THREAD_PRIORITY_NORMAL):
            poppedThread = MedPriorityQueue[0];
            break;
        case(RVCOS_THREAD_PRIORITY_LOW):
            poppedThread = LowPriorityQueue[0];
            break;
    }
    removeThread(poppedThread);

    return poppedThread;
}

int removeThread(TThreadID tid) {
    switch(TCB_list[tid].prio) {
        case(RVCOS_THREAD_PRIORITY_HIGH):
            for(int i = 0; i < HighPriorityQueueLength; i++) {
                if(HighPriorityQueue[i] == tid) {
                    // Thread ID found in queue, now resize queue
                    HighPriorityQueueLength--;
                    for(int j = i; j < HighPriorityQueueLength; j++) {
                        HighPriorityQueue[j] = HighPriorityQueue[j+1];
                    }
                    return 1;
                }
            }
            break;

        case(RVCOS_THREAD_PRIORITY_NORMAL):
            for(int i = 0; i < MedPriorityQueueLength; i++) {
                if(MedPriorityQueue[i] == tid) {
                    MedPriorityQueueLength--;
                    for(int j = i; j < MedPriorityQueueLength; j++) {
                        MedPriorityQueue[j] = MedPriorityQueue[j+1];
                    }
                    return 1;
                }
            }
            break;

        case(RVCOS_THREAD_PRIORITY_LOW):
            for(int i = 0; i < LowPriorityQueueLength; i++) {
                if(LowPriorityQueue[i] == tid) {
                    LowPriorityQueueLength--;
                    for(int j = i; j < LowPriorityQueueLength; j++) {
                        LowPriorityQueue[j] = LowPriorityQueue[j+1];
                    }
                    return 1;
                }
            }
            break;
    }
    // Thread not found in queue
    return 0;
}

void pushThread(TThreadID thread) {
    switch(TCB_list[thread].prio) {
        case(RVCOS_THREAD_PRIORITY_HIGH):
            HighPriorityQueue[HighPriorityQueueLength] = thread;
            HighPriorityQueueLength++;
            break;
        case(RVCOS_THREAD_PRIORITY_NORMAL):
            MedPriorityQueue[MedPriorityQueueLength] = thread;
            MedPriorityQueueLength++;
            break;
        case(RVCOS_THREAD_PRIORITY_LOW):
            LowPriorityQueue[LowPriorityQueueLength] = thread;
            LowPriorityQueueLength++;
            break;
    }
}

// @449
void scheduler(uint32_t isImmediate) {
    // schedule algorithm
    TThreadID currentThread = get_tp();
    // Thread that should be switched to
    TThreadID newThread;

    // Check if current thread yields time for remaining quantum
    if(isImmediate) {
        // Select thread in same priority queue
        newThread = popThread(TCB_list[currentThread].prio);
    } else {
        // Select thread in highest priority queue
        if(HighPriorityQueueLength > 0) {
            // Something exists in High Priority
            newThread = popThread(RVCOS_THREAD_PRIORITY_HIGH);
        } else if (MedPriorityQueueLength > 0) {
            // Nothing in high priority, find something in normal queue
            newThread = popThread(RVCOS_THREAD_PRIORITY_NORMAL);
        } else if (LowPriorityQueueLength > 0) {
            // Nothing in high or normal, find something in low queue
            newThread = popThread(RVCOS_THREAD_PRIORITY_LOW);
        } else {
            // All priority queues empty, select idle thread
            newThread = 1;
        }
    }

    // Store current thread ID in tp
    set_tp(newThread);

    // newThread selected, context switch
    // https://stackoverflow.com/questions/24515505/assignment-discards-volatile-qualifier-from-pointer-target-type
    ContextSwitch((volatile uint32_t**)&TCB_list[currentThread].stackPointer, TCB_list[newThread].stackPointer);

    // Context switch done, now run new thread
    // skeleton(TCB_list[newThread].param, TCB_list[newThread].entry, newThread);
}

// Help from Piazza post @338
TStatus RVCInitialize(uint32_t *gp) {

    // If main thread is already initialized, return error
    if(mainThreadInitialized) {
        return RVCOS_STATUS_ERROR_INVALID_STATE;
    }
    mainThreadInitialized = 1;
    
    // Make isEmpty 1 for all TCB structs
    for(int i = 0; i < 256; i++) {
        TCB_list[i].isEmpty = 1;
        TCB_list[i].waitingOn = -1; // initially no thread is waiting for another thread
        // TCB_list[i].returnValue = 0;
        TCB_list[i].ticksUntilReady = 0;
    }

    // Store gp in global variable for use in other system calls
    application_global = gp;
    
    // Initialize TCB for main thread
    TCB_list[0].prio = RVCOS_THREAD_PRIORITY_NORMAL; // Normal priority
    TCB_list[0].state = RVCOS_THREAD_STATE_RUNNING;
    TCB_list[0].isEmpty = 0;
    TCB_list[0].tid = 0;

    // Create TCB for idle thread
    TCB_list[1].prio = 0;
    TCB_list[1].state = RVCOS_THREAD_STATE_READY;
    TCB_list[1].isEmpty = 0;
    TCB_list[1].tid = 1;

    // Allocate idle thread stack for spin function
    TCB_list[1].baseOfStack = malloc((TMemorySize)2048);
    
    // Initialize idle thread stack with spin function
    // uint32_t *InitStack(uint32_t *sp, TThreadEntry fun, void* param, TThreadIDRef tp);
    TCB_list[1].stackPointer = InitStack((uint32_t*)(TCB_list[1].baseOfStack + 2048), (TThreadEntry)&spin, 0, TCB_list[1].tid);

    return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadCreate(TThreadEntry entry, void *param, TMemorySize memsize, TThreadPriority prio, TThreadIDRef tid) {

    // Check if thread can be created
    if (entry == 0 || tid == 0) {
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }
    int threadID = findEmptyTCB(); // check if TCB array is available
    if (threadID == -1) {
        // Max threads reached
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }

    TCB_list[threadID].isEmpty = 0;

    // Assign thread ID
    TCB_list[threadID].tid = threadID;
    // Store threadID in reference passed in by parameter tid
    *tid = threadID;

    // Assign priority
    TCB_list[threadID].prio = prio;

    // Pointer to entry function
    TCB_list[threadID].entry = entry;

    // Store thread param
    TCB_list[threadID].param = param;

    // Set state of thread to created
    TCB_list[threadID].state = RVCOS_THREAD_STATE_CREATED;

    // Store memsize of thread
    TCB_list[threadID].memsize = memsize;

    // Allocated Thread stack
    TCB_list[threadID].baseOfStack = malloc(memsize);

    // Thread successfully created
    return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadDelete(TThreadID thread) {
    // Check if thread exists
    if(TCB_list[thread].isEmpty == 1) {
        return RVCOS_STATUS_ERROR_INVALID_ID;
    } 

    // Check if thread is in correct state
    if (TCB_list[thread].state != RVCOS_THREAD_STATE_DEAD) {
        // Wrong state to delete thread
        return RVCOS_STATUS_ERROR_INVALID_STATE;
    }
    
    // Change isEmpty to 1
    TCB_list[thread].isEmpty = 1;

    // Free memory allocated to thread stack
    free(TCB_list[thread].baseOfStack);

    // Thread successfully deleted
    return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadActivate(TThreadID thread) {
    // Check if thread exists
    if(TCB_list[thread].isEmpty == 1) {
        return RVCOS_STATUS_ERROR_INVALID_ID;
    }
    // Check if thread is in correct state
    if(!(TCB_list[thread].state == RVCOS_THREAD_STATE_CREATED || TCB_list[thread].state == RVCOS_THREAD_STATE_DEAD)) {
        return RVCOS_STATUS_ERROR_INVALID_STATE;
    }


    // Initialize stack
    // uint32_t *InitStack(uint32_t *sp,TThreadEntry fun,uint32_t param, uint32_t tp);
    // Piaza post @338
    if(TCB_list[thread].state == RVCOS_THREAD_STATE_CREATED) {
        // only initial stack when thread is just created
        TCB_list[thread].stackPointer = InitStack((uint32_t*)(TCB_list[thread].baseOfStack + TCB_list[thread].memsize), (TThreadEntry)&skeleton, TCB_list[thread].param, TCB_list[thread].tid);
    }

    TCB_list[thread].state = RVCOS_THREAD_STATE_READY;

    // Add activated thread to priority queue
    pushThread(thread);

    // Call scheduler to determine if thread should run
    scheduler(0);

    return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadTerminate(TThreadID thread, TThreadReturn returnval){
    // check if thread exists
    if(TCB_list[thread].isEmpty == 1) {
        return RVCOS_STATUS_ERROR_INVALID_ID;
    }

    // Check if thread is in correct state
    if(!(TCB_list[thread].state == RVCOS_THREAD_STATE_CREATED || TCB_list[thread].state == RVCOS_THREAD_STATE_DEAD)) {
        return RVCOS_STATUS_ERROR_INVALID_STATE;
    }

    // Remove thread from queue if previously Ready
    if(TCB_list[thread].state == RVCOS_THREAD_STATE_READY) {
        removeThread(thread);
    }

    // Change state of thread to dead
    TCB_list[thread].state = RVCOS_THREAD_STATE_DEAD;
    TCB_list[thread].ticksUntilReady = 0; // In case thread was sleeping but is then terminated

    // Set return value
    TCB_list[thread].returnValue = returnval;

    for(int i = 2; i < 256; i++) {
        // Check if threads in TCB array are waiting on current thread
        if(TCB_list[i].state == RVCOS_THREAD_STATE_WAITING) {
            if(TCB_list[i].waitingOn == thread) {
                // store return value of currentthread to waiting thread
                *TCB_list[i].returnValueRef = returnval;
                // Change each waiting thread to ready
                TCB_list[i].state = RVCOS_THREAD_STATE_READY;
                pushThread(i);
            }
        }
    }

    scheduler(0);

    return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadWait(TThreadID thread, TThreadReturnRef returnref) {
    // check if thread exists
    if(TCB_list[thread].isEmpty == 1) {
        return RVCOS_STATUS_ERROR_INVALID_ID;
    }

    // Check if returnref is NULL
    if(!returnref) {
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }

    int currentThread = get_tp();

    // Change thread state to dead
    TCB_list[currentThread].state = RVCOS_THREAD_STATE_WAITING;

    // Make current thread wait on thread
    TCB_list[currentThread].waitingOn = TCB_list[thread].tid;

    // Save the return reference
    TCB_list[currentThread].returnValueRef = returnref;

    // call scheduler to find highest priority thread
    scheduler(0);

    return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadID(TThreadIDRef threadref) {
    // Check if pointer the threadref is NULL
    if(!threadref) {
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }

    // Get current thread ID through global variable
    int currentThread = get_tp();
    *(uint32_t*)threadref = currentThread;

    return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadState(TThreadID thread, TThreadStateRef stateref) {
    // Check if thread exists
    if(TCB_list[thread].isEmpty == 1) {
        return RVCOS_STATUS_ERROR_INVALID_ID;
    }
    // Check if stateref is NULL
    if(!stateref) {
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }
    // Otherwise store thread state in stateref
    *(uint32_t *)stateref = TCB_list[thread].state;

    return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadSleep(TTick tick) {
    if(tick == RVCOS_TIMEOUT_INFINITE) {
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }

    int currentThread = get_tp();
    // TCB_list[currentThread].state = RVCOS_THREAD_STATE_READY;

    // Determine if thread is immediate or not
    if(tick == RVCOS_TIMEOUT_IMMEDIATE) {
        // Change thread to ready on immediate
        TCB_list[currentThread].state = RVCOS_THREAD_STATE_READY;
        // Add thread to queue
        pushThread(TCB_list[currentThread].tid);
        // Call scheduler to run another thread
        scheduler(1);
    } else {
        // Put thread to sleep for tick ticks
        // Change state of current thread to waiting
        TCB_list[currentThread].state = RVCOS_THREAD_STATE_WAITING;
        // Set numbers of ticks for current thread to wait for
        TCB_list[currentThread].ticksUntilReady = tick;
        // Call scheduler to run another thread
        scheduler(0);
    }
    return RVCOS_STATUS_SUCCESS;
}

TStatus RVCTickMS(uint32_t *tickmsref) {
    // check if pointer to tickmsref if NULL
    if(!tickmsref) {
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }
    // Set tick time interval to 2 milliseconds
    *(uint32_t*)tickmsref = 2;

    return RVCOS_STATUS_SUCCESS;
}

TStatus RVCTickCount(TTickRef tickref) {
    if(!tickref) {
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }
    // store global tick counter in tickref
    *tickref = global;

    return RVCOS_STATUS_SUCCESS;
}

// Help from Piazza post @555
TStatus RVCWriteText(const TTextCharacter *buffer, TMemorySize writesize) {
    if(!buffer) {
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }
    for(int i = 0; i < writesize; i++) {
        if(cursor == 36*64 && buffer[i] != '\b') {
            // End of display
            continue;
        }
        if(buffer[i] == '\b' && cursor != 0) {
            VIDEO_MEM[cursor - 1] = ' ';
            cursor--;
        } else if(buffer[i] == '\b' && cursor == 0) {
            continue;
        } else if(buffer[i] == '\n' && cursor + 63 < (36*64)) {
            // VIDEO_MEM[i - 1 + cursor] = ' ';
            cursor += 64 - cursor % 64;
        } else if (buffer[i] == '\n' && cursor + 63 >= (36*64)) {
            continue;
        } else if (buffer[i] == '\r') {
            cursor -= cursor % 64;
            continue;
        } else {
            VIDEO_MEM[cursor] = buffer[i];
            cursor += 1;
        }
    }
    //cursor += tempCursor;
    return RVCOS_STATUS_SUCCESS;
}

TStatus RVCReadController(SControllerStatusRef statusref) {
    // Check if statusref is null
    if(!statusref) {
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }

    // Piazza @410
    *(uint32_t *)statusref = (*((volatile uint32_t *)0x40000018));
    
    return RVCOS_STATUS_SUCCESS;

}

uint32_t *InitStack(uint32_t *sp, TThreadEntry fun, void* param, TThreadID tp){
    sp--;
    *sp = (uint32_t)fun; //sw      ra,48(sp)
    sp--;
    *sp = tp;//sw      tp,44(sp)
    sp--;
    *sp = 0;//sw      t0,40(sp)
    sp--;
    *sp = 0;//sw      t1,36(sp)
    sp--;
    *sp = 0;//sw      t2,32(sp)
    sp--;
    *sp = 0;//sw      s0,28(sp)
    sp--;
    *sp = 0;//sw      s1,24(sp)
    sp--;
    *sp = *(uint32_t*)param;//sw      a0,20(sp)
    sp--;
    *sp = 0;//sw      a1,16(sp)
    sp--;
    *sp = 0;//sw      a2,12(sp)
    sp--;
    *sp = 0;//sw      a3,8(sp)
    sp--;
    *sp = 0;//sw      a4,4(sp)
    sp--;
    *sp = 0;//sw      a5,0(sp)
    return sp;
}