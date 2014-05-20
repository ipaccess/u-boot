
#include "linux/types.h"
#include "linux/bitops.h"
#include "asm/system.h"
#include "asm/atomic.h"
#include "asm/io.h"


//TODO We're not implementing wait queues.
typedef struct
{
}wait_queue_head_t;

#define wake_up(_x) {}
//Just block when waiting
#define wait_event(wq, condition)       \
do {                                    \
    if (condition)                      \
        break;                          \
} while (1)

#define __ffs(x) (ffs(x) - 1)

#define pr_info printf
#define pr_err printf

/* PAGE_SHIFT determines the page size */
#define PAGE_SHIFT		12
//#define PAGE_SIZE		(1UL << PAGE_SHIFT)
#define PAGE_MASK		(~(PAGE_SIZE-1)) 
#define BIT(nr)			(1UL << (nr)) 
#define THIS_MODULE 0
#define dev_name(...) 0
//#define __devinit

#define dma_free_coherent(_u1, _u2, _buf, _u3) free(_buf)

struct stuby
{
    int offset;
};
enum sw_version {
	VERSION_1 = 0,
	VERSION_2,
};  
struct flash_platform_data {
	//const char	*map_name;
	//const char	*name;
	//unsigned int	width;
	//unsigned int    interleave;
	//int		(*init)(void);
	//void		(*exit)(void);
	//void		(*set_vpp)(int on);
	//void		(*mmcontrol)(struct mtd_info *mtd, int sync_read);
	//struct mtd_partition *parts;
    struct stuby* parts; //TODO rather hacky
	unsigned int	nr_parts;
	enum sw_version	version;
}; 

struct flash_platform_data msm_nand_data = {
	.parts		= NULL,
	.nr_parts	= 0,
	//.interleave     = 0,
	.version	= VERSION_2,
};
#if 0
struct nand_oobfree {
	__u32 offset;
	__u32 length;
};
#endif

#if 0
struct kvec {
	void *iov_base; /* and that should *never* hold a userland pointer */
	size_t iov_len;
}; 
#endif

typedef u32 resource_size_t; 

// These defines should have no effect
#define GFP_KERNEL 0
#define EXPORT_SYMBOL_GPL(_x) {}


#if 0
//TODO u-boot has a version of this structure, but eccpos isn't big enough.
// defined here: #include "mtd/mtd-abi.h"
#define MTD_MAX_OOBFREE_ENTRIES_LARGE	32
#define MTD_MAX_ECCPOS_ENTRIES_LARGE	448
/*
 * Internal ECC layout control structure. For historical reasons, there is a
 * similar, smaller struct nand_ecclayout_user (in mtd-abi.h) that is retained
 * for export to user-space via the ECCGETLAYOUT ioctl.
 * nand_ecclayout should be expandable in the future simply by the above macros.
 */
struct nand_ecclayout {
	__u32 eccbytes;
	__u32 eccpos[MTD_MAX_ECCPOS_ENTRIES_LARGE];
	__u32 oobavail;
	struct nand_oobfree oobfree[MTD_MAX_OOBFREE_ENTRIES_LARGE];
}; 
#endif

//TODO check these are correct
#define smp_mb()    dmb()
#define smp_rmb()   dmb()
#define smp_wmb()   dmb()

static inline int atomic_cmpxchg(atomic_t *ptr, int old, int new)
{
	unsigned long oldval, res;

	smp_mb();

	do {
		__asm__ __volatile__("@ atomic_cmpxchg\n"
		"ldrex	%1, [%3]\n"
		"mov	%0, #0\n"
		"teq	%1, %4\n"
		"strexeq %0, %5, [%3]\n"
		    : "=&r" (res), "=&r" (oldval), "+Qo" (ptr->counter)
		    : "r" (&ptr->counter), "Ir" (old), "r" (new)
		    : "cc");
	} while (res);

	smp_mb();

	return oldval;
}


