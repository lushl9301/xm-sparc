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
#include <kthread.h>
#include <gaccess.h>
#include <physmm.h>
#include <processor.h>
#include <spinlock.h>
#include <sched.h>
#include <smp.h>
#include <stdc.h>
#include <hypercalls.h>
#include <virtmm.h>
#include <vmmap.h>

#include <objects/trace.h>

#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
#include <drivers/ttnocports.h>
extern struct messageTTNoC xmMessageTTNoCRemoteRx[CONFIG_TTNOC_NODES];
extern struct messageTTNoC xmMessageTTNoCRemoteTx[CONFIG_TTNOC_NODES];
#endif

extern xm_u32_t resetStatusInit[];

extern struct {
    xm_u32_t noArgs;
#define HYP_NO_ARGS(args) ((args)&~0x80000000)
} hypercallFlagsTab[NR_HYPERCALLS];

__hypercall xm_s32_t MulticallSys(void *__gParam startAddr, void *__gParam endAddr) {
//get multiple hypercalls, and update startAddr to endAddr
#define BATCH_GET_PARAM(_addr, _arg) *(xm_u32_t *)((_addr)+sizeof(xm_u32_t)*(2+(_arg)))
    extern xm_s32_t (*hypercallsTab[NR_HYPERCALLS])(xmWord_t, ...);
    xmAddress_t addr;
    xm_u32_t noHyp;

    ASSERT(!HwIsSti());
    if (endAddr<startAddr)
        return XM_INVALID_PARAM;

    if (CheckGParam(startAddr, (xmAddress_t)endAddr-(xmAddress_t)startAddr, 4, PFLAG_RW)<0)
        return XM_INVALID_PARAM;

    for (addr=(xmAddress_t)startAddr; addr<(xmAddress_t)endAddr;) {
        noHyp=*(xm_u32_t *)addr;
        *(xm_u32_t *)(addr+sizeof(xm_u32_t))&=~(0xffff<<16);
        if ((noHyp>=NR_HYPERCALLS)||
            (*(xm_u32_t *)(addr+sizeof(xm_u32_t))!=HYP_NO_ARGS(hypercallFlagsTab[noHyp].noArgs))) {
            *(xm_u32_t *)(addr+sizeof(xm_u32_t))|=(XM_INVALID_PARAM<<16);
            PWARN("[MULTICALL] hyp %d no. params mismatches\n", noHyp);
            return XM_MULTICALL_ERROR;
        }
        *(xm_u32_t *)(addr+sizeof(xm_u32_t))|=hypercallsTab[noHyp](BATCH_GET_PARAM(addr, 0), BATCH_GET_PARAM(addr, 1), BATCH_GET_PARAM(addr, 2), BATCH_GET_PARAM(addr, 3), BATCH_GET_PARAM(addr, 4))<<16;
        addr+=(HYP_NO_ARGS(hypercallFlagsTab[noHyp].noArgs)+2)*sizeof(xm_u32_t);
    }

#undef BATCH_GET_PARAM
    return XM_OK;
}

__hypercall xm_s32_t HaltPartitionSys(xmId_t partitionId) {
//halt partition and reschedule
    localSched_t *sched=GET_LOCAL_SCHED();
    cpuCtxt_t ctxt;
    xm_s32_t e;

    ASSERT(!HwIsSti());
    if (partitionId!=KID2PARTID(sched->cKThread->ctrl.g->id)) {
        if (!(GetPartition(sched->cKThread)->cfg->flags&XM_PART_SYSTEM))
            return XM_PERM_ERROR;

        if (partitionId>=xmcTab.noPartitions)
            return XM_INVALID_PARAM;

        //TODO why need to find the first unsetted CPU
        for (e=0; e<partitionTab[partitionId].cfg->noVCpus; e++)
            if (!AreKThreadFlagsSet(partitionTab[partitionId].kThread[e], KTHREAD_HALTED_F))
                break;
        //TODO
        if (e>=partitionTab[partitionId].cfg->noVCpus)
            return XM_NO_ACTION;

        HALT_PARTITION(partitionId);
#ifdef CONFIG_DEBUG
        kprintf("[HYPERCALL] (0x%x) Halted\n", partitionId);
#endif
        return XM_OK;
    }

    HALT_PARTITION(partitionId);
#ifdef CONFIG_DEBUG
    kprintf("[HYPERCALL] (0x%x) Halted\n", partitionId);
#endif
    // here to re-schedule
    Schedule();
    GetCpuCtxt(&ctxt);
    SystemPanic(&ctxt, "[HYPERCALL] A halted partition is being executed");
    return XM_OK;
}

