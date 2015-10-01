/*
 * $FILE: leon_pic.c
 *
 * LEON I/O interrupt controller as defined in Datasheet LEON AT697E
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
#include <irqs.h>
#include <processor.h>
#include <spinlock.h>
#include <arch/processor.h>
#include <arch/xm_def.h>

#ifdef CONFIG_LEON3
RESERVE_IOPORTS(LEON_PIC_BASE, 10);
#define ITMP_REG 0x40
RESERVE_IOPORTS(LEON_PIC_BASE+ITMP_REG, 1);

  #define ILEVEL_MASK 0xFFFE0000
    #define ILEVEL_PCI_BIT (1<<30)
    #define ILEVEL_DSU_BIT (1<<27)
    #define ILEVEL_TIMER2_BIT (1<25)
    #define ILEVEL_TIMER1_BIT (1<<24)
    #define ILEVEL_IO3_BIT (1<<23)
    #define ILEVEL_IO2_BIT (1<<22)
    #define ILEVEL_IO1_BIT (1<<21)
    #define ILEVEL_IO0_BIT (1<<20)
    #define ILEVEL_UART1_BIT (1<<19)
    #define ILEVEL_UART2_BIT (1<<18)
    #define ILEVEL_AMBA_BIT (1<<17)

    #define IMASK_PCI_BIT (1<<14)
    #define IMASK_DSU_BIT (1<<11)
    #define IMASK_TIMER2_BIT (1<<9)
    #define IMASK_TIMER1_BIT (1<<8)
    #define IMASK_IO3_BIT (1<<7)
    #define IMASK_IO2_BIT (1<<6)
    #define IMASK_IO1_BIT (1<<5)
    #define IMASK_IO0_BIT (1<<4)
    #define IMASK_UART1_BIT (1<<3)
    #define IMASK_UART2_BIT (1<<2)
    #define IMASK_AMBA_BIT (1<<1)

#define ITP_REG 0x4
  #define IPEND_MASK 0x0000FFFE
    #define IPEND_PCI_BIT (1<<14)
    #define IPEND_DSU_BIT (1<<11)
    #define IPEND_TIMER2_BIT (1<<9)
    #define IPEND_TIMER1_BIT (1<<8)
    #define IPEND_IO3_BIT (1<<7)
    #define IPEND_IO2_BIT (1<<6)
    #define IPEND_IO1_BIT (1<<5)
    #define IPEND_IO0_BIT (1<<4)
    #define IPEND_UART1_BIT (1<<3)
    #define IPEND_UART2_BIT (1<<2)
    #define IPEND_AMBA_BIT (1<<1)

#define ITF_REG 0x8
  #define IFORCE_MASK 0x0000FFFE
    #define IFORCE_PCI_BIT (1<<14)
    #define IFORCE_DSU_BIT (1<<11)
    #define IFORCE_TIMER2_BIT (1<<9)
    #define IFORCE_TIMER1_BIT (1<<8)
    #define IFORCE_IO3_BIT (1<<7)
    #define IFORCE_IO2_BIT (1<<6)
    #define IFORCE_IO1_BIT (1<<5)
    #define IFORCE_IO0_BIT (1<<4)
    #define IFORCE_UART1_BIT (1<<3)
    #define IFORCE_UART2_BIT (1<<2)
    #define IFORCE_AMBA_BIT (1<<1)

#define ITC_REG 0xC
  #define ICLEAR_MASK 0x0000FFFE
    #define ICLEAR_PCI_BIT (1<<14)
    #define ICLEAR_DSU_BIT (1<<11)
    #define ICLEAR_TIMER2_BIT (1<<9)
    #define ICLEAR_TIMER1_BIT (1<<8)
    #define ICLEAR_IO3_BIT (1<<7)
    #define ICLEAR_IO2_BIT (1<<6)
    #define ICLEAR_IO1_BIT (1<<5)
    #define ICLEAR_IO0_BIT (1<<4)
    #define ICLEAR_UART1_BIT (1<<3)
    #define ICLEAR_UART2_BIT (1<<2)
    #define ICLEAR_AMBA_BIT (1<<1)


#define GET_APIC_BASE() (LEON_PIC_BASE+(4*GET_CPU_ID()))
#define GET_APIC_BASE_NCPU(ncpu) (LEON_PIC_BASE+(4*ncpu))
#define GET_PIC_BASE(cpu) LEON_PIC_BASE

#endif


#define INT_LEVEL_REG 0x0
#define INT_PEND_REG 0x4
#define INT_FORCE_REG 0x8
#define INT_CLEAR_REG 0xc
#define MPROC_STATUS_REG 0x10
#define BROADCAST_REG 0x14

#define PROC0_INT_MASK_REG 0x40
#define PROC1_INT_MASK_REG 0x44
#define PROC2_INT_MASK_REG 0x48
#define PROC3_INT_MASK_REG 0x4c
    #define IMASK_MASK 0xFFFFFFFE

#define PROC0_INT_FORCE_REG 0x80
#define PROC1_INT_FORCE_REG 0x84
#define PROC2_INT_FORCE_REG 0x88
#define PROC3_INT_FORCE_REG 0x8c

#define PROC0_EXT_INT_ACK_REG 0xc0
#define PROC1_EXT_INT_ACK_REG 0xc4
#define PROC2_EXT_INT_ACK_REG 0xc8
#define PROC3_EXT_INT_ACK_REG 0xcc


xm_u32_t HwIrqGetMask(void) {
    return ~LoadIoReg(GET_APIC_BASE()+PROC0_INT_MASK_REG);
}

void HwIrqSetMask(xm_u32_t mask) {
//IMASK_MASK irq mask's mask?
    StoreIoReg(GET_APIC_BASE()+PROC0_INT_MASK_REG, (~mask)&IMASK_MASK);
}

static void APicForceIrq(xm_u32_t irq) {
    StoreIoReg(GET_APIC_BASE()+PROC0_INT_FORCE_REG, (1<<irq));
}

static void APicEnableIrq(xm_u32_t irq) {
    StoreIoReg(GET_APIC_BASE()+PROC0_INT_MASK_REG, LoadIoReg(GET_APIC_BASE()+PROC0_INT_MASK_REG)|((1<<irq)&IMASK_MASK));
}

static void APicDisableIrq(xm_u32_t irq) {
    StoreIoReg(GET_APIC_BASE()+PROC0_INT_MASK_REG, LoadIoReg(GET_APIC_BASE()+PROC0_INT_MASK_REG)&~((1<<irq)&IMASK_MASK));
}

xm_u32_t APicGetExtIrq(void) {
    return LoadIoReg(GET_APIC_BASE()+PROC0_EXT_INT_ACK_REG)&0x1f;
}

#ifdef CONFIG_LEON3
static void APicClearIrq(xm_u32_t irq) {
    StoreIoReg(LEON_PIC_BASE+ITC_REG, (1<<irq));
}
#endif

void InitPic(void) {
    xm_s32_t e;

    // Masking all HW IRQs, all irqs have the same priority level (0)
    for (e=0; e<CONFIG_NO_CPUS; e++)
        StoreIoReg(GET_APIC_BASE_NCPU(e)+PROC0_INT_MASK_REG, 0x0);

    for (e=0; e<CONFIG_NO_HWIRQS; e++) {
        hwIrqCtrl[e].Enable=APicEnableIrq;
        hwIrqCtrl[e].Disable=APicDisableIrq;
        hwIrqCtrl[e].Ack=APicDisableIrq;
        hwIrqCtrl[e].End=APicEnableIrq;
        hwIrqCtrl[e].Force=APicForceIrq;
#ifdef CONFIG_LEON3
        hwIrqCtrl[e].Clear=APicClearIrq;
#endif
    }
}


#ifdef CONFIG_SMP

void WakeUpCpu(xm_s32_t cpu) {
    StoreIoReg(GET_PIC_BASE(cpu)+MPROC_STATUS_REG, 1<<cpu);
}

xm_u8_t SparcGetNoCpus(void) {
    return (LoadIoReg(GET_PIC_BASE(0)+MPROC_STATUS_REG)>>28)+1;
}

void SendIpi(xm_u8_t dst, xm_u8_t dstShortHand, xm_u8_t vector) {
    xm_u32_t cpuIdSelf=GET_CPU_ID();
    xm_s32_t e;

    switch(dstShortHand) {
    case SELF_IPI:
        dst=cpuIdSelf;
    case NO_SHORTHAND_IPI:
        //ASSERT((dst>=0)&&(dst<GET_NRCPUS()));
        StoreIoReg(GET_APIC_BASE_NCPU(dst)+PROC0_INT_MASK_REG, LoadIoReg(GET_APIC_BASE_NCPU(dst)+PROC0_INT_MASK_REG)|((1<<vector)&IMASK_MASK));
        StoreIoReg(GET_APIC_BASE_NCPU(dst)+PROC0_INT_FORCE_REG, 1<<vector);
        break;
    case ALL_INC_SELF:
        for (e=0; e<GET_NRCPUS(); e++) {
            StoreIoReg(GET_APIC_BASE_NCPU(e)+PROC0_INT_MASK_REG, LoadIoReg(GET_APIC_BASE_NCPU(e)+PROC0_INT_MASK_REG)|((1<<vector)&IMASK_MASK));
            StoreIoReg(GET_APIC_BASE_NCPU(e)+PROC0_INT_FORCE_REG, 1<<vector);
        }
        break;
    case ALL_EXC_SELF:
        for (e=0; e<GET_NRCPUS(); e++) {
            if (e==cpuIdSelf)
                continue;
            StoreIoReg(GET_APIC_BASE_NCPU(e)+PROC0_INT_MASK_REG, LoadIoReg(GET_APIC_BASE_NCPU(e)+PROC0_INT_MASK_REG)|((1<<vector)&IMASK_MASK));
            StoreIoReg(GET_APIC_BASE_NCPU(e)+PROC0_INT_FORCE_REG, 1<<vector);
        }
        break;
    }
}
#endif