// ***************** Taken from arch/arm/mach-msm/include/mach/dma.h  START ************
#if 0
 /* If the CMD_PTR register has CMD_PTR_LIST selected, the data mover
 * is going to walk a list of 32bit pointers as described below.  Each
 * pointer points to a *array* of dmov_s, etc structs.  The last pointer
 * in the list is marked with CMD_PTR_LP.  The last struct in each array
 * is marked with CMD_LC (see below).
 */
#define CMD_PTR_ADDR(addr)  ((addr) >> 3)
#define CMD_PTR_LP          (1 << 31) /* last pointer */
#define CMD_PTR_PT          (3 << 29) /* ? */ 
/* Single Item Mode */
typedef struct {
	unsigned cmd;
	unsigned src;
	unsigned dst;
	unsigned len;
} dmov_s;
 
#define DMOV_CRCIS_PER_CONF 10

#define DMOV_ADDR(off, ch) ((off) + ((ch) << 2))

#define DMOV_CMD_PTR(ch)      DMOV_ADDR(0x000, ch)
#define DMOV_CMD_LIST         (0 << 29) /* does not work */
#define DMOV_CMD_PTR_LIST     (1 << 29) /* works */
#define DMOV_CMD_INPUT_CFG    (2 << 29) /* untested */
#define DMOV_CMD_OUTPUT_CFG   (3 << 29) /* untested */
#define DMOV_CMD_ADDR(addr)   ((addr) >> 3)

#define DMOV_RSLT(ch)         DMOV_ADDR(0x040, ch)
#define DMOV_RSLT_VALID       (1 << 31) /* 0 == host has empties result fifo */
#define DMOV_RSLT_ERROR       (1 << 3)
#define DMOV_RSLT_FLUSH       (1 << 2)
#define DMOV_RSLT_DONE        (1 << 1)  /* top pointer done */
#define DMOV_RSLT_USER        (1 << 0)  /* command with FR force result */

#define DMOV_FLUSH0(ch)       DMOV_ADDR(0x080, ch)
#define DMOV_FLUSH1(ch)       DMOV_ADDR(0x0C0, ch)
#define DMOV_FLUSH2(ch)       DMOV_ADDR(0x100, ch)
#define DMOV_FLUSH3(ch)       DMOV_ADDR(0x140, ch)
#define DMOV_FLUSH4(ch)       DMOV_ADDR(0x180, ch)
#define DMOV_FLUSH5(ch)       DMOV_ADDR(0x1C0, ch)
#define DMOV_FLUSH_TYPE       (1 << 31)

#define DMOV_STATUS(ch)       DMOV_ADDR(0x200, ch)
#define DMOV_STATUS_RSLT_COUNT(n)    (((n) >> 29))
#define DMOV_STATUS_CMD_COUNT(n)     (((n) >> 27) & 3)
#define DMOV_STATUS_RSLT_VALID       (1 << 1)
#define DMOV_STATUS_CMD_PTR_RDY      (1 << 0)

#define DMOV_CONF(ch)         DMOV_ADDR(0x240, ch)
#define DMOV_CONF_SD(sd)      (((sd & 4) << 11) | ((sd & 3) << 4))
#define DMOV_CONF_IRQ_EN             (1 << 6)
#define DMOV_CONF_FORCE_RSLT_EN      (1 << 7)
#define DMOV_CONF_SHADOW_EN          (1 << 12)
#define DMOV_CONF_MPU_DISABLE        (1 << 11)
#define DMOV_CONF_PRIORITY(n)        (n << 0)

#define DMOV_DBG_ERR(ci)      DMOV_ADDR(0x280, ci)

#define DMOV_RSLT_CONF(ch)    DMOV_ADDR(0x300, ch)
#define DMOV_RSLT_CONF_FORCE_TOP_PTR_RSLT (1 << 2)
#define DMOV_RSLT_CONF_FORCE_FLUSH_RSLT   (1 << 1)
#define DMOV_RSLT_CONF_IRQ_EN             (1 << 0)

#define DMOV_ISR              DMOV_ADDR(0x380, 0)

