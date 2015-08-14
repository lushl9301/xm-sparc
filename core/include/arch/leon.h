/*
 * $FILE: leon.h
 *
 * LEON definitions
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_ARCH_LEON_H_
#define _XM_ARCH_LEON_H_

#define DEFAULT_PORT LEON_UART1_BASE
#define LEON_PRESCALER_KHZ (1000) // 1 Mhz

#ifdef CONFIG_LEON3
#define LEON_IOREGS 0x80000000

#define LEON_IO_AREA 0xfff00000
#define LEON_MEMORY_CFG_BASE (LEON_IOREGS+0x0)
#define LEON_MEMORY_WPR_BASE (LEON_IOREGS+0x1C)
//#define LEON_PCR_BASE (LEON_IOREGS+0x24)
//#define LEON_FAILAR_BASE (LEON_IOREGS+0xC)
//#define LEON_FAILSR_BASE (LEON_IOREGS+0x10)
//#define LEON_IDLE_BASE (LEON_IOREGS+0x18)
//#define LEON_WATCHDOG_BASE (LEON_IOREGS+0x4C)
#define LEON_TIMER_CFG_BASE (LEON_IOREGS+0x300)
#define LEON_TIMER1_BASE (LEON_IOREGS+0x310)
#define LEON_TIMER2_BASE (LEON_IOREGS+0x320)
#define LEON_AHB_DEBUG_UART (LEON_IOREGS+0x700)
#define LEON_UART1_BASE (LEON_IOREGS+0x100)
#define LEON_UART2_BASE (LEON_IOREGS+0x900)
#define LEON_PIC_BASE (LEON_IOREGS+0x200)
#define LEON_GPIO_BASE (LEON_IOREGS+0xa00)

#define LEON_CCR_BASE 0x2

#define LEON_IFLUSH_ASI 0x15
#define LEON_DFLUSH_ASI 0x16

#else
#error Processor not valid
#endif

#define LEON_MMU_FLUSH 0x18 //0x03
#define LEON_MMU_BYPASS 0x1c
#define LEON_MMU_ASI 0x19
#define LEON_MMU_CTRL_REG 0x0
#define LEON_MMU_CPR_REG 0x100
#define LEON_MMU_CTXT_REG 0x200
#define LEON_MMU_FSTAT_REG 0x300
#define LEON_MMU_FADDR_REG 0x400

#ifdef _XM_KERNEL_

#define CONFIG_REGISTER_WINDOWS 8

// LEON_MEMORY_CFG_BASE
#define IOP_BIT 19
#define IOP_MASK (1<<IOP_BIT)

#ifndef __ASSEMBLY__


// can write together with the reset
 
static inline void StoreIoReg(xmAddress_t pAddr, xm_u32_t value) {
#if defined(CONFIG_MMU)
    __asm__ __volatile__("sta %0, [%1] %2\n\t"::"r"(value),
			 "r"(pAddr), "i"(LEON_MMU_BYPASS):"memory");
#else
    __asm__ __volatile__("st %0, [%1]\n\t"::"r"(value), "r"(pAddr):"memory");
#endif
}


static inline xm_u32_t LoadIoReg(xmAddress_t pAddr) {
    xm_u32_t retVal;
#if defined(CONFIG_MMU)
    __asm__ __volatile__("lda [%1] %2, %0\n\t": "=r"(retVal): "r"(pAddr), "i"(LEON_MMU_BYPASS));
#else
    __asm__ __volatile__("ld [%1], %0\n\t": "=r"(retVal): "r"(pAddr));
#endif
    return retVal;
}

#ifdef CONFIG_MMU
static inline xm_u8_t ReadByPassMmuByte(void *pAddr) {
    xm_u8_t retVal;
    __asm__ __volatile__("lduba [%1] %2, %0\n\t": "=r"(retVal): "r"(pAddr), "i"(LEON_MMU_BYPASS));
    return retVal;
}

static inline xm_u32_t ReadByPassMmuWord(void *pAddr) {
    xm_u32_t retVal;
    __asm__ __volatile__("lda [%1] %2, %0\n\t": "=r"(retVal): "r"(pAddr), "i"(LEON_MMU_BYPASS));
    return retVal;
}

static inline void WriteByPassMmuWord(void *pAddr, xm_u32_t val) {
    __asm__ __volatile__("sta %0, [%1] %2\n\t"::"r"(val),
			 "r"(pAddr), "i"(LEON_MMU_BYPASS):"memory");
}

#else

static inline xm_u8_t ReadByPassMmuByte(void *pAddr) {
    return *(xm_u8_t *)pAddr;
}

static inline xm_u32_t ReadByPassMmuWord(void *pAddr) {
    return *(xm_u32_t *)pAddr;
}

static inline void WriteByPassMmuWord(void *pAddr, xm_u32_t val) {
    *(xm_u32_t *)pAddr=val;
}

#endif

#endif
#endif
#endif