#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
__hypercall xm_s32_t HaltPartitionNodeSys(xmId_t nodeId, xmId_t partitionId) {
    localSched_t *sched=GET_LOCAL_SCHED();

    ASSERT(!HwIsSti());
    if (!(GetPartition(sched->cKThread)->cfg->flags&XM_PART_SYSTEM))
        return XM_PERM_ERROR;

    if ((nodeId<0)||(nodeId>=CONFIG_TTNOC_NODES))
        return XM_INVALID_PARAM;

    if (ttnocNodes[nodeId].txSlot==NULL)
        return XM_INVALID_CONFIG;

    if (partitionId>xmMessageTTNoCRemoteRx[nodeId].infoNode.noParts)
        return XM_INVALID_PARAM;

    setCommandPart(nodeId,partitionId,TTNOC_CMD_HALT);
    return XM_OK;
}
#endif

__hypercall xm_s32_t SuspendVCpuSys(xmId_t vCpuId) {
//suspendVCpu; if k is current running thread, reschedule
    localSched_t *sched=GET_LOCAL_SCHED();
    kThread_t *k;

    ASSERT(!HwIsSti());

    if (vCpuId>=GetPartition(sched->cKThread)->cfg->noVCpus)
        return XM_INVALID_PARAM;

    k=GetPartition(sched->cKThread)->kThread[vCpuId];

    if (!AreKThreadFlagsSet(k, KTHREAD_SUSPENDED_F|KTHREAD_HALTED_F))
        return XM_NO_ACTION;

    SUSPEND_VCPU(KID2PARTID(k->ctrl.g->id), KID2VCPUID(k->ctrl.g->id));

    if (k==sched->cKThread)
        Schedule();

    return XM_OK;
}

__hypercall xm_s32_t ResumeVCpuSys(xmId_t vCpuId) {
//RESUME_VCPU
    localSched_t *sched=GET_LOCAL_SCHED();
    kThread_t *k;

    ASSERT(!HwIsSti());

    if (vCpuId>=GetPartition(sched->cKThread)->cfg->noVCpus)
        return XM_INVALID_PARAM;

    k=GetPartition(sched->cKThread)->kThread[vCpuId];
    if (AreKThreadFlagsSet(k, KTHREAD_SUSPENDED_F)&&!AreKThreadFlagsSet(k, KTHREAD_HALTED_F))
        return XM_NO_ACTION;

    RESUME_VCPU(KID2PARTID(k->ctrl.g->id), KID2VCPUID(k->ctrl.g->id));

    if (k==sched->cKThread)
        Schedule();
#ifdef CONFIG_SMP
    else {
        xm_u8_t cpu=xmcVCpuTab[(KID2PARTID(sched->cKThread->ctrl.g->id)*xmcTab.hpv.noCpus)+vCpuId].cpu;
        if (cpu!=GET_CPU_ID())
            SendIpi(cpu,NO_SHORTHAND_IPI,SCHED_PENDING_IPI_VECTOR);
            ///here may contain bugs if there is only one cpu
            // should be no bug
    }
#endif

    return XM_OK;
}

__hypercall xm_s32_t ResetVCpuSys(xmId_t vCpuId, xmAddress_t ptdL1, xmAddress_t entryPoint, xm_u32_t status) {
//halt VCpu; ResetKThread;
    localSched_t *sched=GET_LOCAL_SCHED();
    partition_t *partition=GetPartition(sched->cKThread);
    struct physPage *ptdL1Page;

    if (vCpuId>=partition->cfg->noVCpus)
        return XM_INVALID_PARAM;

    if (!(ptdL1Page=PmmFindPage(ptdL1, partition, 0)))
        return XM_INVALID_PARAM;

    if (ptdL1Page->type!=PPAG_PTDL1)
        return XM_INVALID_PARAM;

    HALT_VCPU(partition->cfg->id, vCpuId);

    ResetKThread(partition->kThread[vCpuId], ptdL1, entryPoint, status);

/*#ifdef CONFIG_SMP
    xm_u8_t cpu=xmcVCpuTab[(KID2PARTID(sched->cKThread->ctrl.g->id)*xmcTab.hpv.noCpus)+vCpuId].cpu;
    if (cpu!=GET_CPU_ID())
       SendIpi(cpu,NO_SHORTHAND_IPI,SCHED_PENDING_IPI_VECTOR);
#endif*/

    return XM_OK;
}

__hypercall xmId_t GetVCpuIdSys(void) {
//
    localSched_t *sched=GET_LOCAL_SCHED();
    return KID2VCPUID(sched->cKThread->ctrl.g->id);
}

__hypercall xm_s32_t HaltVCpuSys(xmId_t vCpuId) {
//similar as above
    localSched_t *sched=GET_LOCAL_SCHED();
    kThread_t *k;

    ASSERT(!HwIsSti());

    if (vCpuId>=GetPartition(sched->cKThread)->cfg->noVCpus)
        return XM_INVALID_PARAM;

    k=GetPartition(sched->cKThread)->kThread[vCpuId];

    if (AreKThreadFlagsSet(k, KTHREAD_HALTED_F))
        return XM_NO_ACTION;
    //TODO how about smp
    HALT_VCPU(KID2PARTID(k->ctrl.g->id), KID2VCPUID(k->ctrl.g->id));

    if (k==sched->cKThread)
        Schedule();

    return XM_OK;
}

