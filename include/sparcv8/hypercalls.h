/*
 * $FILE: hypercalls.h
 *
 * Processor-related hypercalls definition
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_ARCH_HYPERCALLS_H_
#define _XM_ARCH_HYPERCALLS_H_

//@ \void{<track id="hypercall-numbers">}
#define __MULTICALL_NR 0
#define __HALT_PARTITION_NR 1
#define __SUSPEND_PARTITION_NR 2
#define __RESUME_PARTITION_NR 3
#define __RESET_PARTITION_NR 4
#define __SHUTDOWN_PARTITION_NR 5
#define __HALT_SYSTEM_NR 6
#define __RESET_SYSTEM_NR 7
#define __IDLE_SELF_NR 8

#define __GET_TIME_NR 9
#define __SET_TIMER_NR 10
#define __READ_OBJECT_NR 11
#define __WRITE_OBJECT_NR 12
#define __SEEK_OBJECT_NR 13
#define __CTRL_OBJECT_NR 14

#define __CLEAR_IRQ_MASK_NR 15
#define __SET_IRQ_MASK_NR 16
#define __FORCE_IRQS_NR 17
#define __CLEAR_IRQS_NR 18
#define __ROUTE_IRQ_NR 19

#define __UPDATE_PAGE32_NR 20
#define __SET_PAGE_TYPE_NR 21
#define __INVLD_TLB_NR 22
#define __RAISE_IPVI_NR 23
#define __RAISE_PARTITION_IPVI_NR 24
#define __OVERRIDE_TRAP_HNDL_NR 25
#define __FLUSH_CACHE_NR 26
#define __SET_CACHE_STATE_NR 27

#define __SWITCH_SCHED_PLAN_NR 28
#define __GET_GID_BY_NAME_NR 29
#define __RESET_VCPU_NR 30
#define __HALT_VCPU_NR 31
#define __SUSPEND_VCPU_NR 32
#define __RESUME_VCPU_NR 33

#define __GET_VCPUID_NR 34

#define sparc_atomic_add_nr 35
#define sparc_atomic_and_nr 36
#define sparc_atomic_or_nr 37
#define sparc_inport_nr 38
#define sparc_outport_nr 39
#define sparc_write_tbr_nr 40
#define sparc_write_ptdl1_nr 41

#if defined(CONFIG_DEV_TTNOC)||defined(CONFIG_DEV_TTNOC_MODULE)
#define __RESET_PARTITION_NODE_NR 42
#define __HALT_PARTITION_NODE_NR 43
#define __RESET_SYSTEM_NODE_NR 44
#define __HALT_SYSTEM_NODE_NR 45
#define __SWITCH_SCHED_PLAN_NODE_NR 46
#endif

//@ \void{</track id="hypercall-numbers">}
#define NR_HYPERCALLS 47

//@ \void{<track id="asm-hypercall-numbers">}
#define sparc_iret_nr 0
#define sparc_flush_regwin_nr 1
#define sparc_get_psr_nr 2
#define sparc_set_psr_nr 3
#define sparc_set_pil_nr 4
#define sparc_clear_pil_nr 5
#define sparc_ctrl_winflow_nr 6
//@ \void{</track id="asm-hypercall-numbers">}

#define NR_ASM_HYPERCALLS 7

#ifndef __ASSEMBLY__

#define ASM_HYPERCALL_TAB(_ahc) \
    __asm__ (".section .ahypercallstab, \"a\"\n\t" \
	     ".align 4\n\t" \
	     ".long "#_ahc"\n\t" \
	     ".previous\n\t")

#endif

#endif
