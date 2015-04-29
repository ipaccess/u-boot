/*
 * Copyright (c) 2015, ip.access Ltd.
 */

#include <common.h>
#include <asm/io.h>
#include <linux/types.h>

// Pad name     : ifc_ad12      | ant1_dio11
// Signal name  : THERM_ALERT   | MCU_RESET
// GPIO         : 38            | 24
// IRQ          : 09            | --

#define TOP_BIT             (0x80000000)
#define GPIO_0_BASE         (0xff70f000)
#define THERM_ALERT_GPIO    (38)
#define MCU_RESET_GPIO      (24)

//#define GPIO_TO_CHIP(_gpio)     ((_gpio & ~(32-1)) >> 5)
#define GPIO_TO_CHIP(_gpio)     (_gpio >> 5)
#define GPIO_BIT_POS(_gpio)     (_gpio & (32-1))
#define GPIO_BASE_ADDR(_gpio)   (GPIO_0_BASE + 0x100 * GPIO_TO_CHIP(_gpio))
#define GPIO_DIR_REG(_gpio)     (*(volatile uint32_t*)(GPIO_BASE_ADDR(_gpio) + 0x0))
#define GPIO_DAT_REG(_gpio)     (*(volatile uint32_t*)(GPIO_BASE_ADDR(_gpio) + 0x8))

#define GPIO_SET_INPUT(_gpio) \
            GPIO_DIR_REG(_gpio) &= ~(TOP_BIT >> GPIO_BIT_POS(_gpio))
#define GPIO_SET_OUTPUT(_gpio) \
            GPIO_DIR_REG(_gpio) |= (TOP_BIT >> GPIO_BIT_POS(_gpio))

#define GPIO_IS_HI(_gpio) \
            GPIO_DAT_REG(_gpio) & (TOP_BIT >> GPIO_BIT_POS(_gpio))
#define GPIO_IS_LO(_gpio) \
            (!(GPIO_IS_HI(_gpio)))

#define GPIO_SET_HI(_gpio) \
            GPIO_DAT_REG(_gpio) |= (TOP_BIT >> GPIO_BIT_POS(_gpio))
#define GPIO_SET_LO(_gpio) \
            GPIO_DAT_REG(_gpio) &= ~(TOP_BIT >> GPIO_BIT_POS(_gpio))

int ipa9131_stall_on_thermal_alert(void)
{
    //THERM_ALERT_GPIO is Low when there is a thermal alert.
    //MCU_RESET must be held low to keep the microntroller in reset and cause the PAs to be powered down.

    GPIO_SET_INPUT(THERM_ALERT_GPIO);
    GPIO_SET_OUTPUT(MCU_RESET_GPIO);

    // Keep the MCU in reset.
    GPIO_SET_LO(MCU_RESET_GPIO);

    if(GPIO_IS_LO(THERM_ALERT_GPIO))
    {
#if !(defined(CONFIG_SPL_BUILD)) // We're almost out of space.
        puts("Thermal Alert! stall boot until in temperature.\n");
#endif
        while(GPIO_IS_LO(THERM_ALERT_GPIO))
        {
        }
        // Take the MCU out of resest
        GPIO_SET_HI(MCU_RESET_GPIO);
    }
    return 0;
}

// Some test code.
#if 0
static int do_temp_monitor(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    uint32_t data_reg;

    GPIO_SET_INPUT(THERM_ALERT_GPIO);
    GPIO_SET_OUTPUT(MCU_RESET_GPIO);

    data_reg = GPIO_DAT_REG(THERM_ALERT_GPIO);

    printf("%s\n", __func__);

    printf("Data reg: 0x%x\n", data_reg);
    printf("Bit: 0x%x\n",GPIO_IS_HI(THERM_ALERT_GPIO));

    if(2 != argc)
    {
        return CMD_RET_SUCCESS;
    }

    if(0 == strcmp(argv[1], "on"))
    {
        printf("%s mcu reset hi\n",__func__);
        GPIO_SET_HI(MCU_RESET_GPIO);
    }
    else if(0 == strcmp(argv[1], "off"))
    {
        printf("%s mcu reset lo\n",__func__);
        GPIO_SET_LO(MCU_RESET_GPIO);
    }

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
    temp_monitor, 2, 1, do_temp_monitor,
           "",
           ""
);
#endif