#define DMOV_CI_CONF(ci)      DMOV_ADDR(0x390, ci)
#define DMOV_CI_CONF_RANGE_END(n)      ((n) << 24)
#define DMOV_CI_CONF_RANGE_START(n)    ((n) << 16)
#define DMOV_CI_CONF_MAX_BURST(n)      ((n) << 0)

#define DMOV_CI_DBG_ERR(ci)   DMOV_ADDR(0x3B0, ci)

#define DMOV_CRCI_CONF0       DMOV_ADDR(0x3D0, 0)
#define DMOV_CRCI_CONF1       DMOV_ADDR(0x3D4, 0)
#define DMOV_CRCI_CONF0_SD(crci, sd) (sd << (crci*3))
#define DMOV_CRCI_CONF1_SD(crci, sd) (sd << ((crci-DMOV_CRCIS_PER_CONF)*3))

#define DMOV_CRCI_CTL(crci)   DMOV_ADDR(0x400, crci)
#define DMOV_CRCI_CTL_BLK_SZ(n)        ((n) << 0)
#define DMOV_CRCI_CTL_RST              (1 << 17)
#define DMOV_CRCI_MUX                  (1 << 18)
 
/* bits for the cmd field of the above structures */
#define CMD_LC      (1 << 31)  /* last command */
#define CMD_FR      (1 << 22)  /* force result -- does not work? */
#define CMD_OCU     (1 << 21)  /* other channel unblock */
#define CMD_OCB     (1 << 20)  /* other channel block */
#define CMD_TCB     (1 << 19)  /* ? */
#define CMD_DAH     (1 << 18)  /* destination address hold -- does not work?*/
#define CMD_SAH     (1 << 17)  /* source address hold -- does not work? */

#define CMD_MODE_SINGLE     (0 << 0) /* dmov_s structure used */
#define CMD_MODE_SG         (1 << 0) /* untested */
#define CMD_MODE_IND_SG     (2 << 0) /* untested */
#define CMD_MODE_BOX        (3 << 0) /* untested */

#define CMD_DST_SWAP_BYTES  (1 << 14) /* exchange each byte n with byte n+1 */
#define CMD_DST_SWAP_SHORTS (1 << 15) /* exchange each short n with short n+1 */
#define CMD_DST_SWAP_WORDS  (1 << 16) /* exchange each word n with word n+1 */

#define CMD_SRC_SWAP_BYTES  (1 << 11) /* exchange each byte n with byte n+1 */
#define CMD_SRC_SWAP_SHORTS (1 << 12) /* exchange each short n with short n+1 */
#define CMD_SRC_SWAP_WORDS  (1 << 13) /* exchange each word n with word n+1 */

#define CMD_DST_CRCI(n)     (((n) & 15) << 7)
#define CMD_SRC_CRCI(n)     (((n) & 15) << 3) 
#endif //0
// ***************** Taken from arch/arm/mach-msm/include/mach/dma.h  END *************

// ** from mach/barriers.h ***
#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")
#define dmb() __asm__ __volatile__ ("dmb" : : : "memory") 
#define mb() dsb()
#define rmb() dsb()
#define wmb() mb()
//TODO get the following working
#if 0
#define mb() do \
	{ \
		dsb();\
		outer_sync(); \
		write_to_strongly_ordered_memory(); \
	} while (0)
#define rmb()	do { dmb(); write_to_strongly_ordered_memory(); } while (0)
#define wmb()	mb() 
#endif
// ***************************


//TODO need further investigation
#define virt_addr_valid(_x) (1)
//#define virt_to_page(_x) (0)        //TODO will fail
//#define WARN_ON(_x) (0)
//#define vmalloc_to_page(_x) (0)     //TODO will fail
//#define dma_map_page(...) (0)        //TODO will fail 
#define dma_mapping_error(...)(0)
#define dma_sync_single_for_cpu(...)
#define dma_sync_single_for_device(...)
#define dma_unmap_page(...)
//#define page_address(...) (0)         //TODO will fail

//TODO
// - Use of dma_alloc_coherent() to to be looked at, i would expect a compilation error.
// - compare the use of this with lk. msm_nand_dma_map()
