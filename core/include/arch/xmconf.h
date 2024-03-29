/*
 * $FILE: xmconf.h
 *
 * Config parameters for both, XM and partitions
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_ARCH_XMCONF_H_
#define _XM_ARCH_XMCONF_H_

/* <track id="test-hw-events"> */
#define XM_HM_EV_SPARC_WRITE_ERROR (XM_HM_MAX_GENERIC_EVENTS+0)
#define XM_HM_EV_SPARC_INSTR_ACCESS_MMU_MISS (XM_HM_MAX_GENERIC_EVENTS+1)
#define XM_HM_EV_SPARC_INSTR_ACCESS_ERROR (XM_HM_MAX_GENERIC_EVENTS+2)
#define XM_HM_EV_SPARC_REGISTER_HARDWARE_ERROR (XM_HM_MAX_GENERIC_EVENTS+3)
#define XM_HM_EV_SPARC_INSTR_ACCESS_EXCEPTION (XM_HM_MAX_GENERIC_EVENTS+4)
#define XM_HM_EV_SPARC_PRIVILEGED_INSTR (XM_HM_MAX_GENERIC_EVENTS+5)
#define XM_HM_EV_SPARC_ILLEGAL_INSTR (XM_HM_MAX_GENERIC_EVENTS+6)
#define XM_HM_EV_SPARC_FP_DISABLED (XM_HM_MAX_GENERIC_EVENTS+7)
#define XM_HM_EV_SPARC_CP_DISABLED (XM_HM_MAX_GENERIC_EVENTS+8)
#define XM_HM_EV_SPARC_UNIMPLEMENTED_FLUSH (XM_HM_MAX_GENERIC_EVENTS+9)
#define XM_HM_EV_SPARC_WATCHPOINT_DETECTED (XM_HM_MAX_GENERIC_EVENTS+10)
#define XM_HM_EV_SPARC_MEM_ADDR_NOT_ALIGNED (XM_HM_MAX_GENERIC_EVENTS+11)
#define XM_HM_EV_SPARC_FP_EXCEPTION (XM_HM_MAX_GENERIC_EVENTS+12)
#define XM_HM_EV_SPARC_CP_EXCEPTION (XM_HM_MAX_GENERIC_EVENTS+13)
#define XM_HM_EV_SPARC_DATA_ACCESS_ERROR (XM_HM_MAX_GENERIC_EVENTS+14)
#define XM_HM_EV_SPARC_DATA_ACCESS_MMU_MISS (XM_HM_MAX_GENERIC_EVENTS+15)
#define XM_HM_EV_SPARC_DATA_ACCESS_EXCEPTION (XM_HM_MAX_GENERIC_EVENTS+16)
#define XM_HM_EV_SPARC_TAG_OVERFLOW (XM_HM_MAX_GENERIC_EVENTS+17)
#define XM_HM_EV_SPARC_DIVIDE_EXCEPTION (XM_HM_MAX_GENERIC_EVENTS+18)
/* </track id="test-hw-events"> */

#define XM_HM_MAX_EVENTS (XM_HM_MAX_GENERIC_EVENTS+19)


#define XM_MEM_REG_STRAM 0
#define XM_MEM_REG_SDRAM 1

#define XM_TRAP_IRQ_15 15 // UNUSED // 15
#define XM_TRAP_PCI_IRQ 14 // PCI // 14
#define XM_TRAP_IRQ_13 13 // UNUSED // 13
#define XM_TRAP_IRQ_12 12 // UNUSED // 12
#define XM_TRAP_DSU_IRQ 11 // DSU // 11
#define XM_TRAP_IRQ_10 10 // UNUSED // 10
#define XM_TRAP_TIMER2_IRQ 9 // TIMER2 // 9
#define XM_TRAP_TIMER1_IRQ 8 // TIMER1 // 8
#define XM_TRAP_IO3_IRQ 7 // IO3 // 7
#define XM_TRAP_IO2_IRQ 6 // IO2 // 6
#define XM_TRAP_IO1_IRQ 5 // IO1 // 5
#define XM_TRAP_IO0_IRQ 4 // IO0 // 4
#define XM_TRAP_UART1_IRQ 3 // UART1 // 3
#define XM_TRAP_UART2_IRQ 2 // UART2 // 2
#define XM_TRAP_AMBA_IRQ 1 // AMBA // 1
#define XM_TRAP_IRQ_0 0

#define XM_CPU_FEATURE_WA1 0x1

// [base, end[
struct xmcIoPort {
    xm_u32_t type;
#define XM_IOPORT_RANGE 0
#define XM_RESTRICTED_IOPORT 1
    union {
        struct xmcIoPortRange {
            xmIoAddress_t base;
            xm_s32_t noPorts;
        } range;
        struct xmcRestrictdIoPort {
            xmIoAddress_t address;
            xm_u32_t mask;
#define XM_DEFAULT_RESTRICTED_IOPORT_MASK (~0)
        } restricted;
    };
};

struct xmcHpvArch {
};

struct xmcPartitionArch {
};

#endif
