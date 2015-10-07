/*
 * $FILE: asm.h
 *
 * Processor
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_ARCH_ASM_H_
#define _XM_ARCH_ASM_H_

#ifndef __ASSEMBLY__
#include __XM_INCFLD(linkage.h)
#ifndef _GENERATE_OFFSETS_
#include __XM_INCFLD(arch/asm_offsets.h)
#endif

#ifdef CONFIG_MMU

#define ASM_EXPTABLE(_a, _b) \
    ".section .exptable, \"a\"\n\t" \
    ".align 4\n\t" \
    ".long "#_a"\n\t" \
    ".long "#_b"\n\t" \
    ".previous\n\t"

#define EXPTABLE(_a) \
    __asm__ (ASM_EXPTABLE(_a))

//_s for size; SB, SH, STUB, UH, etc....
#define ASM_RW(_s, _tmp) \
   __asm__ __volatile__ ("orn %0, %%g0, %0\n\t" : "=r" (ret)); \
   __asm__ __volatile__ ("1:ld"_s" [%2], %1\n\t" \
                         "2:st"_s" %1, [%2]\n\t" \
                         "mov %%g0, %0\n\t" \
                         "3:\n\t" \
                         ASM_EXPTABLE(1b, 3b) \
                         ASM_EXPTABLE(2b, 3b) \
                         : "=r" (ret), "=r" (_tmp) : "r" (addr));

#define GetCpuCtxt(ctxt) do { \
        __asm__ __volatile__ ("1:\n\t" \
                              "st %%g0, [%0+"TO_STR(_PREV_OFFSET)"]\n\t" \
                              "st %%g1, [%0+"TO_STR(_G1_OFFSET)"]\n\t"  \
                              "std %%g2, [%0+"TO_STR(_G2_OFFSET)"]\n\t" \
                              "std %%g4, [%0+"TO_STR(_G4_OFFSET)"]\n\t" \
                              "std %%g6, [%0+"TO_STR(_G6_OFFSET)"]\n\t" \
                              "mov %%y, %%g1\n\t"                       \
                              "st %%g1, [%0+"TO_STR(_Y_OFFSET)"]\n\t"   \
                              "rd %%psr, %%g1\n\t"                      \
                              "st %%g1, [%0+"TO_STR(_PSR_OFFSET)"]\n\t" \
                              "set 1b, %%g1\n\t"                        \
                              "st %%g1, [%0+"TO_STR(_PC_OFFSET)"]\n\t"  \
                              "add %%g1, 4, %%g1\n\t"                   \
                              "st %%g1, [%0+"TO_STR(_NPC_OFFSET)"]\n\t" \
                              : :"r" (ctxt):"g1");                     \
} while(0)

static inline xm_s32_t AsmRWCheck(xmAddress_t param, xmSize_t size, xm_u32_t align) {
    xmAddress_t addr;
    xm_s32_t ret;
    xm_u8_t tmp1;
    xm_u16_t tmp2;
    xm_u32_t tmp4;
    xm_u64_t tmp8;
    for (addr=param; addr<param+size; addr=(addr&PAGE_MASK)+PAGE_SIZE) {
        switch (align) {
        case 1:
            ASM_RW("ub", tmp1);
            break;
        case 2:
            ASM_RW("uh", tmp2);
            break;
        case 4:
            ASM_RW("", tmp4);
            break;
        case 8:
            ASM_RW("d", tmp8);
            break;
        }
        if (ret)
            return -1;
    }
    return 0;
}

//read without write back
#define ASM_RD(_s, _tmp) \
    __asm__ __volatile__ ("orn %0, %%g0, %0\n\t" : "=r" (ret)); \
    __asm__ __volatile__ ("1:ld"_s" [%2], %1\n\t" \
                          "mov %%g0, %0\n\t" \
                          "2:\n\t" \
                          ASM_EXPTABLE(1b, 2b) \
                          : "=r"(ret), "=r" (_tmp) : "r" (addr))

static inline xm_s32_t AsmROnlyCheck(xmAddress_t param, xmSize_t size, xm_u32_t align) {
    xmAddress_t addr;
    xm_s32_t ret;
    xm_u8_t tmp1;
    xm_u16_t tmp2;
    xm_u32_t tmp4;
    xm_u64_t tmp8;
    for (addr=param; addr<param+size; addr=(addr&PAGE_MASK)+PAGE_SIZE) {
        switch (align) {
        case 1:
            ASM_RD("ub", tmp1);
            break;
        case 2:
            ASM_RD("uh", tmp2);
            break;
        case 4:
            ASM_RD("", tmp4);
            break;
        case 8:
            ASM_RD("d", tmp8);
            break;
        }

        if (ret)
            return -1;
    }

    return 0;
}
#endif

/*
#define LoadPtdL1(_ptdL1, _id) do { \
    contextTab[(_id)+1]=((_ptdL1)>>4)|_PG_ARCH_PTD_PRESENT; \
    SetMmuCtxt((_id)+1); \
} while(0)
*/

