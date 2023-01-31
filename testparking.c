#include <8051.h>
#include "preemptive.h"

__data __at (0x20) char spot1;
__data __at (0x21) char spot2;
__data __at (0x22) char mutex;
__data __at (0x23) char empty;
__data __at (0x24) char full;
__data __at (0x25) char cars[4];
__data __at (0x29) char carID;

void Park(void) {
    SemaphoreWaitBody(empty, L(__COUNTER__));
    SemaphoreWaitBody(mutex, L(__COUNTER__));
    EA = 0;
    if(spot1 == '-') {
        spot1 = cars[curThreadID];
        showTime();
        In(spot1, '1');
    }
    else if(spot2 == '-') {
        spot2 = cars[curThreadID];
        showTime();
        In(spot2, '2');
    }
    EA = 1;
    SemaphoreSignal(mutex);
    SemaphoreSignal(full);

    delay(3);

    SemaphoreWaitBody(full, L(__COUNTER__));
    SemaphoreWaitBody(mutex, L(__COUNTER__));
    EA = 0;
    if(spot1 == cars[curThreadID]) {
        showTime();
        Out(spot1);
        spot1 = '-';
    }
    else if(spot2 == cars[curThreadID]) {
        showTime();
        Out(spot2);
        spot2 = '-';
    }
    SemaphoreSignal(mutex);
    SemaphoreSignal(empty);
    EA = 1;

    ThreadExit();
}

void main(void) {
    spot1 = '-';
    spot2 = '-';
    carID = '1';
    SemaphoreCreate(mutex, 1);
    SemaphoreCreate(empty, 2);
    SemaphoreCreate(full, 0);
   
    while(1) {
        SemaphoreWaitBody(threadEmpty, L(__COUNTER__));
        cars[ThreadCreate(Park)] = carID;
        carID = (carID == '5') ? '1' : carID + 1;
    }

    ThreadExit();
}

void _sdcc_gsinit_startup(void) {
    __asm
            ljmp  _Bootstrap
    __endasm;
}

void _mcs51_genRAMCLEAR(void) {}
void _mcs51_genXINIT(void) {}
void _mcs51_genXRAMCLEAR(void) {}

void timer0_ISR(void) __interrupt(1) {
    __asm
        ljmp _myTimer0Handler
    __endasm;
}