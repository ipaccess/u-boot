#include "ssbi.h"
#include "tlmm.h"

#include <asm/io.h>



#define SSBI_RETRIES      1000  /* General retry limit */
#define SSBI_DELAY        1000  /* General delay in nanoseconds */

#define SSBI2_0_SSBI2_CTL (SSBI2_0_BASE+0x0)
#define SSBI2_0_SSBI2_RESET (SSBI2_0_BASE+0x4)
#define SSBI2_0_SSBI2_CMD (SSBI2_0_BASE+0x8)
#define SSBI2_0_SSBI2_BYPASS (SSBI2_0_BASE+0xc)
#define SSBI2_0_SSBI2_RD (SSBI2_0_BASE+0x10)
#define SSBI2_0_SSBI2_STATUS (SSBI2_0_BASE+0x14)
#define SSBI2_0_SSBI2_PRIORITIES (SSBI2_0_BASE+0x18)
#define SSBI2_0_SSBI2_MODE2 (SSBI2_0_BASE+0x1c)
#define SSBI2_0_SSBI2_MASK0 (SSBI2_0_BASE+0x20)

#define SSBI2_CTL_OFFSET        (SSBI2_0_SSBI2_CTL-SSBI2_0_BASE)
#define SSBI2_RESET_OFFSET      (SSBI2_0_SSBI2_RESET-SSBI2_0_BASE)
#define SSBI2_CMD_OFFSET        (SSBI2_0_SSBI2_CMD-SSBI2_0_BASE)
#define SSBI2_BYPASS_OFFSET     (SSBI2_0_SSBI2_BYPASS-SSBI2_0_BASE)
#define SSBI2_RD_OFFSET         (SSBI2_0_SSBI2_RD-SSBI2_0_BASE)
#define SSBI2_STATUS_OFFSET     (SSBI2_0_SSBI2_STATUS-SSBI2_0_BASE)
#define SSBI2_PRIORITIES_OFFSET (SSBI2_0_SSBI2_PRIORITIES-SSBI2_0_BASE)
#define SSBI2_MODE2_OFFSET      (SSBI2_0_SSBI2_MODE2-SSBI2_0_BASE)
#define SSBI2_MASK0_OFFSET      (SSBI2_0_SSBI2_MASK0-SSBI2_0_BASE)

#define SSBI_CMD_READ           (1)
#define SSBI_CMD_WRITE          (0)

#define   SSBI2_STATUS__READY___M (0x00000002)
#define   SSBI2_STATUS__MCHN_BUSY___M (0x00000001)

#define TLMMGPIO_BASE (0x94040000)
#define TLMMGPIO_GPIO_PAGE (TLMMGPIO_BASE+0x0040)
#define TLMMGPIO_GPIO_CFG (TLMMGPIO_BASE+0x0044)
#define   GPIO_CFG__FUNC_SEL___M (0x0000003C)
#define   GPIO_CFG__FUNC_SEL___S (2)

#define   SSBI2_CTL__IDLE_SYMS___S (0)
#define   SSBI2_CTL__SSBI_DATA_DEL___S (2)
#define   SSBI2_CTL__ENABLE_SSBI_INT___S (4)
#define   SSBI2_CTL__SEL_RD_DATA___S (5)
#define   SSBI2_CTL__FTM_MODE___S (7)
#define   SSBI2_CTL__SLAVE_ID___S (8)
#define   SSBI2_CTL__DISABLE_TERM_SYM___S (14)
#define   SSBI2_CTL__ACTIVATE_RESERVE___S (15)
#define   SSBI2_CTL__SSBI_DATA_PDEN___S (16)
#define   SSBI2_CTL__WAKEUP_FINAL_CNT___S (17)
#define   SSBI2_MODE2__SSBI2_MODE___S (0)
#define   SSBI2_MODE2__ADDR_WIDTH___S (1)
#define   SSBI2_MODE2__SSBI2_MODE___M (0x00000001)
#define   SSBI2_MODE2__ADDR_WIDTH___M (0x0000000E)
#define   SSBI2_CMD__REG_ADDR_14_0___S (8)
#define   SSBI2_CMD__REG_ADDR_14_0___M (0x007FFF00)
#define   SSBI2_CMD__RDWRN___S (24)
#define   SSBI2_STATUS__RD_READY___M (0x00000004)
#define   SSBI2_RD__REG_DATA___M (0x000000FF)
#define   SSBI2_CMD__REG_DATA___M (0x000000FF)

#define SSBI_REG_WRITE(offset,value) writel((value),(base+offset))
#define SSBI_REG_READ(offset) readl(base+offset)



