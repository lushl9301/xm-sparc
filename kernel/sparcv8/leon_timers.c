/*
 * $FILE: leon_timers.c
 *
 * LEON's timers
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#include <assert.h>
#include <kdevice.h>
#include <ktimer.h>
#include <stdc.h>
#include <processor.h>
#include <arch/irqs.h>
#include <arch/xm_def.h>

#if defined(CONFIG_LEON3) //******* LEON3 ********

#define PIT1_IRQ TIMER1_TRAP_NR
#define PIT2_IRQ TIMER2_TRAP_NR

#define CLOCK_IRQ (PIT1_IRQ)
#define TIMER_IRQ (PIT2_IRQ+GET_CPU_ID())

RESERVE_HWIRQ(TIMER1_TRAP_NR);
RESERVE_HWIRQ(TIMER2_TRAP_NR);
RESERVE_IOPORTS(LEON_TIMER1_BASE, 3);
RESERVE_IOPORTS(LEON_TIMER2_BASE, 3);
RESERVE_IOPORTS(LEON_TIMER_CFG_BASE, 2);

#define TIMC_REG 0x0

#define TIMR_REG 0x4
  #define TIM_RLD_MASK 0xFFFFFFFF
  #define TIM_RLD_MASK_BITS 32

#define TIMCTR_REG 0x8
  #define LD_BIT 0x4
  #define RL_BIT 0x2
  #define EN_BIT 0x1
////////////
  #define IE_BIT 0x8
////////////


#define SCAC_REG 0x0
  #define SCAC_CTR_VAL_MASK 0x3FF

#define SCAR_RLD_REG 0x4

#define SCAR_REG 0x4
  #define SCAC_RLD_VAL_MASK 0x3FF

#define WDG_REG 0x0
  #define WDC_MASK 0x00FFFFFF

#define LEON_GPTIMER_CFG_BASE LEON_TIMER_CFG_BASE
#define LEON_CLOCK_BASE LEON_TIMER1_BASE
#define LEON_TIMER_BASE LEON_TIMER2_BASE
#define TIMER_RLD_REG TIMR_REG
#define TIMER_CTRL_REG TIMCTR_REG


#endif

#define GET_TIMER() (GET_CPU_ID()*0x10)


static hwTimer_t pitTimer[CONFIG_NO_CPUS];
static timerHandler_t pitHandler[CONFIG_NO_CPUS];

static void TimerIrqHandler(cpuCtxt_t *ctxt, void *irqData) {
    if (pitHandler[GET_CPU_ID()])
        (*pitHandler[GET_CPU_ID()])();
    HwEnableIrq(TIMER_IRQ);
}


static xm_s32_t InitPitTimer(void) {
    localCpu_t *cpu=GET_LOCAL_CPU();

    // Programming LEON's preescaler to LEON_PRESCALER_KHZ

    pitTimer[GET_CPU_ID()].freqKhz=LEON_PRESCALER_KHZ;
    cpu->globalIrqMask&=~(1<<TIMER_IRQ);
    SetIrqHandler(TIMER_IRQ, TimerIrqHandler, 0);

    StoreIoReg(LEON_TIMER_BASE+TIMER_RLD_REG+GET_TIMER(), 0);
    StoreIoReg(LEON_TIMER_BASE+TIMER_CTRL_REG+GET_TIMER(), 0);
    HwEnableIrq(TIMER_IRQ);
    pitTimer[GET_CPU_ID()].flags|=HWTIMER_ENABLED|PER_CPU;

    return 1;
}

static void SetPitTimer(xmTime_t interval) {
    StoreIoReg(LEON_TIMER_BASE+TIMER_RLD_REG+GET_TIMER(), interval&TIM_RLD_MASK);
    StoreIoReg(LEON_TIMER_BASE+TIMER_CTRL_REG+GET_TIMER(), LD_BIT|EN_BIT|IE_BIT);
}

static xmTime_t GetMaxIntervalPit(void) {
    return 1000000LL; // 1s
}

static xmTime_t GetMinIntervalPit(void) {
    return 50LL; // 50usec
}

static timerHandler_t SetTimerHandlerPit(timerHandler_t TimerHandler) {
    timerHandler_t OldPitUserHandler=pitHandler[GET_CPU_ID()];
  
    pitHandler[GET_CPU_ID()]=TimerHandler;
    return OldPitUserHandler;
}

static void ShutdownPitTimer(void) {
    pitTimer[GET_CPU_ID()].flags&=~HWTIMER_ENABLED;
}

static hwTimer_t pitTimer[]={[0 ... CONFIG_NO_CPUS-1]={
        .name="LEON timer",
        .flags=0,
        .InitHwTimer=InitPitTimer,
        .SetHwTimer=SetPitTimer,
        .GetMaxInterval=GetMaxIntervalPit,
        .GetMinInterval=GetMinIntervalPit,
        .SetTimerHandler=SetTimerHandlerPit,
        .ShutdownHwTimer=ShutdownPitTimer,
    }
};

static hwClock_t pitClock;

#ifdef CONFIG_LEON3

static struct pitClockData {
    volatile xm_u32_t ticks;
} pitClockData={
    .ticks=0,
};

static void ClockIrqHandler(cpuCtxt_t *ctxt, void *irqData) {
    pitClockData.ticks++;
}

static xm_s32_t InitPitClock(void) {
    localCpu_t *cpu=GET_LOCAL_CPU();
    // Programming LEON's preescaler to LEON_PRESCALER_KHZ
    StoreIoReg(LEON_GPTIMER_CFG_BASE+SCAR_RLD_REG, ((cpuKhz/LEON_PRESCALER_KHZ)-1)&SCAC_RLD_VAL_MASK);
    pitClock.freqKhz=LEON_PRESCALER_KHZ;
    cpu->globalIrqMask&=~(1<<CLOCK_IRQ);
    StoreIoReg(LEON_CLOCK_BASE+TIMER_RLD_REG, 0);
    StoreIoReg(LEON_CLOCK_BASE+TIMER_CTRL_REG, 0);

    SetIrqHandler(CLOCK_IRQ, ClockIrqHandler, 0);

    StoreIoReg(LEON_CLOCK_BASE+TIMER_RLD_REG, TIM_RLD_MASK);
    StoreIoReg(LEON_CLOCK_BASE+TIMER_CTRL_REG, LD_BIT|RL_BIT|EN_BIT|IE_BIT);
    HwEnableIrq(CLOCK_IRQ);
    pitClock.flags|=HWCLOCK_ENABLED;

    return 1;
}
#endif

#define ITP_REG 0x4
#define ITC_REG 0xC

#include <spinlock.h>

static hwTime_t ReadPitClock(void) {
#ifdef CONFIG_DEBUG
//    static spinLock_t clockLock=SPINLOCK_INIT;
    static hwTime_t lastVal[CONFIG_NO_CPUS];
#endif
    hwTime_t cTime;
#ifdef CONFIG_LEON3
    hwTime_t t;

    if (!(pitClock.flags&HWCLOCK_ENABLED))
        return 0;

    // I. Ripoll's reading clock algorithm, don't modify
    cTime=TIM_RLD_MASK-LoadIoReg(LEON_CLOCK_BASE+TIMC_REG);

    // Checking timer1/clock overflow
    if (LoadIoReg(LEON_PIC_BASE+ITP_REG)&(1<<CLOCK_IRQ)) {
        StoreIoReg(LEON_PIC_BASE+ITC_REG, 1<<CLOCK_IRQ);
        pitClockData.ticks++;
        cTime=TIM_RLD_MASK-LoadIoReg(LEON_CLOCK_BASE+TIMC_REG);
    }

    t=pitClockData.ticks;
    cTime+=(t<<TIM_RLD_MASK_BITS);
#endif

#ifdef CONFIG_DEBUG
//    SpinLock(&clockLock);
    ASSERT(cTime>=lastVal[GET_CPU_ID()]);
    lastVal[GET_CPU_ID()]=cTime;
//    SpinUnlock(&clockLock);
#endif

    return cTime;
}


static void ShutdownPitClock(void) {
#ifdef CONFIG_LEON3
    StoreIoReg(LEON_CLOCK_BASE+TIMCTR_REG, 0);
    HwDisableIrq(CLOCK_IRQ);
#endif
}

static hwClock_t pitClock={
    .name="LEON clock",
    .flags=0,
    .InitClock=InitPitClock,
    .GetTimeUsec=ReadPitClock,
    .ShutdownClock=ShutdownPitClock,
};

hwClock_t *sysHwClock=&pitClock;

hwTimer_t *GetSysHwTimer(void) {
    return &pitTimer[GET_CPU_ID()];
}