__hypercall xm_s32_t HaltSystemSys(void) {
//HaltSystem is implemented in setup.c
    localSched_t *sched=GET_LOCAL_SCHED();
    extern void HaltSystem(void);

    ASSERT(!HwIsSti());

    if (!(GetPartition(sched->cKThread)->cfg->flags&XM_PART_SYSTEM))
        return XM_PERM_ERROR;

    HwCli();
    HaltSystem();
    return XM_OK;
}

#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
__hypercall xm_s32_t HaltSystemNodeSys(xmId_t nodeId) {
    localSched_t *sched=GET_LOCAL_SCHED();

    ASSERT(!HwIsSti());

    if (!(GetPartition(sched->cKThread)->cfg->flags&XM_PART_SYSTEM))
        return XM_PERM_ERROR;

    if ((nodeId<0)||(nodeId>=CONFIG_TTNOC_NODES))
            return XM_INVALID_PARAM;

    if (ttnocNodes[nodeId].txSlot==NULL)
            return XM_INVALID_CONFIG;

    setCommandHyp(nodeId,TTNOC_CMD_HALT);

    return XM_OK;
}
#endif

// XXX: the ".data" section is restored during the initialisation
__hypercall xm_s32_t ResetSystemSys(xm_u32_t resetMode) {
//
    localSched_t *sched=GET_LOCAL_SCHED();


    ASSERT(!HwIsSti());
    if (!(GetPartition(sched->cKThread)->cfg->flags&XM_PART_SYSTEM))
        return XM_PERM_ERROR;

    if ((resetMode!=XM_COLD_RESET)&&(resetMode!=XM_WARM_RESET))
        return XM_INVALID_PARAM;
    resetStatusInit[0]=(XM_RESET_STATUS_PARTITION_NORMAL_START<<XM_HM_RESET_STATUS_USER_CODE_BIT);
    ResetSystem(resetMode);

    return XM_OK;
}

#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
__hypercall xm_s32_t ResetSystemNodeSys(xmId_t nodeId, xm_u32_t resetMode) {
    localSched_t *sched=GET_LOCAL_SCHED();


    ASSERT(!HwIsSti());
    if (!(GetPartition(sched->cKThread)->cfg->flags&XM_PART_SYSTEM))
       return XM_PERM_ERROR;

    if ((resetMode!=XM_COLD_RESET)&&(resetMode!=XM_WARM_RESET))
        return XM_INVALID_PARAM;

    if ((nodeId<0)||(nodeId>=CONFIG_TTNOC_NODES))
            return XM_INVALID_PARAM;

    if (ttnocNodes[nodeId].txSlot==NULL)
            return XM_INVALID_CONFIG;

    if (resetMode==XM_COLD_RESET)
       setCommandHyp(nodeId,TTNOC_CMD_COLD_RESET);
    else
       setCommandHyp(nodeId,TTNOC_CMD_WARM_RESET);

    return XM_OK;
}

#endif

__hypercall xm_s32_t FlushCacheSys(xm_u32_t cache) {
//set current flag to ~(XM_DCACHE|XM_ICACHE)
    localSched_t *sched=GET_LOCAL_SCHED();

    ASSERT(!HwIsSti());
    if (cache&~(XM_DCACHE|XM_ICACHE))
        return XM_INVALID_PARAM;
    SetKThreadFlags(sched->cKThread,(cache&KTHREAD_FLUSH_CACHE_W)<<KTHREAD_FLUSH_CACHE_B);
    return XM_OK;
}

__hypercall xm_s32_t SetCacheStateSys(xm_u32_t cache) {
//enable cache and ...
    localSched_t *sched=GET_LOCAL_SCHED();

    ASSERT(!HwIsSti());
    if (cache&~(XM_DCACHE|XM_ICACHE))
        return XM_INVALID_PARAM;

    //clear first then reset?
    ClearKThreadFlags(sched->cKThread, KTHREAD_CACHE_ENABLED_W);
    SetKThreadFlags(sched->cKThread, (cache&KTHREAD_CACHE_ENABLED_W)<<KTHREAD_CACHE_ENABLED_B);

    return XM_OK;
}

