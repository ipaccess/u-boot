#include <common.h>
#include <asm/io.h>

#if ! defined(CONFIG_SYS_HZ_CLOCK)
#define CONFIG_SYS_HZ_CLOCK 32768
#endif
#define TIMER_LOAD_VAL (CONFIG_SYS_HZ_CLOCK/CONFIG_SYS_HZ)

#define SCSS_TMR_BASE (0x9C000000)
#define SCSS_TMR_SCSS_GPT0_EN (SCSS_TMR_BASE+0x000C)
#define SCSS_TMR_SCSS_GPT0_CLR (SCSS_TMR_BASE+0x0010)
#define SCSS_TMR_SCSS_GPT0_MTCH (SCSS_TMR_BASE+0x0004)
#define SCSS_TMR_SCSS_GPT0_CNT (SCSS_TMR_BASE+0x0008)

#define GPT0_DISABLE_FLAG (0x00000000)
#define GPT0_ENABLE_FLAG (0x00000001)
#define GPT0_AUTORELOAD_ON_MAX_FLAG (0x00000002)

int timer_init(void)
{
    writel(GPT0_DISABLE_FLAG, SCSS_TMR_SCSS_GPT0_EN);      /* Disable timer */
    writel(0xffffffff, SCSS_TMR_SCSS_GPT0_CLR);            /* Clear timer */
    writel(0xffffffff, SCSS_TMR_SCSS_GPT0_MTCH);           /* Set reload value to max */
    writel(GPT0_AUTORELOAD_ON_MAX_FLAG | GPT0_ENABLE_FLAG, /* Enable and activate auto-reload */
           SCSS_TMR_SCSS_GPT0_EN);
    return 0;
}

int timer_deinit(void)
{
    writel(GPT0_DISABLE_FLAG, SCSS_TMR_SCSS_GPT0_EN);
    return 0;
}

ulong get_timer_raw(void)
{
    return readl(SCSS_TMR_SCSS_GPT0_CNT);
}

ulong get_timer_masked(void)
{
    return get_timer_raw()/TIMER_LOAD_VAL;
}

void udelay_masked(unsigned long usec)
{
    u32 tmo;
    u32 endtime;
    signed long diff;

    tmo = CONFIG_SYS_HZ_CLOCK / CONFIG_SYS_HZ;
    tmo *= usec;
    tmo /= CONFIG_SYS_HZ;

    endtime = get_timer_raw() + tmo;

    do {
        u32 now = get_timer_raw();
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
    return get_timer(0);
}

ulong get_tbclk(void)
{
    return CONFIG_SYS_HZ;
}