#define SAVE_GLOBAL_REG(_sp) \
    "sub "#_sp", 32, "#_sp"\n\t" \
    "std %%g6, ["#_sp"+24]\n\t" \
    "std %%g4, ["#_sp"+16]\n\t" \
    "std %%g2, ["#_sp"+8]\n\t" \
    "st %%g1, ["#_sp"+4]\n\t" \
    "rd %%y, %%g5\n\t" \
    "st %%g5, ["#_sp"+0]\n\t"

#define RESTORE_GLOBAL_REG(_sp) \
    "ld ["#_sp"+0], %%g5\n\t"        \
    "wr %%g5, %%y\n\t" \
    "ldd ["#_sp"+24], %%g6\n\t" \
    "ldd ["#_sp"+16], %%g4\n\t" \
    "ldd ["#_sp"+8], %%g2\n\t" \
    "ld ["#_sp"+4], %%g1\n\t" \
    "add "#_sp", 32, "#_sp"\n\t"


//TODO REGWIN?
//Window invalid mask
//i is General-purpose in registers; l is General-purpose local registers
#define SAVE_REGWIN(_sp) \
    "sub "#_sp", 64, "#_sp"\n\t" \
    "std %%i6, ["#_sp"+24]\n\t" \
    "std %%i4, ["#_sp"+16]\n\t" \
    "std %%i2, ["#_sp"+8]\n\t" \
    "std %%i0, ["#_sp"+0]\n\t" \
    "std %%l6, ["#_sp"+56]\n\t" \
    "std %%l4, ["#_sp"+48]\n\t" \
    "std %%l2, ["#_sp"+40]\n\t" \
    "std %%l0, ["#_sp"+32]\n\t"

#define RESTORE_REGWIN(_sp) \
    "ldd ["#_sp"+24], %%i6\n\t" \
    "ldd ["#_sp"+16], %%i4\n\t" \
    "ldd ["#_sp"+8], %%i2\n\t" \
    "ldd ["#_sp"+0], %%i0\n\t" \
    "ldd ["#_sp"+56], %%l6\n\t" \
    "ldd ["#_sp"+48], %%l4\n\t" \
    "ldd ["#_sp"+40], %%l2\n\t" \
    "ldd ["#_sp"+32], %%l0\n\t"        \
    "add "#_sp", 64, "#_sp"\n\t"

#define WR_DELAY \
    "nop\n\t" \
    "nop\n\t" \
    "nop\n\t"

/* <track id="test-context-switch"> */
//TODO ...CONTEXT_SWITCH
#define CONTEXT_SWITCH(newThread, currentThread) \
    __asm__ __volatile__ (".global .Tbegin_cs, .Tend_cs\n\t" \
          ".Tbegin_cs:\n\t"                  \
          /* save the global reg */          \
          SAVE_GLOBAL_REG(%%sp) \
          /* store the in-use regwin */ \
          /* %g1 <- newThread */ \
          /* %g2 <- currentThread */ \
          /* %g3 <- sp */ \
          /* %g4 <- counter */ \
          /* %g5 <- cwp */ \
          /* %g6 <- wim */ \
          /* %g7 <- tmp */ \
          "mov %%sp, %%g3\n\t" \
          "rd %%psr, %%g7\n\t" \
          "and %%g7, 0x1f, %%g7\n\t" \
          "mov 1, %%g5\n\t" \
          "sll %%g5, %%g7, %%g5\n\t" \
          "rd %%wim, %%g6\n\t" \
          "wr %%g0, %%wim\n\t" \
          WR_DELAY \
          "clr %%g4\n\t" \
          "1:\n\t" \
          SAVE_REGWIN(%%g3) \
          "sll %%g5, 1, %%g7\n\t" \
          "srl %%g5, "TO_STR((CONFIG_REGISTER_WINDOWS-1))", %%g5\n\t" \
          "or %%g5, %%g7, %%g5\n\t" \
          "inc %%g4\n\t" \
          "andcc %%g5, %%g6, %%g0\n\t" \
          "be,a 1b\n\t" \
          "restore\n\t" \
          /* store cwp, wim, # stored regwin, 1f */ \
          /* %g4 <- counter */ \
          /* %g5 <- 1f */ \
          /* %g6 <- wim */ \
          /* %g7 <- PSR(cwp) */ \
          "sub %%g3, 16, %%g3\n\t" \
          "set 1f, %%g5\n\t" \
          "std %%g4, [%%g3+8]\n\t" \
          "rd %%psr, %%g7\n\t" \
          "std %%g6, [%%g3]\n\t" \
          /* change the stack to the new one */ \
          "ld [%1], %%g7\n\t" \
          "add %%g7, "TO_STR(_KSTACK_OFFSET)", %%g7\n\t" \
          "st %%g3, [%%g7]\n\t" \
          "st %0, [%1]\n\t" \
          /* move newthread to oldthread address*/ \
          "ld [%0+"TO_STR(_KSTACK_OFFSET)"], %%g3\n\t" \
          /* restore the new kThread's data */ \
          "ldd [%%g3], %%g6\n\t" \
          "wr %%g7, %%psr\n\t" \
          "wr %%g6, %%wim\n\t" \
          WR_DELAY \
          "ldd [%%g3+8], %%g4\n\t" \
          "add %%g3, 16, %%g3\n\t" \
          "jmp %%g5\n\t" \
          "mov %%g3, %%sp\n\t" \
          "1:\n\t" \
          /* restore the stored regwin */ \
          RESTORE_REGWIN(%%g3) \
          "dec %%g4\n\t" \
          "cmp %%g4, %%g0\n\t" \
          "bne,a 1b\n\t" \
          "save\n\t" \
          "mov %%g3, %%sp\n\t" \
          /* restore the global reg */ \
          RESTORE_GLOBAL_REG(%%sp) \
          ".Tend_cs:\n\t" \
          : : "r" (newThread), "r" (currentThread) : \
                         "g3", "g4", "g5", "g6", "g7", \
          "i0", "i1", "i2", "i3", "i4", "i5",       "i7", \
          "l0", "l1", "l2", "l3", "l4", "l5", "l6", "l7", \
          "o0", "o1", "o2", "o3", "o4", "o5",       "o7")