__hypercall xm_s32_t SwitchSchedPlanSys(xm_u32_t newPlanId, xm_u32_t *__gParam currentPlanId) {
//check plans and switch plan
#ifdef CONFIG_CYCLIC_SCHED
    localSched_t *sched=GET_LOCAL_SCHED();

    ASSERT(!HwIsSti());

    if (!(GetPartition(sched->cKThread)->cfg->flags&XM_PART_SYSTEM))
        return XM_PERM_ERROR;

    if (CheckGParam(currentPlanId, sizeof(xm_u32_t), 4, PFLAG_RW|PFLAG_NOT_NULL)<0)
        return XM_INVALID_PARAM;

    if (sched->data->cyclic.plan.current->id==newPlanId)
        return XM_NO_ACTION;

    if ((newPlanId<0)||(newPlanId>=xmcTab.hpv.cpuTab[GET_CPU_ID()].noSchedCyclicPlans)) return XM_INVALID_PARAM;

    if (SwitchSchedPlan(newPlanId, currentPlanId))
        return XM_INVALID_CONFIG;

    return XM_OK;
#else
    return XM_NO_ACTION;
#endif
}

#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
__hypercall xm_s32_t SwitchSchedPlanNodeSys(xmId_t nodeId, xm_u32_t newPlanId, xm_u32_t *__gParam currentPlanId) {

    localSched_t *sched=GET_LOCAL_SCHED();

    ASSERT(!HwIsSti());

    if (!(GetPartition(sched->cKThread)->cfg->flags&XM_PART_SYSTEM))
        return XM_PERM_ERROR;

    if ((nodeId<0)||(nodeId>=CONFIG_TTNOC_NODES))
            return XM_INVALID_PARAM;

    if (ttnocNodes[nodeId].txSlot==NULL)
            return XM_INVALID_CONFIG;

    if (CheckGParam(currentPlanId, sizeof(xm_u32_t), 4, PFLAG_RW|PFLAG_NOT_NULL)<0)
        return XM_INVALID_PARAM;

    if (xmMessageTTNoCRemoteRx[nodeId].stateHyp.currSchedPlan==newPlanId)
        return XM_NO_ACTION;

    if (newPlanId>= xmMessageTTNoCRemoteRx[nodeId].infoNode.noSchedPlans)
            return XM_INVALID_PARAM;

    *currentPlanId=xmMessageTTNoCRemoteRx[nodeId].stateHyp.currSchedPlan;
    setCommandNewSchedPlan(nodeId,newPlanId);

    return XM_OK;
}
#endif

__hypercall xm_s32_t SuspendPartitionSys(xmId_t partitionId) {
//similar as HaltPartitionSys;
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t e;
    ASSERT(!HwIsSti());
    if (partitionId!=KID2PARTID(sched->cKThread->ctrl.g->id)) {
        if (!(GetPartition(sched->cKThread)->cfg->flags&XM_PART_SYSTEM))
            return XM_PERM_ERROR;

        if (partitionId>=xmcTab.noPartitions)
            return XM_INVALID_PARAM;
        // if all VCpus are suspened or halted, then no action???
        for (e=0; e<partitionTab[partitionId].cfg->noVCpus; e++)
            if (!AreKThreadFlagsSet(partitionTab[partitionId].kThread[e], KTHREAD_SUSPENDED_F|KTHREAD_HALTED_F))
                break;

        if (e>=partitionTab[partitionId].cfg->noVCpus)
            return XM_NO_ACTION;

        SUSPEND_PARTITION(partitionId);
        return XM_OK;
    }

    SUSPEND_PARTITION(partitionId);
    Schedule();
    return XM_OK;
}

__hypercall xm_s32_t ResumePartitionSys(xmId_t partitionId) {
//
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t e;
    ASSERT(!HwIsSti());

    if (partitionId==KID2PARTID(sched->cKThread->ctrl.g->id))
        return XM_NO_ACTION;

    if (!(GetPartition(sched->cKThread)->cfg->flags&XM_PART_SYSTEM))
        return XM_PERM_ERROR;

    if (partitionId>=xmcTab.noPartitions)
        return XM_INVALID_PARAM;

    for (e=0; e<partitionTab[partitionId].cfg->noVCpus; e++)
        if (AreKThreadFlagsSet(partitionTab[partitionId].kThread[e], KTHREAD_SUSPENDED_F)&&!AreKThreadFlagsSet(partitionTab[partitionId].kThread[e], KTHREAD_HALTED_F))
            break;

    if (e>=partitionTab[partitionId].cfg->noVCpus)
        return XM_NO_ACTION;

    RESUME_PARTITION(partitionId);
    return XM_OK;
}

__hypercall xm_s32_t ResetPartitionSys(xmId_t partitionId, xm_u32_t resetMode, xm_u32_t status) {
//iff current parition is system partition then do reset partition
    localSched_t *sched=GET_LOCAL_SCHED();
    ASSERT(!HwIsSti());
    if (partitionId!=KID2PARTID(sched->cKThread->ctrl.g->id)) {
        if (!(GetPartition(sched->cKThread)->cfg->flags&XM_PART_SYSTEM))
            return XM_PERM_ERROR;
        if (partitionId>=xmcTab.noPartitions)
            return XM_INVALID_PARAM;
    }
    if ((resetMode==XM_WARM_RESET)||(resetMode==XM_COLD_RESET)) {
        if (!ResetPartition(&partitionTab[partitionId], resetMode&XM_RESET_MODE, status))
            return XM_OK;
        else
            return XM_INVALID_CONFIG;
    }

    return XM_INVALID_PARAM;
}

