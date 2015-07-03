/*
 * $FILE: hypercalls.c
 *
 * XM's hypercalls
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
#include <spinlock.h>
#include <hypercalls.h>
#include <sched.h>

__hypercall xm_s32_t SparcAtomicAddSys(xm_u32_t *__gParam addr, xm_u32_t val) {
    if (CheckGParam(addr, sizeof(xm_u32_t), 4, PFLAG_RW|PFLAG_NOT_NULL)<0)
        return XM_INVALID_PARAM;
    *addr=*addr+val;
    return XM_OK;
}

__hypercall xm_u32_t SparcAtomicAndSys(xm_u32_t *__gParam addr, xm_u32_t val) {
    if (CheckGParam(addr, sizeof(xm_u32_t), 4, PFLAG_RW|PFLAG_NOT_NULL)<0)
        return XM_INVALID_PARAM;
    *addr=*addr&val;
    return XM_OK;
}

__hypercall xm_u32_t SparcAtomicOrSys(xm_u32_t *__gParam addr, xm_u32_t val) {
    if (CheckGParam(addr, sizeof(xm_u32_t), 4, PFLAG_RW|PFLAG_NOT_NULL)<0)
        return XM_INVALID_PARAM;
    *addr=*addr|val;
    return XM_OK;
}

static inline xm_u32_t IoPortLogSearch(xm_u32_t port) {
    localSched_t *sched=GET_LOCAL_SCHED();
    struct xmcIoPort *ioPort;
    xm_s32_t s, m, e;
    xm_u32_t a0, a1;

    for (s=0, e=GetPartition(sched->cKThread)->cfg->noIoPorts, m=(s+e)>>1; (m<e); m=(s+e)>>1) {
	ioPort=&xmcIoPortTab[m+GetPartition(sched->cKThread)->cfg->ioPortsOffset];
	if (ioPort->type==XM_RESTRICTED_IOPORT) {
	    a0=ioPort->restricted.address;
	    a1=a0+sizeof(xm_u32_t);
	    if (port>=a0&&port<a1)
		return ioPort->restricted.mask;
	} else {
	    a0=ioPort->range.base;
	    a1=a0+sizeof(xm_u32_t)*ioPort->range.noPorts;
	    if (port>=a0&&port<a1)
		return ~0UL;
	}
	if (a0>port) 
	    e=m;
	else 
	    s=m+1;
    }
    
    return 0;
}

__hypercall xm_s32_t SparcIoOutportSys(xm_u32_t port, xm_u32_t value) {
    xm_u32_t mask=0, oldValue;

    ASSERT(!HwIsSti());
    if (port&0x3) return XM_INVALID_PARAM;
    if (!(mask=IoPortLogSearch(port)))
	return XM_PERM_ERROR;
    
    if (mask==~0UL) {  /* If no mask then write directly into the port */
	StoreIoReg(port, value);
    } else {           /* Otherwise read first the value of the port */
	/* <track id="restricted-port-mask"> */
	oldValue=LoadIoReg(port);
	StoreIoReg(port, ((oldValue&~(mask))|(value&mask)));
	/* </track id="restricted-port-mask"> */
    }
    return XM_OK;
}

__hypercall xm_u32_t SparcIoInportSys(xm_u32_t port, xm_u32_t *__gParam value) {
    xm_u32_t mask=0;
    
    ASSERT(!HwIsSti());
    if (CheckGParam(value, 4, sizeof(xm_u32_t), 1)<0)
        return XM_INVALID_PARAM;
    
    if (!value) return XM_INVALID_PARAM;
    if (port&0x3) return XM_INVALID_PARAM;
    if (!(mask=IoPortLogSearch(port)))
	return XM_PERM_ERROR;
    
    *value=(LoadIoReg(port)&mask); 

    return XM_OK;
}

__hypercall void SparcWriteTbrSys(xmWord_t val) {
    localSched_t *sched=GET_LOCAL_SCHED();
    sched->cKThread->ctrl.g->kArch.tbr=val;
    sched->cKThread->ctrl.g->partCtrlTab->arch.tbr=val;
}

__hypercall xm_s32_t OverrideTrapHndlSys(xm_s32_t entry, struct trapHandler *__gParam handler) {
    return XM_OK;
}

#ifdef CONFIG_MMU
__hypercall xm_s32_t SparcWritePtdL1Sys(xmWord_t val) {
#if 0
    localSched_t *sched=GET_LOCAL_SCHED();
    struct physPage *ptdL1Page;
    
    if (!IS_PTD_PRESENT(val))
	return XM_INVALID_PARAM;

    if (!(ptdL1Page=PmmFindPage(GET_PTD_ADDR(val), sched->cKThread, 0)))
	return XM_INVALID_PARAM;

    if (ptdL1Page->type!=PPAG_PTD1) {
	PWARN("Page %x is not PTD1\n", GET_PTD_ADDR(val));
	return XM_INVALID_PARAM;
    }
    
    sched->cKThread->ctrl.g->partCtrlTab->arch.ptdL1=val;
    contextTab[sched->cKThread->ctrl.g->cfg->id+1]=val;
    //FlushTlbCtxt();
    FlushTlb();
#endif
    return XM_OK;
}

#endif

