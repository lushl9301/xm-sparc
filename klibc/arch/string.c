/*
 * $FILE: string.c
 *
 * String related functions
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#include <stdc.h>
#include <arch/xm_def.h>

#ifdef __ARCH_MEMCPY

static inline void *_MemCpy1(xm_s8_t *dst, const xm_s8_t *src, xm_u32_t count) {
    register xm_s8_t *d=dst;
    register const xm_s8_t *s=src;
    ++count;
    while (--count) {
	*d=*s;
	++d; ++s;
    }
    return dst;
}

static inline void *_MemCpy2(xm_s16_t *dst, const xm_s16_t *src, xm_u32_t count) {
    register xm_s16_t *d=dst;
    register const xm_s16_t *s=src;
    ++count;
    while (--count) {
	*d=*s;
	++d; ++s;
    }
    return dst;
}

static inline void *_MemCpy4(xm_s32_t *dst, const xm_s32_t *src, xm_u32_t count) {
    register xm_s32_t *d=dst;
    register const xm_s32_t *s=src;
    ++count;
    while (--count) {
	*d=*s;
	++d; ++s;
    }
    return dst;
}

static inline void *_MemCpy8(xm_s64_t *dst, const xm_s64_t *src, xm_u32_t count) {
    register xm_s64_t *d=dst;
    register const xm_s64_t *s=src;
    ++count;
    while (--count) {
	*d=*s;
	++d; ++s;
    }
    return dst;
}

void *memcpy(void *dst, const void *src, xm_u32_t count) {
    xm_s8_t *d, *s;
    xm_s32_t r;
    d=(xm_s8_t *)dst;
    s=(xm_s8_t *)src;
    // Are both buffers aligned to 8 bytes?
    if (!(((xm_u32_t)dst|(xm_u32_t)src)&7)) {
	r=count&7;
	_MemCpy8((xm_s64_t *)d, (xm_s64_t *)s, count>>3);
	if (!r)
	    return dst;
	d=&d[count-r];
	s=&s[count-r];
	count=r;
    }
    // Are both buffers aligned to 4 bytes?
    if (!(((xm_u32_t)dst|(xm_u32_t)src)&3)) {
	r=count&3;
	_MemCpy4((xm_s32_t *)d, (xm_s32_t *)s, count>>2);
	if (!r)
	    return dst;
	d=&d[count-r];
	s=&s[count-r];
	count=r;
    }
    // Are both buffers aligned to 2 bytes?
    if (!(((xm_u32_t)dst|(xm_u32_t)src)&1)) {
	r=count&1;
	_MemCpy2((xm_s16_t *)d, (xm_s16_t *)s, count>>1);
	if (!r)
	    return dst;
	d=&d[count-r];
	s=&s[count-r];
	count=r;
    }

    // Copying the buffers byte per byte
    return _MemCpy1(d, s, count);
}

#if 0
static inline void *_MemCpyPhys1(xm_s8_t *dst, const xm_s8_t *src, xm_u32_t count) {
    register xm_s8_t *d=dst;
    register const xm_s8_t *s=src;

    ++count;
    while (--count) {
        __asm__ __volatile__("lduba [%1] %2, %%g1\n\t" \
                             "stba %%g1, [%0] %2\n\t": : "r"(d), "r"(s), "i"(LEON_MMU_BYPASS): "g1");

	++d; ++s;
    }
    return dst;
}

static inline void *_MemCpyPhys2(xm_s16_t *dst, const xm_s16_t *src, xm_u32_t count) {
    register xm_s16_t *d=dst;
    register const xm_s16_t *s=src;
    ++count;
    while (--count) {
        __asm__ __volatile__("lduha [%1] %2, %%g1\n\t"                  \
                             "stha %%g1, [%0] %2\n\t": : "r"(d), "r"(s), "i"(LEON_MMU_BYPASS): "g1");
	++d; ++s;
    }
    return dst;
}

static inline void *_MemCpyPhys4(xm_s32_t *dst, const xm_s32_t *src, xm_u32_t count) {
    register xm_s32_t *d=dst;
    register const xm_s32_t *s=src;
    ++count;
    while (--count) {
        __asm__ __volatile__("lda [%1] %2, %%g1\n\t" \
                             "sta %%g1, [%0] %2\n\t": : "r"(d), "r"(s), "i"(LEON_MMU_BYPASS): "g1");
	++d; ++s;
    }
    return dst;
}

static inline void *_MemCpyPhys8(xm_s64_t *dst, const xm_s64_t *src, xm_u32_t count) {
    register xm_s64_t *d=dst;
    register const xm_s64_t *s=src;
    ++count;
    while (--count) {
        __asm__ __volatile__("ldda [%1] %2, %%g2\n\t" \
                             "stda %%g2, [%0] %2\n\t": : "r"(d), "r"(s), "i"(LEON_MMU_BYPASS): "g2", "g3");
	++d; ++s;
    }
    return dst;
}

void *MemCpyPhys(void *dst, const void *src, xm_u32_t count) {
    xm_s8_t *d, *s;
    xm_s32_t r;
    d=(xm_s8_t *)dst;
    s=(xm_s8_t *)src;
    // Are both buffers aligned to 8 bytes?
    if (!(((xm_u32_t)dst|(xm_u32_t)src)&7)) {
	r=count&7;
	_MemCpyPhys8((xm_s64_t *)d, (xm_s64_t *)s, count>>3);
	if (!r)
	    return dst;
	d=&d[count-r];
	s=&s[count-r];
	count=r;
    }
    // Are both buffers aligned to 4 bytes?
    if (!(((xm_u32_t)dst|(xm_u32_t)src)&3)) {
	r=count&3;
	_MemCpyPhys4((xm_s32_t *)d, (xm_s32_t *)s, count>>2);
	if (!r)
	    return dst;
	d=&d[count-r];
	s=&s[count-r];
	count=r;
    }
    // Are both buffers aligned to 2 bytes?
    if (!(((xm_u32_t)dst|(xm_u32_t)src)&1)) {
	r=count&1;
	_MemCpyPhys2((xm_s16_t *)d, (xm_s16_t *)s, count>>1);
	if (!r)
	    return dst;
	d=&d[count-r];
	s=&s[count-r];
	count=r;
    }

    // Copying the buffers byte per byte
    return _MemCpyPhys1(d, s, count);
}
#endif

#endif