#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
__hypercall xm_s32_t ResetPartitionNodeSys(xmId_t nodeId, xmId_t partitionId, xm_u32_t resetMode) {
    localSched_t *sched=GET_LOCAL_SCHED();

    ASSERT(!HwIsSti());

    if (!(GetPartition(sched->cKThread)->cfg->flags&XM_PART_SYSTEM))
           return XM_PERM_ERROR;

    if ((nodeId<0)||(nodeId>=CONFIG_TTNOC_NODES))
            return XM_INVALID_PARAM;

    if (ttnocNodes[nodeId].txSlot==NULL)
            return XM_INVALID_CONFIG;

    if (partitionId>xmMessageTTNoCRemoteRx[nodeId].infoNode.noParts)
            return XM_INVALID_PARAM;

    if ((resetMode!=XM_WARM_RESET)||(resetMode!=XM_COLD_RESET))
            return XM_INVALID_PARAM;

    if (resetMode==XM_COLD_RESET)
        setCommandPart(nodeId,partitionId,TTNOC_CMD_COLD_RESET);
    else
            setCommandPart(nodeId,partitionId,TTNOC_CMD_WARM_RESET);

    return XM_OK;
}
#endif

__hypercall xm_s32_t ShutdownPartitionSys(xmId_t partitionId) {
//similarly, iff current thread is system partition
    localSched_t *sched=GET_LOCAL_SCHED();

    ASSERT(!HwIsSti());
    if (partitionId!=KID2PARTID(sched->cKThread->ctrl.g->id)) {
        if (!(GetPartition(sched->cKThread)->cfg->flags&XM_PART_SYSTEM))
            return XM_PERM_ERROR;
        if (partitionId>=xmcTab.noPartitions)
            return XM_INVALID_PARAM;
    }

    SHUTDOWN_PARTITION(partitionId);
    return XM_OK;
}

__hypercall xm_s32_t IdleSelfSys(void) {
//sched yield to idleKThread and re-schedule
    localSched_t *sched=GET_LOCAL_SCHED();
#ifdef CONFIG_AUDIT_EVENTS
    xmWord_t arg=KID2PARTID(sched->cKThread->ctrl.g->id);
#endif
    ASSERT(!HwIsSti());

#ifdef CONFIG_AUDIT_EVENTS
    RaiseAuditEvent(TRACE_SCHED_MODULE, AUDIT_SCHED_PART_IDLE, 1, &arg);
#endif

    if (xmcTab.hpv.cpuTab[GET_CPU_ID()].schedPolicy!=CYCLIC_SCHED)
        ClearKThreadFlags(sched->cKThread, KTHREAD_READY_F);

    SchedYield(sched, sched->idleKThread);
    return XM_OK;
}

__hypercall xm_s32_t SetTimerSys(xm_u32_t clockId, xmTime_t abstime, xmTime_t interval) {
//combine disarm and arm timer in the same func
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t ret=XM_OK;

    ASSERT(!HwIsSti());

    if ((abstime<0)||(interval<0))
        return XM_INVALID_PARAM;

    // Disarming a timer
    if (!abstime) {
        switch(clockId) {
        case XM_HW_CLOCK:
            DisarmKTimer(&sched->cKThread->ctrl.g->kTimer);
            return XM_OK;
        case XM_EXEC_CLOCK:
            DisarmVTimer(&sched->cKThread->ctrl.g->vTimer, &sched->cKThread->ctrl.g->vClock);
            return XM_OK;
        case XM_WATCHDOG_TIMER:
            DisarmKTimer(&sched->cKThread->ctrl.g->watchdogTimer);
            return XM_OK;
        default:
            return XM_INVALID_PARAM;
        }
    }
    // Arming a timer
    switch(clockId) {
    case XM_HW_CLOCK:
        ret=ArmKTimer(&sched->cKThread->ctrl.g->kTimer, abstime, interval);
        break;
    case XM_EXEC_CLOCK:
        ret=ArmVTimer(&sched->cKThread->ctrl.g->vTimer, &sched->cKThread->ctrl.g->vClock, abstime, interval);
        break;
    case XM_WATCHDOG_TIMER:
        ret=ArmKTimer(&sched->cKThread->ctrl.g->watchdogTimer, abstime, interval);
        break;
    default:
        return XM_INVALID_PARAM;
    }

    return ret;
}

