/*
 * $FILE: kthread.c
 *
 * Kernel context (ARCH dependent part)
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
#include <gaccess.h>
#include <kthread.h>
#include <processor.h>
#include <sched.h>
#include <smp.h>
#include <spinlock.h>
#include <stdc.h>
#include <arch/xm_def.h>

#ifdef CONFIG_FLUSH_CACHE_AFTER_CS
static kThread_t *cSPrevPart=0;
#endif

void SwitchKThreadArchPre(kThread_t *new, kThread_t *current) {
    ASSERT(!HwIsSti());

#ifdef CONFIG_MMU
    if (new->ctrl.g) {
        if (new->ctrl.g->kArch.ptdL1) {
            SetMmuCtxt(new->ctrl.g->kArch.mmuCtxt);
            FlushTlbCtxt();
        }
    }
#else
    if (new->ctrl.g) {
	StoreIoReg(LEON_MEMORY_WPR_BASE+WPR1_REG, new->ctrl.g->kArch.wpReg);
    }
#endif
#ifdef CONFIG_FLUSH_CACHE_AFTER_CS
    if (current->ctrl.g)
	cSPrevPart=current;
#endif
}

void SwitchKThreadArchPost(kThread_t *current) {
//called by StartUpGuest and schedule()-> if newK != cKThread (CS)
    xm_u32_t cache=0;
    ASSERT(!HwIsSti());
    // Disabling FPU
    HwDisableFpu();
    if (current->ctrl.g) {
        cpuCtxt_t *cpuCtxt=current->ctrl.irqCpuCtxt;
        if (cpuCtxt) {
            ASSERT((cpuCtxt->psr&0xff000000)==(GetPsr()&0xff000000));
            //EF bit Set if a floating-point coprocessor is available
            cpuCtxt->psr&=~PSR_EF_BIT;
        }
    }
    // Flushing the cache

    //TODO do we need to check cache enable first?
#ifdef CONFIG_FLUSH_CACHE_AFTER_CS
    if (current->ctrl.g&&(current!=cSPrevPart))
        SetKThreadFlags(current, KTHREAD_FLUSH_DCACHE_F|KTHREAD_FLUSH_ICACHE_F);
#endif
    if (AreKThreadFlagsSet(current, KTHREAD_DCACHE_ENABLED_F))
        cache|=DCACHE;

    if (AreKThreadFlagsSet(current, KTHREAD_DCACHE_ENABLED_F))
        cache|=ICACHE;

    SetCacheState(cache);
}

void KThreadArchInit(kThread_t *k) {
//TODO empty
}

void SetupKThreadArch(kThread_t *k) {
//TODO empty
}

void SetupKStack(kThread_t *k, void *StartUp, xmAddress_t entryPoint) {
    extern xm_u32_t ArchStartupGuest;
    k->ctrl.kStack=(xm_u32_t *)&k->kStack[CONFIG_KSTACK_SIZE-MIN_STACK_FRAME-8];
    *--(k->ctrl.kStack)=(xm_u32_t)0; /* o1 */
    *--(k->ctrl.kStack)=(xm_u32_t)entryPoint; /* o0 */
    *--(k->ctrl.kStack)=(xm_u32_t)&ArchStartupGuest;  /* %g5 */
    *--(k->ctrl.kStack)=(xm_u32_t)StartUp;  /* %g4 */
    *--(k->ctrl.kStack)=(xm_u32_t)GetPsr()&~(PSR_CWP_MASK|PSR_ICC_MASK); /*  %PSR (%g7) */
    *--(k->ctrl.kStack)=(xm_u32_t)2; /* %WIM (%g6) */
}

void SetupPctArch(partitionControlTable_t *partCtrlTab, kThread_t *k) {
    xm_s32_t e;
    partCtrlTab->trap2Vector[0]=DATA_STORE_ERROR;
    partCtrlTab->trap2Vector[1]=INSTRUCTION_ACCESS_MMU_MISS;
    partCtrlTab->trap2Vector[2]=INSTRUCTION_ACCESS_ERROR;
    partCtrlTab->trap2Vector[3]=R_REGISTER_ACCESS_ERROR;
    partCtrlTab->trap2Vector[4]=INSTRUCTION_ACCESS_EXCEPTION;
    partCtrlTab->trap2Vector[5]=PRIVILEGED_INSTRUCTION;
    partCtrlTab->trap2Vector[6]=ILLEGAL_INSTRUCTION;
    partCtrlTab->trap2Vector[7]=FP_DISABLED;
    partCtrlTab->trap2Vector[8]=CP_DISABLED;
    partCtrlTab->trap2Vector[9]=UNIMPLEMENTED_FLUSH;
    partCtrlTab->trap2Vector[10]=WATCHPOINT_DETECTED;
    partCtrlTab->trap2Vector[11]=MEM_ADDRESS_NOT_ALIGNED;
    partCtrlTab->trap2Vector[12]=FP_EXCEPTION;
    partCtrlTab->trap2Vector[13]=CP_EXCEPTION;
    partCtrlTab->trap2Vector[14]=DATA_ACCESS_ERROR;
    partCtrlTab->trap2Vector[15]=DATA_ACCESS_MMU_MISS;
    partCtrlTab->trap2Vector[16]=DATA_ACCESS_EXCEPTION;
    partCtrlTab->trap2Vector[17]=TAG_OVERFLOW;
    partCtrlTab->trap2Vector[18]=DIVISION_BY_ZERO;

    partCtrlTab->hwIrq2Vector[1]=INTERRUPT_LEVEL_1; // AMBA
    partCtrlTab->hwIrq2Vector[2]=INTERRUPT_LEVEL_2; // UART2
    partCtrlTab->hwIrq2Vector[3]=INTERRUPT_LEVEL_3; // UART1
    partCtrlTab->hwIrq2Vector[4]=INTERRUPT_LEVEL_4; // IO0
    partCtrlTab->hwIrq2Vector[5]=INTERRUPT_LEVEL_5; // IO1
    partCtrlTab->hwIrq2Vector[6]=INTERRUPT_LEVEL_6; // IO2
    partCtrlTab->hwIrq2Vector[7]=INTERRUPT_LEVEL_7; // IO3
    partCtrlTab->hwIrq2Vector[8]=INTERRUPT_LEVEL_8; // TIMER1
    partCtrlTab->hwIrq2Vector[9]=INTERRUPT_LEVEL_9; // TIMER2
    partCtrlTab->hwIrq2Vector[10]=INTERRUPT_LEVEL_10; // UNUSED
    partCtrlTab->hwIrq2Vector[11]=INTERRUPT_LEVEL_11; // DSU
    partCtrlTab->hwIrq2Vector[12]=INTERRUPT_LEVEL_12; // UNUSED
    partCtrlTab->hwIrq2Vector[13]=INTERRUPT_LEVEL_13; // UNUSED
    partCtrlTab->hwIrq2Vector[14]=INTERRUPT_LEVEL_14; // DSU
    partCtrlTab->hwIrq2Vector[15]=INTERRUPT_LEVEL_15; // UNUSED

    for (e=0; e<XM_VT_EXT_MAX; e++)
	partCtrlTab->extIrq2Vector[e]=224+e;

    if (KTHREAD_FP_F & partCtrlTab->flags)
       partCtrlTab->iFlags|=PSR_EF_BIT;
}
