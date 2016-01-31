/*
 * $FILE: irqs.c
 *
 * IRQS' code
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
#include <bitwise.h>
#include <kdevice.h>
#include <irqs.h>
#include <kthread.h>
#include <physmm.h>
#include <processor.h>
#include <sched.h>
#include <stdc.h>

#ifdef CONFIG_VERBOSE_TRAP
xm_s8_t *trap2Str[]={
    __STR(DATA_STORE_ERROR), // 0
    __STR(INSTRUCTION_ACCESS_MMU_MISS), // 1
    __STR(INSTRUCTION_ACCESS_ERROR), // 2
    __STR(R_REGISTER_ACCESS_ERROR), // 3
    __STR(INSTRUCTION_ACCESS_EXCEPTION), // 4
    __STR(PRIVILEGED_INSTRUCTION), // 5
    __STR(ILLEGAL_INSTRUCTION), // 6
    __STR(FP_DISABLED), // 7
    __STR(CP_DISABLED), // 8
    __STR(UNIMPLEMENTED_FLUSH), // 9
    __STR(WATCHPOINT_DETECTED), // 10
    __STR(MEM_ADDRESS_NOT_ALIGNED), // 11
    __STR(FP_EXCEPTION), // 12
    __STR(CP_EXCEPTION), // 13
    __STR(DATA_ACCESS_ERROR), // 14
    __STR(DATA_ACCESS_MMU_MISS), // 15
    __STR(DATA_ACCESS_EXCEPTION),// 16
    __STR(TAG_OVERFLOW), // 17
    __STR(DIVISION_BY_ZERO), // 18
};
#endif


#ifdef CONFIG_SMP
RESERVE_HWIRQ(HALT_ALL_IPI_VECTOR);
RESERVE_HWIRQ(SCHED_PENDING_IPI_VECTOR);
#endif

extern localCpu_t localCpuInfo[CONFIG_NO_CPUS];

xm_s32_t ArchTrapIsSysCtxt(cpuCtxt_t *ctxt) {
//if not in system trap then return 0
    extern xm_u8_t WindowOverflowTrap[], EWindowOverflowTrap[];
    extern xm_u8_t WindowUnderflowTrap[], EWindowUnderflowTrap[];
    extern xm_u8_t SIRetCheckRetAddr[], EIRetCheckRetAddr[];

    if ((ctxt->pc>=(xmAddress_t)WindowOverflowTrap)&&(ctxt->pc < (xmAddress_t)EWindowOverflowTrap))
        return 0;

    if ((ctxt->pc>=(xmAddress_t)WindowUnderflowTrap)&&(ctxt->pc<(xmAddress_t)EWindowUnderflowTrap))
        return 0;

    if ((ctxt->pc>=(xmAddress_t)SIRetCheckRetAddr)&&(ctxt->pc<(xmAddress_t)EIRetCheckRetAddr))
        return 0;

    return 1;
}

static xm_s32_t SparcFpFault(cpuCtxt_t *ctxt, xm_u16_t *hmEvent) {
//disable
    *hmEvent=XM_HM_EV_SPARC_FP_DISABLED;
    return 0;
}

#if defined(CONFIG_MMU)
static xm_s32_t SparcTrapPageFault(cpuCtxt_t *ctxt, xm_u16_t *hmEvent) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xmAddress_t faultAddress=GetMmuFaultAddressReg();

    sched->cKThread->ctrl.g->partCtrlTab->arch.faultStatusReg=GetMmuFaultStatusReg();
    sched->cKThread->ctrl.g->partCtrlTab->arch.faultAddressReg=faultAddress;
    if (faultAddress>=CONFIG_XM_OFFSET)
        *hmEvent=XM_HM_EV_MEM_PROTECTION;

    return 0;
}
#endif

#ifdef CONFIG_SMP
static void SmpHaltAllHndl(cpuCtxt_t *ctxt, void *data) {
//TODO not implemented well
    HaltSystem();
}

static void SmpSchedPendingIPIHndl(cpuCtxt_t *ctxt, void *data) {
//
    localSched_t *sched=GET_LOCAL_SCHED();
    SetSchedPending();
    sched->data->cyclic.flags|=RESCHED_ENABLED;
}
#endif

xmAddress_t IrqVector2Address(xm_s32_t vector) {
//tbr; trap vector base address register
    localSched_t *sched=GET_LOCAL_SCHED();
    xmAddress_t *tbr=(xmAddress_t *)sched->cKThread->ctrl.g->partCtrlTab->arch.tbr;
    cpuCtxt_t *ctxt=sched->cKThread->ctrl.irqCpuCtxt;
    if (CheckGParam(tbr, 256*16, 4, 0)<0)
        PartitionPanic(ctxt, "Incorrect TBR address (0x%x)\n", tbr);
    //TODO why 4? sizeof trap vector?; 4 * 8 bit = 32 bit.
    return (xmAddress_t)&(tbr[vector*4]);
}

void ArchSetupIrqs(void) {
//InitPic: assign APIC Ops to hwIrqCtrl[e] function pointers;
    extern void InitPic(void);
#ifdef CONFIG_SMP
    int e;
#endif
    InitPic();

#ifdef CONFIG_SMP
    for (e=0; e<CONFIG_NO_CPUS; e++)
        localCpuInfo[e].globalIrqMask&=~(1<<HALT_ALL_IPI_VECTOR|1<<SCHED_PENDING_IPI_VECTOR);

    SetIrqHandler(HALT_ALL_IPI_VECTOR, SmpHaltAllHndl, 0);
    SetIrqHandler(SCHED_PENDING_IPI_VECTOR, SmpSchedPendingIPIHndl, 0);
#endif

     SetTrapHandler(7, SparcFpFault);

#if defined(CONFIG_MMU)
    SetTrapHandler(4, SparcTrapPageFault);
    SetTrapHandler(1, SparcTrapPageFault);
    SetTrapHandler(16, SparcTrapPageFault);
    SetTrapHandler(15, SparcTrapPageFault);
#endif
}