__hypercall xm_s32_t GetTimeSys(xm_u32_t clockId, xmTime_t *__gParam time) {
//everyone can get hw clock
    localSched_t *sched=GET_LOCAL_SCHED();

    if (CheckGParam(time, sizeof(xm_s64_t), 8, PFLAG_RW|PFLAG_NOT_NULL)<0)
        return XM_INVALID_PARAM;

    switch(clockId) {
    case XM_HW_CLOCK:
        *time=GetSysClockUsec();
        break;

    case XM_EXEC_CLOCK:
        *time=GetTimeUsecVClock(&sched->cKThread->ctrl.g->vClock);
        break;
    default:
        return XM_INVALID_PARAM;
    }

    return XM_OK;
}

/*
__hypercall xm_s32_t SetIrqLevelSys(xm_u32_t level) {
    localSched_t *sched=GET_LOCAL_SCHED();
    ASSERT(!HwIsSti());


    //sched->cKThread->ctrl.g->partCtrlTab->iFlags&=~IFLAGS_IRQ_MASK;
    //sched->cKThread->ctrl.g->partCtrlTab->iFlags|=level&IFLAGS_IRQ_MASK;

    return XM_OK;
}
*/

__hypercall xm_s32_t ClearIrqMaskSys(xm_u32_t hwIrqsMask, xm_u32_t extIrqsPend) {
//hwIrqCtrl[irq].Enable(irq);
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_u32_t unmasked;
    xm_s32_t e;

    ASSERT(!HwIsSti());
    sched->cKThread->ctrl.g->partCtrlTab->hwIrqsMask&=~hwIrqsMask;
    sched->cKThread->ctrl.g->partCtrlTab->extIrqsMask&=~extIrqsPend;
    unmasked = hwIrqsMask & GetPartition(sched->cKThread)->cfg->hwIrqs;
    for (e=0; unmasked; e++)
        if (unmasked&(1<<e)) {
            HwEnableIrq(e);
            unmasked&=~(1<<e);
        }
    return XM_OK;
}

__hypercall xm_s32_t SetIrqMaskSys(xm_u32_t hwIrqsMask, xm_u32_t extIrqsPend) {
//disable
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_u32_t masked;
    xm_s32_t e;

    ASSERT(!HwIsSti());
    sched->cKThread->ctrl.g->partCtrlTab->hwIrqsMask|=hwIrqsMask;
    sched->cKThread->ctrl.g->partCtrlTab->extIrqsMask|=extIrqsPend;
    masked=hwIrqsMask & GetPartition(sched->cKThread)->cfg->hwIrqs;
    /// maybe here we can delete the last line in the if-clause
    for (e=0; masked; e++)
        if (masked&(1<<e)) {
            HwDisableIrq(e);
            masked&=~(1<<e);
        }
    return XM_OK;
}

__hypercall xm_s32_t ForceIrqsSys(xm_u32_t hwIrqMask, xm_u32_t extIrqMask) {
//
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_u32_t forced;
    xm_s32_t e;

    ASSERT(!HwIsSti());

//    sched->cKThread->ctrl.g->partCtrlTab->hwIrqsPend|=hwIrqMask;
    sched->cKThread->ctrl.g->partCtrlTab->extIrqsPend|=extIrqMask;
    forced=hwIrqMask&GetPartition(sched->cKThread)->cfg->hwIrqs;
    hwIrqMask&=~forced;
    for (e=0; forced; e++)
        if (forced&(1<<e)) {
            HwForceIrq(e);
            forced&=~(1<<e);
        }
    for (e=0; hwIrqMask; e++)
        if (hwIrqMask&(1<<e)) {
            sched->cKThread->ctrl.g->partCtrlTab->hwIrqsPend|=(1<<e);
            hwIrqMask&=~(1<<e);
        }
    return XM_OK;
}

__hypercall xm_s32_t ClearIrqsSys(xm_u32_t hwIrqMask, xm_u32_t extIrqMask) {
//clear; not doing anymore?
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_u32_t pending;
    xm_s32_t e;

    ASSERT(!HwIsSti());

    sched->cKThread->ctrl.g->partCtrlTab->hwIrqsPend&=~hwIrqMask;
    sched->cKThread->ctrl.g->partCtrlTab->extIrqsPend&=~extIrqMask;
    pending=hwIrqMask&GetPartition(sched->cKThread)->cfg->hwIrqs;
    for (e=0; pending; e++)
        if (pending&(1<<e)) {
            HwClearIrq(e);
            pending&=~(1<<e);
        }
    return XM_OK;
}

__hypercall xm_s32_t RouteIrqSys(xm_u32_t type, xm_u32_t irq, xm_u16_t vector) {
//
    localSched_t *sched=GET_LOCAL_SCHED();
    ASSERT(!HwIsSti());

    if (irq>=32)
        return XM_INVALID_PARAM;
    switch(type) {
    case XM_TRAP_TYPE:
        sched->cKThread->ctrl.g->partCtrlTab->trap2Vector[irq]=vector;
        break;
    case XM_HWIRQ_TYPE:
        sched->cKThread->ctrl.g->partCtrlTab->hwIrq2Vector[irq]=vector;
        break;
    case XM_EXTIRQ_TYPE:
        sched->cKThread->ctrl.g->partCtrlTab->extIrq2Vector[irq]=vector;
        break;
    default:
        return XM_INVALID_PARAM;
    }

    return XM_OK;
}

