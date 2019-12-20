
#ifndef DEBUG_H
#define DEBUG_H
#include <asm/io.h>
#include <common.h>

/*Definitions from various parts of LK code*/
#define CACHE_LINE 32
#define addr_t unsigned long
#define BIT(x)          (1 << (x))
#define arch_clean_invalidate_cache_range flush_cache
#define arch_invalidate_cache_range flush_cache

#define REG64(addr) ((volatile uint64_t *)(addr))
#define REG32(addr) ((volatile uint32_t *)(addr))
#define REG16(addr) ((volatile uint16_t *)(addr))
#define REG8(addr) ((volatile uint8_t *)(addr))

#define RMWREG64(addr, startbit, width, val) *REG64(addr) = (*REG64(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG32(addr, startbit, width, val) *REG32(addr) = (*REG32(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG16(addr, startbit, width, val) *REG16(addr) = (*REG16(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG8(addr, startbit, width, val) *REG8(addr) = (*REG8(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))

#define writehw(v, a) (*REG16(a) = (v))
#define readhw(a) (*REG16(a))

#define INT_MAX     ((int)(~0U>>1))
#define UINT_MAX (INT_MAX * 2U + 1U)

/*clashing with standard sdhci driver*/
#define sdhci_send_command lk_sdhci_send_command
#define sdhci_reset lk_sdhci_reset
#define sdhci_init lk_sdhci_init

#define ASSERT(x) \
    do { if (unlikely(!(x))) { panic("ASSERT FAILED at (%s:%d): %s\n", __FILE__, __LINE__, #x); } } while (0)

#define ROUNDUP(a, b) (((a) + ((b)-1)) & ~((b)-1))
#define ROUNDDOWN(a, b) ((a) & ~((b)-1))

/* Macro returns UINT_MAX in case of overflow */
#define ROUND_TO_PAGE(x,y) ((ROUNDUP((x),((y)+1)) < (x))?UINT_MAX:ROUNDUP((x),((y)+1)))

/* allocate a buffer on the stack aligned and padded to the cpu's cache line size */
#define STACKBUF_DMA_ALIGN(var, size) \
    uint8_t __##var[(size) + CACHE_LINE] __attribute__((aligned(CACHE_LINE))); uint8_t *var = (uint8_t *)(ROUNDUP((addr_t)__##var, CACHE_LINE))

/* Macro to allocate buffer in both local & global space, the STACKBUF_DMA_ALIGN cannot
 * be used for global space.
 * If we use STACKBUF_DMA_ALIGN 'C' compiler throws the error "Initializer element
 * is not constant", since global variable need to be initialized with a constant value.
 */
#define BUF_DMA_ALIGN(var, size) \
    static uint8_t var[ROUNDUP(size, CACHE_LINE)] __attribute__((aligned(CACHE_LINE)));

#define CRITICAL 0
#define ALWAYS 0
#define INFO 1
#define SPEW 2
#define DEBUGLEVEL CRITICAL 
#define dprintf(level, x...) do { if ((level) <= DEBUGLEVEL) { printf(x); } } while (0)
#endif
