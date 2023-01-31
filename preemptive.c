#include <8051.h>
#include "preemptive.h"
        
extern void main(void);

void Bootstrap(void) {
    bitmap = 0;
    TMOD = 0;
    IE = 0x82;
    TR0 = 1;

    SemaphoreCreate(threadMutex, 1);
    SemaphoreCreate(threadEmpty, 4);
    SemaphoreCreate(threadFull, 0);

    SemaphoreWaitBody(threadEmpty, L(__COUNTER__));
    curThreadID = ThreadCreate(main);
    RESTORESTATE;
}

ThreadID ThreadCreate(FunctionPtr fp) {
    SemaphoreWaitBody(threadMutex, L(__COUNTER__));
    EA = 0;
    if(!(bitmap & 1)) {
        bitmap = bitmap | 1;
        newThread = 0;
    }
    else if(!(bitmap & 2)) {
        bitmap = bitmap | 2;
        newThread = 1;
    }
    else if(!(bitmap & 4)) {
        bitmap = bitmap | 4;
        newThread = 2;
    }
    else if(!(bitmap & 8)) {
        bitmap = bitmap | 8;
        newThread = 3;
    }

    tmpSP = SP;
    SP = ((newThread + 4) << 4) - 0x01;

    __asm
    PUSH DPL
    PUSH DPH
    __endasm;

    __asm
    ANL A, #0
    PUSH ACC
    PUSH ACC
    PUSH ACC
    PUSH ACC
    __endasm;

    PSW = newThread << 3;
    __asm
    PUSH PSW
    __endasm;
    
    savedSP[newThread] = SP;

    SP = tmpSP;
    EA = 1;
    SemaphoreSignal(threadMutex);
    SemaphoreSignal(threadFull);
    return newThread;
}

void ThreadExit(void) {
    SemaphoreWaitBody(threadFull, L(__COUNTER__));
    SemaphoreWaitBody(threadMutex, L(__COUNTER__));
    if(curThreadID == 0) 
        bitmap = bitmap & 14;
    else if(curThreadID == 1)
        bitmap = bitmap & 13;
    else if(curThreadID == 2)
        bitmap = bitmap & 11;
    else if(curThreadID == 3)
        bitmap = bitmap & 7;

    if(bitmap & 1)
        curThreadID = 0;
    else if(bitmap & 2)
        curThreadID = 1;
    else if(bitmap & 4)
        curThreadID = 2;
    else if(bitmap & 8)
        curThreadID = 3;
    
    RESTORESTATE;
    EA = 1;
    SemaphoreSignal(threadMutex);
    SemaphoreSignal(threadEmpty);
    if(bitmap == 0)
        while(1);
}

/*
void delay(unsigned char n) {
    __asm
        MOV R7, DPL
        MOV A, 0x35
        ADD A, 0X29
        MOV R1, A
        MOV A, R7
        ADD A, 0x3B
        MOV @R1, A
    DELAY:
        MOV A, 0x35
        ADD A, 0X29
        MOV R1, A
        MOV A, @R1
        CJNE A, 0x3B, DELAY 
    __endasm; 
}
*/

unsigned char now(void) {
    return currentTime;
}

void myTimer0Handler(void) {
    EA = 0;
    SAVESTATE;
    __asm
        MOV A, R0
        PUSH ACC
        MOV A, R1
        PUSH ACC
        MOV A, R2
        PUSH ACC
        MOV A, R3
        PUSH ACC
        MOV A, R4
        PUSH ACC
        MOV A, R5
        PUSH ACC
        MOV A, R6
        PUSH ACC
        MOV A, R7
        PUSH ACC
    __endasm;

    
    timerCnt = (timerCnt == 15) ? 0 : timerCnt + 1;
    currentTime = currentTime + (timerCnt == 14);

    curThreadID = (curThreadID == MAXTHREADS - 1) ? 0 : curThreadID + 1;
    while(!(bitmap & curThreadID) && tmpSP != curThreadID)
        curThreadID = (curThreadID == MAXTHREADS - 1) ? 0 : curThreadID + 1;

    __asm
        POP ACC
        MOV R7, A
        POP ACC
        MOV R6, A
        POP ACC
        MOV R5, A
        POP ACC
        MOV R4, A
        POP ACC
        MOV R3, A
        POP ACC
        MOV R2, A
        POP ACC
        MOV R1, A
        POP ACC
        MOV R0, A
    __endasm;
    RESTORESTATE;
    EA = 1;
    __asm
        RETI
    __endasm;
}