static int _ready(u32 base)
{
    u32 v;
    int retry;

    debug("%s: starting _ready(0x%08x)\n", __func__, base);

    /*
     * Waiting for the SSBI controller to be READY
     */
    retry = 0;
    v = SSBI_REG_READ(SSBI2_STATUS_OFFSET);
    while(((v & SSBI2_STATUS__READY___M) == 0) &&
          ((v & SSBI2_STATUS__MCHN_BUSY___M) != 0))
    {
        if (retry > SSBI_RETRIES)
        {
            debug("%s: _ready(0x%08x) - retries exceeded\n", __func__, base);
            return -1;
        }
        retry++;
        debug("%s: about to udelay\n", __func__);
        udelay(SSBI_DELAY);
        debug("%s: udelay done\n", __func__);
        v = SSBI_REG_READ(SSBI2_STATUS_OFFSET);
    }

    debug("%s: ready for action!\n", __func__);
    return 0;
}



int ssbi_ready(u32 base)
{
    int tlmm;
    int func;
    u32 v;

    debug("%s: starting ssbi_ready(0x%08x)\n", __func__, base);

    switch (base)
    {
        case SSBI2_0_BASE:         /* ==== SSBI Master for PM8058 ==== */
            tlmm = 140;             /* GPIO associated with SSBI #0 */
            func = 1;
            break;
        case SSBI2_1_BASE:         /* ==== SSBI Master for FTR8700 #1 ==== */
            tlmm = 141;             /* GPIO associated with SSBI #1 */
            func = 1;
            break;
        case SSBI2_2_BASE:         /* ==== SSBI Master for FTR8700 #2 ==== */
            tlmm = 92;              /* GPIO associated with SSBI #2 */
            func = 2;
            break;
        default:
            return -1;
    }

    debug("%s: tlmm=%d, func=%d\n", __func__, tlmm, func);

    SSBI_REG_WRITE(TLMMGPIO_GPIO_PAGE, tlmm);
    v = SSBI_REG_READ(TLMMGPIO_GPIO_CFG);
    v = ((v) & GPIO_CFG__FUNC_SEL___M) >> GPIO_CFG__FUNC_SEL___S;
    if (v != (u32) func)
    {
        return -1;
    }

    return _ready(base);
}



int ssbi_init(u32 base)
{
    int tlmm;
    int func;

    switch (base)
    {
    case SSBI2_0_BASE:         /* ==== SSBI Master for PM8058 ==== */
        tlmm = 140;             /* GPIO associated with SSBI #0 */
        func = 1;
        break;

    case SSBI2_1_BASE:         /* ==== SSBI Master for FTR8700 #1 ==== */
        tlmm = 141;             /* GPIO associated with SSBI #1 */
        func = 1;
        break;

    case SSBI2_2_BASE:         /* ==== SSBI Master for FTR8700 #2 ==== */
        tlmm = 92;              /* GPIO associated with SSBI #2 */
        func = 2;
        break;

    default:
        return -1;
    }


    /*
     * Configure the TLMM GPIO associated with the SSBI controller
     */
    tlmm_drv(tlmm, TLMM_DRIVE_16MA);
    tlmm_func(tlmm, func);
    tlmm_pull(tlmm, TLMM_PULL_UP);

    /*
     * Resetting the SSBI controller to its default state
     */
    SSBI_REG_WRITE(SSBI2_RESET_OFFSET, 1);

    /*
     * Make sure "Data Pulldown Enable" bit is clear in SSBI Control
     * register
     */
    SSBI_REG_WRITE(SSBI2_CTL_OFFSET,
           (1 << SSBI2_CTL__IDLE_SYMS___S) |
           (0 << SSBI2_CTL__SSBI_DATA_DEL___S) |
           (0 << SSBI2_CTL__ENABLE_SSBI_INT___S) |
           (0 << SSBI2_CTL__SEL_RD_DATA___S) |
           (0 << SSBI2_CTL__FTM_MODE___S) |
           (0 << SSBI2_CTL__SLAVE_ID___S) |
           (0 << SSBI2_CTL__DISABLE_TERM_SYM___S) |
           (1 << SSBI2_CTL__ACTIVATE_RESERVE___S) |
           (0 << SSBI2_CTL__SSBI_DATA_PDEN___S) |
           (0 << SSBI2_CTL__WAKEUP_FINAL_CNT___S));

    /*
     * Setting SSBI Priority register
     */
    /*
     * Not used because there is no bus arbitration
     */
    SSBI_REG_WRITE(SSBI2_PRIORITIES_OFFSET, 0);

    /*
     * Waiting for the SSBI controller to be READY after the reset
     */
    if (_ready(base) < 0)
    {
        return -1;
    }

    return 0;
}