// Hypercall table
HYPERCALL_TAB(MulticallSys, 0); // 0
HYPERCALL_TAB(HaltPartitionSys, 1); // 1
HYPERCALL_TAB(SuspendPartitionSys, 1); // 2
HYPERCALL_TAB(ResumePartitionSys, 1); // 3
HYPERCALL_TAB(ResetPartitionSys, 3); // 4
HYPERCALL_TAB(ShutdownPartitionSys, 1); // 5
HYPERCALL_TAB(HaltSystemSys, 0); // 6
HYPERCALL_TAB(ResetSystemSys, 1); // 7
HYPERCALL_TAB(IdleSelfSys, 0); // 8

HYPERCALL_TAB(GetTimeSys, 2); // 9
HYPERCALL_TAB(SetTimerSys, 3); // 10
HYPERCALL_TAB(ReadObjectSys, 5); // 11
HYPERCALL_TAB(WriteObjectSys, 5); // 12
HYPERCALL_TAB(SeekObjectSys, 3); // 13
HYPERCALL_TAB(CtrlObjectSys, 3); // 14

HYPERCALL_TAB(ClearIrqMaskSys, 2); // 15
HYPERCALL_TAB(SetIrqMaskSys, 2); // 16
HYPERCALL_TAB(ForceIrqsSys, 2); // 17
HYPERCALL_TAB(ClearIrqsSys, 2); // 18
HYPERCALL_TAB(RouteIrqSys, 3); // 19

#ifdef CONFIG_MMU
HYPERCALL_TAB(UpdatePage32Sys, 2); // 20
HYPERCALL_TAB(SetPageTypeSys, 2); // 21
HYPERCALLR_TAB(InvldTlbSys, 1); // 22
#else
HYPERCALL_TAB(0, 0); // 20
HYPERCALL_TAB(0, 0); // 21
HYPERCALL_TAB(0, 0); // 22
#endif

HYPERCALLR_TAB(RaiseIpviSys, 1); // 23
HYPERCALLR_TAB(RaisePartitionIpviSys, 2); // 24
HYPERCALLR_TAB(OverrideTrapHndlSys, 2); // 25

HYPERCALLR_TAB(FlushCacheSys, 1); // 26
HYPERCALLR_TAB(SetCacheStateSys, 1); // 27
HYPERCALLR_TAB(SwitchSchedPlanSys, 2); // 28
HYPERCALLR_TAB(GetGidByNameSys, 2); // 29

HYPERCALLR_TAB(ResetVCpuSys, 4); // 30
HYPERCALLR_TAB(HaltVCpuSys, 1); // 31
HYPERCALLR_TAB(SuspendVCpuSys, 1); // 32
HYPERCALLR_TAB(ResumeVCpuSys, 1); // 33
HYPERCALLR_TAB(GetVCpuIdSys, 0); // 34


HYPERCALL_TAB(SparcAtomicAddSys, 2); // 35
HYPERCALL_TAB(SparcAtomicAndSys, 2); // 36
HYPERCALL_TAB(SparcAtomicOrSys, 2); // 37
HYPERCALL_TAB(SparcIoInportSys, 2); // 38
HYPERCALL_TAB(SparcIoOutportSys, 2); // 39

HYPERCALL_TAB(SparcWriteTbrSys, 1); // 40
#ifdef CONFIG_MMU
HYPERCALL_TAB(SparcWritePtdL1Sys, 1); // 41
#else
HYPERCALL_TAB(0, 0); // 41
#endif

#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
HYPERCALL_TAB(ResetPartitionNodeSys, 3); // 42
HYPERCALL_TAB(HaltPartitionNodeSys, 2); // 43
HYPERCALL_TAB(ResetSystemNodeSys, 2); // 44
HYPERCALL_TAB(HaltSystemNodeSys, 1); // 45
HYPERCALL_TAB(SwitchSchedPlanNodeSys, 3); // 46
#else
HYPERCALL_TAB(0, 0); // 42
HYPERCALL_TAB(0, 0); // 43
HYPERCALL_TAB(0, 0); // 44
HYPERCALL_TAB(0, 0); // 45
HYPERCALL_TAB(0, 0); // 46
#endif

// ASM hypercall table
ASM_HYPERCALL_TAB(SparcIRetSys);
ASM_HYPERCALL_TAB(SparcFlushRegWinSys);
ASM_HYPERCALL_TAB(SparcGetPsrSys);
ASM_HYPERCALL_TAB(SparcSetPsrSys);
ASM_HYPERCALL_TAB(SparcSetPilSys);
ASM_HYPERCALL_TAB(SparcClearPilSys);
ASM_HYPERCALL_TAB(SparcCtrlWinFlowSys);

#ifdef CONFIG_AUDIT_EVENTS
xm_u8_t auditAsmHCall[NR_ASM_HYPERCALLS];

void AuditAsmHCall(void) {
    if (IsAuditEventMasked(TRACE_HCALLS_MODULE)) {
        xmWord_t payload[2];
        xm_s32_t e;
        for (e=0; e<NR_ASM_HYPERCALLS; e++)
            if (auditAsmHCall[e]) {
                payload[0]=e;
                payload[1]=auditAsmHCall[e];
                RaiseAuditEvent(TRACE_HCALLS_MODULE, AUDIT_ASMHCALL, 2, payload);
                auditAsmHCall[e]=0;
            }
    }
}

#endif
