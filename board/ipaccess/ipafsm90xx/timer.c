#include <common.h>
#include <asm/io.h>
#include "iomap.h"

#if ! defined(CONFIG_SYS_HZ_CLOCK)
#define CONFIG_SYS_HZ_CLOCK 19200000
#endif
#define TIMER_LOAD_VAL (CONFIG_SYS_HZ_CLOCK/CONFIG_SYS_HZ)

#define QTMR_V1_CNTPCT_LO                (0x00000000 + QTMR_BASE)
#define QTMR_V1_CNTPCT_HI                (0x00000004 + QTMR_BASE)
#define QTMR_V1_CNTFRQ                   (0x00000010 + QTMR_BASE)
#define QTMR_V1_CNTP_CVAL_LO             (0x00000020 + QTMR_BASE)
#define QTMR_V1_CNTP_CVAL_HI             (0x00000024 + QTMR_BASE)
#define QTMR_V1_CNTP_TVAL                (0x00000028 + QTMR_BASE)
#define QTMR_V1_CNTP_CTL                 (0x0000002C + QTMR_BASE)


#define QTMR_TIMER_CTRL_ENABLE          (1 << 0)
#define QTMR_TIMER_CTRL_INT_MASK        (1 << 1)

#define QTMR_PHY_CNT_MAX_VALUE          0xFFFFFFFFFFFFFF
DECLARE_GLOBAL_DATA_PTR;

int timer_init(void)
{
    uint32_t ctrl;

    ctrl = readl(QTMR_V1_CNTP_CTL);

    /* Program CTRL Register */
    ctrl |= QTMR_TIMER_CTRL_ENABLE;
    ctrl &= ~QTMR_TIMER_CTRL_INT_MASK;

    writel(ctrl, QTMR_V1_CNTP_CTL);

    gd->arch.tbu = 0;
    gd->arch.tbl = 0;
    return 0;
}

int timer_deinit(void)
{

    uint32_t ctrl;

    ctrl = readl(QTMR_V1_CNTP_CTL);

    /* program cntrl register */
    ctrl &= ~QTMR_TIMER_CTRL_ENABLE;
    ctrl |= QTMR_TIMER_CTRL_INT_MASK;

    writel(ctrl, QTMR_V1_CNTP_CTL);
    return 0;
}

uint64_t get_timer_raw(void)
{

    uint32_t phy_cnt_lo;
    uint32_t phy_cnt_hi_1;
    uint32_t phy_cnt_hi_2;

    do {
        phy_cnt_hi_1 = readl(QTMR_V1_CNTPCT_HI);
        phy_cnt_lo = readl(QTMR_V1_CNTPCT_LO);
        phy_cnt_hi_2 = readl(QTMR_V1_CNTPCT_HI);
    } while (phy_cnt_hi_1 != phy_cnt_hi_2);

    return ((uint64_t)phy_cnt_hi_1 << 32) | phy_cnt_lo;
}

/*1 MHZ rate*/
ulong get_timer_masked(void)
{
    return (get_timer_raw()/TIMER_LOAD_VAL) & 0xFFFFFFFF; /* restrict it to 32 bit timer value, u-boot doesn't support 64 bit timer. now should roll over after 2 days approx*/ 
}

void udelay_masked(unsigned long usec)
{
    u64 tmo;
    u64 endtime;
    int64_t diff;

    tmo = CONFIG_SYS_HZ_CLOCK / CONFIG_SYS_HZ;
    tmo *= usec;
    tmo /= CONFIG_SYS_HZ;

    endtime = get_timer_raw() + tmo;

    do {
        u64 now = get_timer_raw();
        diff = endtime - now;
    } while (diff >= 0);
}

void __udelay(unsigned long usec)
{
    udelay_masked(usec);
}

ulong get_timer(ulong base)
{
    return get_timer_masked() - base;
}

unsigned long long get_ticks(void)
{
    ulong now = get_timer_masked();
    /* increment tbu if tbl has rolled over */
    if (now < gd->arch.tbl)
        gd->arch.tbu++;
    gd->arch.tbl = now;
    return (((unsigned long long)gd->arch.tbu) << 32) | gd->arch.tbl;
}

ulong get_tbclk(void)
{
    return CONFIG_SYS_HZ;
}
