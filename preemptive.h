#ifndef __PREEMPTIVE_H__
#define __PREEMPTIVE_H__

#define MAXTHREADS 4
#define L(x) LABEL(x)
#define LABEL(x) x ## $

typedef char ThreadID;
typedef void (*FunctionPtr)(void);


__data __at (0x30) char savedSP[MAXTHREADS];
__data __at (0x34) char bitmap;
__data __at (0x35) char curThreadID;
__data __at (0x36) char tmpSP;
__data __at (0x37) char newThread;
__data __at (0x38) char threadFull;
__data __at (0x39) char threadEmpty;
__data __at (0x3A) char threadMutex;
__data __at (0x3B) unsigned char currentTime;
__data __at (0x3C) unsigned char timerCnt;
__data __at (0x2A) unsigned char targetTime[MAXTHREADS];

#define CNAME(s) _ ## s

#define SemaphoreCreate(s, n) \
                s = n \

#define SemaphoreWaitBody(s, label) \
               { __asm \
          label: \
                MOV A, CNAME(s) \
                JZ label \
                JB ACC.7, label \
                dec CNAME(s) \
                __endasm; } 

#define SemaphoreSignal(s) \
                __asm \
                INC CNAME(s) \
                __endasm \
                
#define delay(n) \
      DPL = n; \
      DPH = 0X2A; \
      __asm \
            MOV R7, DPL \
            MOV A, 0x35 \
            ADD A, DPH \
            MOV R1, A \
            MOV A, R7 \
            ADD A, 0x3B \
            MOV @R1, A \
      DELAY: \
            MOV A, 0x35 \
            ADD A, DPH \
            MOV R1, A \
            MOV A, @R1 \
            CJNE A, 0x3B, DELAY \
      __endasm; \

#define In(id, s) \
            TMOD |= 0x20; \
            TH1 = -6; \
            SCON = 0x50; \
            TR1 = 1; \
            SBUF = id; \
            while(!TI); \
            TI = 0; \
            SBUF = ' '; \
            while(!TI); \
            TI = 0; \
            SBUF = 'i'; \
            while(!TI); \
            TI = 0; \
            SBUF = 'n'; \
            while(!TI); \
            TI = 0; \
            SBUF = ' '; \
            while(!TI); \
            TI = 0; \
            SBUF = s; \
            while(!TI); \
            TI = 0; \
            SBUF = '\n'; \
            while(!TI); \
            TI = 0; \

#define Out(id) \
            TMOD |= 0x20; \
            TH1 = -6; \
            SCON = 0x50; \
            TR1 = 1; \
            SBUF = id; \
            while(!TI); \
            TI = 0; \
            SBUF = ' '; \
            while(!TI); \
            TI = 0; \
            SBUF = 'o'; \
            while(!TI); \
            TI = 0; \
            SBUF = 'u'; \
            while(!TI); \
            TI = 0; \
            SBUF = 't'; \
            while(!TI); \
            TI = 0; \
            SBUF = '\n'; \
            while(!TI); \
            TI = 0; \

#define showTime() \
            TMOD |= 0x20; \
            TH1 = -6; \
            SCON = 0x50; \
            TR1 = 1; \
            SBUF = (currentTime / 10 % 10) + '0'; \
            while(!TI); \
            TI = 0; \
            SBUF = (currentTime % 10) + '0'; \
            while(!TI); \
            TI = 0; \
            SBUF = ':'; \
            while(!TI); \
            TI = 0; \
            SBUF = ' '; \
            while(!TI); \
            TI = 0; \

#define SAVESTATE \
            __asm \
            PUSH ACC \
            PUSH B \
            PUSH DPL \
            PUSH DPH \
            PUSH PSW \
            __endasm; \
            savedSP[curThreadID] = SP; \
        
#define RESTORESTATE \
            SP = savedSP[curThreadID]; \
            __asm \
            POP PSW \
            POP DPH \
            POP DPL \
            POP B \
            POP ACC \
            __endasm; \

ThreadID ThreadCreate(FunctionPtr fp);
void ThreadYield(void);
void ThreadExit(void);
void myTimer0Handler(void);
//void delay(unsigned char n);
unsigned char now(void);
#endif /*__PREEMPTIVE_H__*/