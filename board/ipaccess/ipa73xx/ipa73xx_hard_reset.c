/*
 * (C) Copyright 2014 ip.access Ltd
*/

/*
 * IPA73XX platform hard reset command.
 *  Implements a custom command "hreset" which asserts the GPIO line ARM17,
 *  which is wired to perform a hard reset on ipa400 boards
 *  (note that this will not work on ipa267 boards)
 *
 */

#include <common.h>
#include <command.h>
#include <asm/arch/picoxcell.h>
#include <asm/arch/picoxcell_gpio.h>
#include <asm/arch/axi2cfg.h>

/* GPIO control registers */
#define GPIO_MUX_REG_ADDR (PICOXCELL_AXI2CFG_BASE + AXI2CFG_SHD_GPIO_MUXING_REG_OFFSET)

/* Register write & read functions */
#define IOWRITE32(vALUE, aDDRESS)   *((volatile u32 *)(aDDRESS)) = (vALUE)
#define IOREAD32(aDDRESS)           (*((volatile u32 *)(aDDRESS)))

/* Values for GPIO lines */
#define GPIO_HI 1
#define GPIO_LO 0

#define HARD_RESET_GPIO             PC3X3_GPIO_PIN_ARM_17
#define HARD_RESET_MUX_MASK         (1 << HARD_RESET_GPIO)


#if defined(CONFIG_CMD_HARD_RESET) || defined(CONFIG_CMD_HARD_RESET_FB)

/*---------------------------------------------------------------------------*/
/* run the set_led command.
 */
int do_hard_reset (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    ulong pinmux;
    
    /*
     * GPIO pin ARM17 is multiplexed with the picoarray, so we have to make sure
     * it is configured as ARM GPIO - set bit 17 of AXI2CFG shared gpio muxing register
     */
    pinmux = IOREAD32(GPIO_MUX_REG_ADDR);
    pinmux |= HARD_RESET_MUX_MASK;
    IOWRITE32(pinmux, GPIO_MUX_REG_ADDR);

    /* Request the pin, set it as an output, and pull it low */
    picoxcell_gpio_request(HARD_RESET_GPIO);
    
    printf("Initiating hard reset...\n\n");
    
    picoxcell_gpio_direction_output(HARD_RESET_GPIO, GPIO_LO);
    
    /* Wait a bit for reset to take effect */
    mdelay(500);
    
    /* If we get here, the reset hasn't worked, so release the GPIO and complain */
    picoxcell_gpio_free(HARD_RESET_GPIO);
    
#if defined(CONFIG_CMD_HARD_RESET_FB)
    printf("Attempted hard reset failed, falling back to a normal reset...\n");
    do_reset(cmdtp, flag, argc, argv);
#else
    printf("Attempted hard reset failed!\n");
#endif
    
    return CMD_RET_FAILURE;
}

/*===========================================================================*/
U_BOOT_CMD(
    hreset, 1, 0,    do_hard_reset,
    "Perform hard RESET of the CPU",
    ""
);

#endif /* CONFIG_CMD_HARD_RESET */