__hypercall xm_s32_t ReadObjectSys(xmObjDesc_t objDesc, void *__gParam buffer, xmSize_t size, xm_u32_t *__gParam flags) {
//use object->read; get object reference from objDesc
    xm_u32_t class;

    ASSERT(!HwIsSti());

    class=OBJDESC_GET_CLASS(objDesc);
    if (class<OBJ_NO_CLASSES) {
        if (objectTab[class]&&objectTab[class]->Read) {
            return objectTab[class]->Read(objDesc, buffer, size, flags);
        } else {
            return XM_OP_NOT_ALLOWED;
        }
    }

    return XM_INVALID_PARAM;
}

__hypercall xm_s32_t WriteObjectSys(xmObjDesc_t objDesc, void *__gParam buffer, xmSize_t size, xm_u32_t *__gParam flags) {
//
    xm_u32_t class;

    ASSERT(!HwIsSti());
    class=OBJDESC_GET_CLASS(objDesc);
    if (class<OBJ_NO_CLASSES) {
        if (objectTab[class]&&objectTab[class]->Write) {
            return objectTab[class]->Write(objDesc, buffer, size, flags);
        } else {
            return XM_OP_NOT_ALLOWED;
        }
    }

    return XM_INVALID_PARAM;
}

__hypercall xm_s32_t SeekObjectSys(xmObjDesc_t objDesc, xmAddress_t offset, xm_u32_t whence) {
//
    xm_u32_t class;

    ASSERT(!HwIsSti());

    class=OBJDESC_GET_CLASS(objDesc);
    if (class<OBJ_NO_CLASSES) {
        if (objectTab[class]&&objectTab[class]->Seek) {
            return objectTab[class]->Seek(objDesc, offset, whence);
        } else {
            return XM_OP_NOT_ALLOWED;
        }
    }
    return XM_INVALID_PARAM;
}

__hypercall xm_s32_t CtrlObjectSys(xmObjDesc_t objDesc, xm_u32_t cmd, void *__gParam arg) {
//
    xm_u32_t class;

    ASSERT(!HwIsSti());

    class=OBJDESC_GET_CLASS(objDesc);
    if (class<OBJ_NO_CLASSES) {
        if (objectTab[class]&&objectTab[class]->Ctrl) {
            return objectTab[class]->Ctrl(objDesc, cmd, arg);
        } else {
            return XM_OP_NOT_ALLOWED;
        }
    }

    return XM_INVALID_PARAM;
}

__hypercall xm_s32_t RaisePartitionIpviSys(xmId_t partitionId, xm_u8_t noIpvi) {
//TODO
    localSched_t *sched=GET_LOCAL_SCHED();
    struct xmcPartIpvi *ipvi;
    kThread_t *k;
    xm_s32_t e,vcpu;
    ASSERT(!HwIsSti());

    if ((partitionId<0)&&(partitionId>=xmcTab.noPartitions))
        return XM_INVALID_PARAM;

    if ((noIpvi<XM_VT_EXT_IPVI0)||(noIpvi>=XM_VT_EXT_IPVI0+CONFIG_XM_MAX_IPVI))
        return XM_INVALID_PARAM;

    ipvi=&GetPartition(sched->cKThread)->cfg->ipviTab[noIpvi-XM_VT_EXT_IPVI0];
    if (ipvi->noDsts<=0)
        //TODO this return is different from below
        return XM_INVALID_CONFIG;

    for (e=0; e<ipvi->noDsts; e++){
        if (partitionId==xmcDstIpvi[ipvi->dstOffset+e]){
            partition_t *p=&partitionTab[xmcDstIpvi[ipvi->dstOffset+e]];//TODO this will be slower than used partitionId
            if (ArePartitionExtIrqPendingSet(p, noIpvi))
                return XM_NO_ACTION;

//            SetPartitionExtIrqPending(p, noIpvi);
            for (vcpu=0; vcpu<p->cfg->noVCpus; vcpu++) {
                k=p->kThread[vcpu];

                if (AreKThreadFlagsSet(k, KTHREAD_HALTED_F))
                    continue;
                if (AreExtIrqPendingSet(k, noIpvi))
                    continue;
                SetExtIrqPending(k, noIpvi);
#ifdef CONFIG_SMP
                xm_u8_t cpu=xmcVCpuTab[(KID2PARTID(k->ctrl.g->id)*xmcTab.hpv.noCpus)+KID2VCPUID(k->ctrl.g->id)].cpu;
                if (cpu!=GET_CPU_ID())
                    SendIpi(cpu,NO_SHORTHAND_IPI,SCHED_PENDING_IPI_VECTOR);
#endif
            }
            return XM_OK;
        }
    }

    return XM_INVALID_CONFIG;
}