#ifdef CONFIG_WA_PROTECT_MMAP_PREG_WPOINT
#ifndef XM_CPU_FEATURE_WA1
#define XM_CPU_FEATURE_WA1 0x1
#endif
#define SET_WA_WATCHPOINT(_r) \
    "sethi %%hi(xmcTab), "#_r"\n\t" \
    "or "#_r", %%lo(xmcTab), "#_r"\n\t" \
    "ld ["#_r"+"TO_STR(_HPV_OFFSET+_CPUTAB_OFFSET+_FEATURES_OFFSET)"], "#_r"\n\t" \
    "andcc "#_r", "TO_STR(XM_CPU_FEATURE_WA1)", %%g0\n\t" \
    "be 5f\n\t " \
    "nop\n\t" \
    "rd %%asr24, "#_r"\n\t" \
    "or "#_r", 0x1, "#_r"\n\t"        \
    "wr "#_r", %%asr24\n\t"        \
    "rd %%asr25, "#_r"\n\t"           \
    "or "#_r", 0x3, "#_r"\n\t"           \
    "wr "#_r", %%asr25\n\t"           \
    WR_DELAY \
    "5:\n\t"
#else
#define SET_WA_WATCHPOINT(_r)
#endif

/* </track id="test-context-switch"> */
/* <track id="test-Jumping-To-Partition"> */
#if defined(CONFIG_MMU)
#define JMP_PARTITION(entry, k) do { \
    __asm__ __volatile__ ("mov %0, %%g2\n\t" \
                          "set "TO_STR(XM_PCTRLTAB_ADDR)", %%g1\n\t" \
                          "rd %%psr, %%g4\n\t" \
                          "set "TO_STR((PSR_EF_BIT|PSR_ET_BIT|PSR_PS_BIT|PSR_PIL_MASK|PSR_CWP_MASK))", %%g3\n\t" \
                          "andn %%g4, %%g3, %%g4\n\t" \
                          "wr %%g4, "TO_STR((CONFIG_REGISTER_WINDOWS-1))", %%psr\n\t" \
                          "wr %%g0, 0x2, %%wim\n\t" \
                          "nop; nop; nop\n\t" \
                          "jmp %%g2\n\t" \
                          "rett %%g2+4\n\t" : : "r" (entry): "g2"); \
} while(0)
#else // MPU
#define JMP_PARTITION(entry, k) \
   __asm__ __volatile__ ("mov %1, %%g1\n\t" \
                         "mov %0, %%g2\n\t" \
                         "rd %%psr, %%g4\n\t" \
                         "set "TO_STR((PSR_EF_BIT|PSR_ET_BIT|PSR_PS_BIT|PSR_PIL_MASK|PSR_CWP_MASK))", %%g3\n\t" \
                         "andn %%g4, %%g3, %%g4\n\t" \
                         "wr %%g4, "TO_STR((CONFIG_REGISTER_WINDOWS-1))", %%psr\n\t" \
                         "wr %%g0, 0x2, %%wim\n\t" \
                         "nop; nop; nop\n\t" \
                         "set "TO_STR((LEON_MEMORY_CFG_BASE))", %%l5\n\t" \
                         "set (1<<19), %%l7\n\t" \
                         "ld [%%l5], %%l6\n\t" \
                         "andn %%l6, %%l7, %%l6\n\t" \
                         "st %%l6, [%%l5]\n\t" \
                         "set "TO_STR((LEON_MEMORY_WPR_BASE+0))", %%l5\n\t" \
                         "set (1<<31), %%l7\n\t" \
                         "ld [%%l5], %%l6\n\t" \
                         "or %%l6, %%l7, %%l6\n\t" \
                         "st %%l6, [%%l5]\n\t" \
                         SET_WA_WATCHPOINT(%%l5) \
                         "jmp %%g2\n\t" \
                         "rett %%g2+4\n\t" : : "r" (entry), "r"((k)->ctrl.g->partCtrlTab): "g1", "g2")
#endif
/* </track id="test-Jumping-To-Partition">  */

#endif

#endif
