
extern int timer_deinit(void);


/*
 * This (badly named) hook would allow us
 * to reinitialise caches while preparing
 * to boot Linux.  The only known use of
 * this is in cpu/armv7/u8500, and this is
 * only called from cleanup_before_linux.
 *
 * So... we can repurpose this to stop our
 * timers on ipafsm92xx prior to booting the
 * kernel.
 */
void cpu_cache_initialization(void)
{
    timer_deinit();
}