__hypercall xm_s32_t RaiseIpviSys(xm_u8_t noIpvi) {
// general version of RaisePartitionIpviSys
    localSched_t *sched=GET_LOCAL_SCHED();
    struct xmcPartIpvi *ipvi;
    kThread_t *k;
    xm_s32_t e,vcpu;
    ASSERT(!HwIsSti());

    if ((noIpvi<XM_VT_EXT_IPVI0)||(noIpvi>=XM_VT_EXT_IPVI0+CONFIG_XM_MAX_IPVI))
        return XM_INVALID_PARAM;
    ipvi=&GetPartition(sched->cKThread)->cfg->ipviTab[noIpvi-XM_VT_EXT_IPVI0];
    if (ipvi->noDsts<=0)
        //TODO return different from above
        return XM_NO_ACTION;

    for (e=0; e<ipvi->noDsts; e++){
        partition_t *p=&partitionTab[xmcDstIpvi[ipvi->dstOffset+e]];

//        SetPartitionExtIrqPending(p, noIpvi);
        for (vcpu=0; vcpu<p->cfg->noVCpus; vcpu++) {
            k=p->kThread[vcpu];

            if (AreKThreadFlagsSet(k, KTHREAD_HALTED_F))
                continue;
            if (AreExtIrqPendingSet(k, noIpvi))
                continue;
            SetExtIrqPending(k, noIpvi);
#ifdef CONFIG_SMP
            xm_u8_t cpu=xmcVCpuTab[(KID2PARTID(k->ctrl.g->id)*xmcTab.hpv.noCpus)+KID2VCPUID(k->ctrl.g->id)].cpu;
            if (cpu!=GET_CPU_ID())
                SendIpi(cpu,NO_SHORTHAND_IPI,SCHED_PENDING_IPI_VECTOR);
#endif

        }
    }

    return XM_OK;
}

__hypercall xm_s32_t GetGidByNameSys(xm_u8_t *__gParam name, xm_u32_t entity) {
//get General id by name
    xm_s32_t e, id=XM_INVALID_CONFIG;

    if (CheckGParam(name, CONFIG_ID_STRING_LENGTH, 1, PFLAG_NOT_NULL)<0)
        return XM_INVALID_PARAM;

    switch(entity) {
    case XM_PARTITION_NAME:
        for (e=0; e<xmcTab.noPartitions; e++)
            if (!strncmp(&xmcStringTab[xmcPartitionTab[e].nameOffset], name, CONFIG_ID_STRING_LENGTH))  {
                id=xmcPartitionTab[e].id;
                break;
            }
        break;
#ifdef CONFIG_CYCLIC_SCHED
    case XM_PLAN_NAME:
        for (e=0; e<xmcTab.noSchedCyclicPlans; e++)
            if (!strncmp(&xmcStringTab[xmcSchedCyclicPlanTab[e].nameOffset], name, CONFIG_ID_STRING_LENGTH))  {
                id=xmcSchedCyclicPlanTab[e].id;
                break;
            }
        break;
#endif
    default:
        return XM_INVALID_PARAM;
    }

    return id;
}

#ifdef CONFIG_AUDIT_EVENTS
void AuditHCall(xm_u32_t hypNr, ...) {
    va_list argPtr;
    xm_u32_t e, noArgs;
    xmWord_t argList[6];

    if (IsAuditEventMasked(TRACE_HCALLS_MODULE)) {
        if (hypNr<NR_HYPERCALLS)
            noArgs=HYP_NO_ARGS(hypercallFlagsTab[hypNr].noArgs);
        else
            noArgs=0;
        ASSERT(noArgs<=5);
        argList[0]=(hypNr<<16)|noArgs;
        va_start(argPtr, hypNr);
        for (e=0; e<noArgs; e++)
            argList[e+1]=va_arg(argPtr, xm_u32_t);
        va_end(argPtr);
        if (noArgs<XMTRACE_PAYLOAD_LENGTH)
            RaiseAuditEvent(TRACE_HCALLS_MODULE, AUDIT_HCALL_BEGIN, noArgs+1, argList);
        else {
            RaiseAuditEvent(TRACE_HCALLS_MODULE, AUDIT_HCALL_BEGIN, XMTRACE_PAYLOAD_LENGTH, argList);
            RaiseAuditEvent(TRACE_HCALLS_MODULE, AUDIT_HCALL_BEGIN2, noArgs-(XMTRACE_PAYLOAD_LENGTH-1), &argList[3]);
        }

    }
}

void AuditHCallRet(xmWord_t retVal) {
    RaiseAuditEvent(TRACE_HCALLS_MODULE, AUDIT_HCALL_END, 1, &retVal);
}
#endif
