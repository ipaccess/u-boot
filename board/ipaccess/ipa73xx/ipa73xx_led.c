/*
 * (C) Copyright 2013 ip.access Ltd
*/

/*
 * IPA73XX platform LED control 
 *
 */

#include <common.h>
#include <command.h>
#include <asm/arch/picoxcell.h>
#include <asm/arch/picoxcell_gpio.h>
#include <asm/arch/axi2cfg.h>
#include "ipa73xx_led.h"

void setLED(led_col col)
{
    
    ulong pinmux;

    /* We have to directly write into registers due to bug in Pico muxin code */
    /* Setup AXI2CFG shared gpio muxing register */
    pinmux = IOREAD32(PICOXCELL_AXI2CFG_BASE + AXI2CFG_SHD_GPIO_MUXING_REG_OFFSET);

    /* Make sure that ARM GPIO 15 is driving the device pin
    * (Set bit 15)
    */
    pinmux |= (1<<PC3X3_GPIO_PIN_ARM_15);
    IOWRITE32(pinmux, PICOXCELL_AXI2CFG_BASE + AXI2CFG_SHD_GPIO_MUXING_REG_OFFSET);

    /* Setup AXI2CFG use ebi gpio register */
    pinmux = IOREAD32(PICOXCELL_AXI2CFG_BASE + AXI2CFG_USE_EBI_GPIO_REG_OFFSET);

    /* Make sure that ebi[26] is a gpio pin (Set bit 12) */
    pinmux |= (1<<12);
    IOWRITE32(pinmux, PICOXCELL_AXI2CFG_BASE + AXI2CFG_USE_EBI_GPIO_REG_OFFSET);

    /* Request pin and set direction for output */
    picoxcell_gpio_request(PC3X3_GPIO_PIN_ARM_15);
    picoxcell_gpio_direction_output(PC3X3_GPIO_PIN_ARM_15, 0);


    if (col == LED_GREEN)
    {
        picoxcell_gpio_set_value(PC3X3_GPIO_PIN_ARM_15, 1);
    }
    else if (col == LED_RED)
    {
        picoxcell_gpio_set_value(PC3X3_GPIO_PIN_ARM_15, 0);
    }

    picoxcell_gpio_free(PC3X3_GPIO_PIN_ARM_15);
}



#ifdef CONFIG_CMD_LEDSET

/*---------------------------------------------------------------------------*/
/* run the set_led command.
 */
int do_set_led (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int     result = 1;

    if (argc == 2)
    {
        if (strcmp (argv[1], "green") == 0)
        {
            setLED(LED_GREEN);
            result = 0;
        }
        else if (strcmp (argv[1], "red") == 0)
        {
            setLED(LED_RED);
            result = 0;
        }
    }

    if (result)
    {
        printf ("Usage:\n%s%s\n", cmdtp->usage, cmdtp->help);
    }

    return result;
}

/*===========================================================================*/

U_BOOT_CMD(
    setled,   2,   1,     do_set_led,
    "set_led - change colour of Sys/Pwr LED",
    "  args: [red | green]"
);

#endif /* CONFIG_CMD_LEDSET */

