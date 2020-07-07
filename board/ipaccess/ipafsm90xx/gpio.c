#include <common.h>
#include "iomap.h"
#include "debug.h"

/* Values for GPIO lines */
#define GPIO_HI 1
#define GPIO_LO 0

#define GPIO_CONFIG_ADDR(x)     (TLMM_BASE_ADDR + 0x1000 + (x)*0x10)
#define GPIO_IN_OUT_ADDR(x)     (TLMM_BASE_ADDR + 0x1004 + (x)*0x10)
#define GPIO_OUT_BIT            1
#define GPIO_IN_BIT             0
#define GPIO_OE_BIT             9


int simple_set_gpio(unsigned int gpio_num, unsigned int value)
{

    unsigned int reg_cfg = GPIO_CONFIG_ADDR(gpio_num);
    unsigned int reg_inout = GPIO_IN_OUT_ADDR(gpio_num);

    /*Make the gpio output*/
    *REG32(reg_cfg) = *REG32(reg_cfg) | (1 << GPIO_OE_BIT);

    *REG32(reg_inout) = (value ? (1 << GPIO_OUT_BIT):0);

    return 0;
}

