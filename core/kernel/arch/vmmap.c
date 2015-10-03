/*
 * $FILE: vmmap.c
 *
 * Virtual memory map management
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
#include <boot.h>
#include <rsvmem.h>
#include <kthread.h>
#include <stdc.h>
#include <vmmap.h>
#include <virtmm.h>
#include <physmm.h>
#include <smp.h>
#include <arch/xm_def.h>
#include <arch/spinlock.h>
#include <arch/processor.h>
#include <arch/paging.h>
#include <arch/physmm.h>

void SetupVmMap(xmAddress_t *stFrameArea, xm_s32_t *noFrames) {
//
    xmAddress_t st, end;
    xm_s32_t e, flags;

    st=xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].startAddr;
    end=st+xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].size-1;
    flags=xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].flags;

    *stFrameArea=ROUNDUP(_PHYS2VIRT(end+1), LPAGE_SIZE);
    *noFrames=((XM_VMAPEND-*stFrameArea)+1)/PAGE_SIZE;

    //_PHYS2VIRT(x) ((xm_u32_t)(x)+CONFIG_XM_OFFSET-CONFIG_XM_LOAD_ADDR)
    st=_PHYS2VIRT(st)-CONFIG_XM_OFFSET; //== st - xm_load_addr
    for (e=0; e<(xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].size>>PTDL3_SHIFT); e++) {
        // deal with all cache
        //TODO should put this if statement outside; this if make for-loop slow.
        if (flags&XM_MEM_AREA_UNCACHEABLE)
            _ptdL3[(st+(e<<PTDL3_SHIFT))>>PTDL3_SHIFT]&=~_PG_ARCH_CACHE;
        else
            _ptdL3[(st+(e<<PTDL3_SHIFT))>>PTDL3_SHIFT]|=_PG_ARCH_CACHE;
    }

    for (e=0; e<*noFrames; e++)
        _ptdL3[(*stFrameArea-CONFIG_XM_OFFSET+e*PAGE_SIZE)>>PTDL3_SHIFT]=0;
    _ptdL3[XM_PCTRLTAB_ADDR>>PTDL2_SHIFT]=0;
    //flush cache and flush tlb
    FlushTlbGlobal();
}

void CloneXMPtdL1(xmWord_t *ptdL1) {
//
    xm_s32_t l1e;
    l1e=VA2PtdL1(CONFIG_XM_OFFSET);
    //write _pgTables -> ptdL1
    WriteByPassMmuWord(&ptdL1[l1e], _pgTables[l1e]);
}

void SetupPtdL1(xmWord_t *ptdL1, kThread_t *k) {
//
    static xm_u8_t mmuCtxt=0;

    CloneXMPtdL1(ptdL1);
    // mmuCtxt is static, so it will increase for each kthread reset
    k->ctrl.g->kArch.mmuCtxt=++mmuCtxt;
    //TODO don't understand
    contextTab[k->ctrl.g->kArch.mmuCtxt]=((xmAddress_t)ptdL1>>4)|_PG_ARCH_PTD_PRESENT;
}

xm_u32_t VmArchAttr2Attr(xm_u32_t entry) {
//
    xm_u32_t flags=entry&(PAGE_SIZE-1), attr=0;

    if (flags&_PG_ARCH_PTE_PRESENT) attr|=_PG_ATTR_PRESENT;
    if ((flags&_PG_ARCH_RW_USER)|(flags&_PG_ARCH_R_USER))
        attr|=_PG_ATTR_USER;
    if ((flags&_PG_ARCH_RW_USER)|(flags&_PG_ARCH_RW))
        attr|=_PG_ATTR_RW;
    if (flags&_PG_ARCH_CACHE) attr|=_PG_ATTR_CACHED;

    return attr|(flags&~0x9f);
}

xm_u32_t VmAttr2ArchAttr(xm_u32_t flags) {
//
    xm_u32_t attr=0;

    if (flags&_PG_ATTR_PRESENT) attr|=_PG_ARCH_PTE_PRESENT;
    if (flags&_PG_ATTR_USER) {
        if (flags&_PG_ATTR_RW)
            attr|=_PG_ARCH_RW_USER;
        else
            attr|=_PG_ARCH_R_USER;
    } else {
        if (flags&_PG_ATTR_RW)
            attr|=_PG_ARCH_RW;
        else
            attr|=_PG_ARCH_R;
    }

    if (flags&_PG_ATTR_CACHED) attr|=_PG_ARCH_CACHE;
    return attr|(flags&0xffff);
}


xm_s32_t VmMapUserPage(partition_t *k, xmWord_t *ptdL1, xmAddress_t pAddr, xmAddress_t vAddr, xm_u32_t flags,
        xmAddress_t (*alloc)(struct xmcPartition *, xmSize_t, xm_u32_t, xmAddress_t *, xmSSize_t *),
        xmAddress_t *pool, xmSSize_t *poolSize) {
//TODO update 3 level page table and cache them?
    struct physPage *pagePtdL2, *pagePtdL3;
    xmWord_t *pPtdL2, *pPtdL3;
    xm_u32_t l1e, l2e, l3e, clearPage=0;
    xmAddress_t pT;
    xm_s32_t e;

    ASSERT(!(pAddr&(PAGE_SIZE-1)));
    ASSERT(!(vAddr&(PAGE_SIZE-1)));
    ASSERT((vAddr<CONFIG_XM_OFFSET)||(vAddr>(CONFIG_XM_OFFSET+(16*1024*1024)-1)));
    l1e=VA2PtdL1(vAddr);
    l2e=VA2PtdL2(vAddr);
    l3e=VA2PtdL3(vAddr);
    //
    if (!(ReadByPassMmuWord(&ptdL1[l1e])&_PG_ARCH_PTD_PRESENT)) {
        //not in ptd
        //allocate a page
        if ((pT=alloc(k->cfg, PAGE_SIZE, PAGE_SIZE, pool, poolSize))==~0) {
            return -1;
        }
        WriteByPassMmuWord(&ptdL1[l1e], (pT>>4)|_PG_ARCH_PTD_PRESENT);
        if (!(pagePtdL2=PmmFindPage(pT, k, 0))) {
            //could not put into level 2 ptd
            return -1;
        }
        pagePtdL2->type=PPAG_PTDL2;
        PPagIncCounter(pagePtdL2);
        clearPage=1;
    } else {
        pT=(ReadByPassMmuWord(&ptdL1[l1e])<<4)&PAGE_MASK;
        if (!(pagePtdL2=PmmFindPage(pT, k, 0))) {
            return -1;
        }
        ASSERT(pagePtdL2);
        ASSERT(pagePtdL2->type==PPAG_PTDL2);
        ASSERT(pagePtdL2->counter>0);
    }

    //cache ptd in first then clean
    pPtdL2=VCacheMapPage(pT, pagePtdL2);
    if (clearPage)
        for (e=0; e<PAGE_SIZE/sizeof(xmWord_t); e++)
            WriteByPassMmuWord(&pPtdL2[e], 0);

    if (!(ReadByPassMmuWord(&pPtdL2[l2e])&_PG_ARCH_PTD_PRESENT)) {
        //if in level 2 page table
        if ((pT=alloc(k->cfg, PAGE_SIZE, PAGE_SIZE, pool, poolSize))==~0) {
            return -1;
        }
        WriteByPassMmuWord(&pPtdL2[l2e], (pT>>4)|_PG_ARCH_PTD_PRESENT);
        if (!(pagePtdL3=PmmFindPage(pT, k, 0))) {
            return -1;
        }
        pagePtdL3->type=PPAG_PTDL3;
        PPagIncCounter(pagePtdL3);
    } else {
        pT=(ReadByPassMmuWord(&pPtdL2[l2e])<<4)&PAGE_MASK;
        if (!(pagePtdL3=PmmFindPage(pT, k, 0))) {
            return -1;
        }
        ASSERT(pagePtdL3);
        ASSERT(pagePtdL3->type==PPAG_PTDL3);
        ASSERT(pagePtdL3->counter>0);
    }
    pPtdL3=VCacheMapPage(pT, pagePtdL3);
    WriteByPassMmuWord(&pPtdL3[l3e], ((pAddr&PAGE_MASK)>>4)|VmAttr2ArchAttr(flags));
    VCacheUnlockPage(pagePtdL2);
    VCacheUnlockPage(pagePtdL3);

    return 0;
}

void VmMapPage(xmAddress_t pAddr, xmAddress_t vAddr, xmWord_t flags) {
//un-cacheable in PTDL3
//usually used in - for (p = 0; p < (noPages * PAGE_SIZE); p += PAGE_SIZE)
    ASSERT(!(pAddr&(PAGE_SIZE-1)));
    ASSERT(!(vAddr&(PAGE_SIZE-1)));
    ASSERT(vAddr>=CONFIG_XM_OFFSET);
    // XXX: all pages are mapped as non-cacheable within XM //TODO why non-cacheable
    _ptdL3[(vAddr-CONFIG_XM_OFFSET)>>PTDL3_SHIFT]=(pAddr>>4)|(VmAttr2ArchAttr(flags)&~_PG_ARCH_CACHE);

    FlushTlbEntry(vAddr);
}