int ssbi_mode_set(u32 base, u32 mode, u32 width)
{
    if (mode == SSBI_MODE_1)
    {
        SSBI_REG_WRITE(SSBI2_MODE2_OFFSET, mode << SSBI2_MODE2__SSBI2_MODE___S);
    }
    else if (mode == SSBI_MODE_2)
    {
        if (width < 8 || width > 15)
        {
            return -1;
        }
        SSBI_REG_WRITE(SSBI2_MODE2_OFFSET, (mode << SSBI2_MODE2__SSBI2_MODE___S | (width - 8) << SSBI2_MODE2__ADDR_WIDTH___S));
    }
    else
    {
        return -1;
    }

    return 0;
}



int ssbi_mode_get(u32 base, u32 * mode, u32 * width)
{
    u32 v = SSBI_REG_READ(SSBI2_MODE2_OFFSET);
    *mode = ((v) & SSBI2_MODE2__SSBI2_MODE___M) >> SSBI2_MODE2__SSBI2_MODE___S;
    *width = (((v) & SSBI2_MODE2__ADDR_WIDTH___M) >> SSBI2_MODE2__ADDR_WIDTH___S) + 8;
    return 0;
}



int ssbi_read(u32 base, u32 addr, u32 * value)
{
    int retry;
    u32 v;

    debug("%s: starting ssbi_read(0x%08x, 0x%08x, <value>)...\n", __func__, base, addr);

    /*
     * Checking for SSBI controller to indicate it is ready for commands
     */
    if (_ready(base) < 0)
    {
        debug("%s: SSBI is not ready\n", __func__);
        return -1;
    }

    SSBI_REG_WRITE(SSBI2_CMD_OFFSET, ((addr << SSBI2_CMD__REG_ADDR_14_0___S) & SSBI2_CMD__REG_ADDR_14_0___M) | (SSBI_CMD_READ << SSBI2_CMD__RDWRN___S));

    retry = 0;
    v = SSBI_REG_READ(SSBI2_STATUS_OFFSET);
    while((v & SSBI2_STATUS__RD_READY___M) == 0)
    {
        if (retry > SSBI_RETRIES)
        {
            *value = 0;
            /*
             * Even on failure, perform the read operation to reset the
             * RD_READY bit
             */
            v = SSBI_REG_READ(SSBI2_RD_OFFSET);
            debug("%s: retries exceeded\n", __func__);
            return -1;
        }
        retry++;
        debug("%s: about to udelay\n", __func__);
        udelay(SSBI_DELAY);
        debug("%s: udelay done\n", __func__);
        v = SSBI_REG_READ(SSBI2_STATUS_OFFSET);
    }

    *value = SSBI_REG_READ(SSBI2_RD_OFFSET) & SSBI2_RD__REG_DATA___M;

    debug("%s: success, value is 0x%08x\n", __func__, *value);
    return 0;
}



int ssbi_write(u32 base, u32 addr, u32 value)
{
    int retry;
    u32 v;

    debug("%s: entering ssbi_write(0x%08x, 0x%08x, 0x%08x)\n", __func__, base, addr, value);

    /*
     * Checking for SSBI controller to indicate it is ready for commands
     */
    if (_ready(base) < 0)
    {
        debug("%s: not ready!\n", __func__);
        return -1;
    }

    /*
     * Sending write address and data to the SSBI Command register
     */
    SSBI_REG_WRITE(SSBI2_CMD_OFFSET,
                   ((addr << SSBI2_CMD__REG_ADDR_14_0___S) &
                    SSBI2_CMD__REG_ADDR_14_0___M) | (SSBI_CMD_WRITE <<
                                                     SSBI2_CMD__RDWRN___S)
                   | (value & SSBI2_CMD__REG_DATA___M));

    /*
     * Wait for the write operation to complete. Returning early can cause the
     * FTR8700 bus select GPIOs to change before the controller does the write!
     */
    retry = 0;
    v = SSBI_REG_READ(SSBI2_STATUS_OFFSET);
    while((v & SSBI2_STATUS__MCHN_BUSY___M) != 0)
    {
        if (retry > SSBI_RETRIES)
        {
            debug("%s: retries exceeded\n", __func__);
            return -1;
        }
        retry++;
        udelay(SSBI_DELAY);
        v = SSBI_REG_READ(SSBI2_STATUS_OFFSET);
        debug("%s: retrying\n", __func__);
    }

    debug("%s: OK\n", __func__);
    return 0;
}
