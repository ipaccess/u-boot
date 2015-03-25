#include "led.h"

#include <common.h>
#include <asm/io.h>




struct simple_gpio_chip {
    void * direction;
    void * data;
};

static struct simple_gpio_chip simple_gpio_chips[] = {
    { .direction = (void *)0xff70f000, .data = (void *)0xff70f008 },
    { .direction = (void *)0xff70f100, .data = (void *)0xff70f108 },
    { .direction = (void *)0xff70f200, .data = (void *)0xff70f208 },
};

static int simple_set_gpio(unsigned int gpio_num, unsigned int value)
{
    int chip;
    int mask;

    chip = gpio_num / 32;

    if (chip > 2)
    {
        return -1;
    }

    mask = 1U << ((32 - (gpio_num % 32)) - 1);

    setbits_be32(simple_gpio_chips[chip].direction, mask);

    if (value)
    {
        setbits_be32(simple_gpio_chips[chip].data, mask);
    }
    else
    {
        clrbits_be32(simple_gpio_chips[chip].data, mask);
    }

    return 0;
}








int setLED(led_index index, led_colour colour)
{
    return 0;
}




void ledc_confidence(void)
{

}




#if defined(CONFIG_CMD_LEDSET)
int do_set_led(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    return CMD_RET_SUCCESS;
}




U_BOOT_CMD(
    setled,   3,   1,     do_set_led,
    "setled - change colour of single LED",
    "  setled [pwr | nwk | mode | svc | gps] [off | green | red | blue | yellow | cyan | magenta | white]"
);
#endif




#if defined(CONFIG_CMD_LEDC)
#define MAX_STEPS 6




int do_ledc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    return CMD_RET_SUCCESS;
}




U_BOOT_CMD(
    ledc, (7 + MAX_STEPS), 1, do_ledc,
           "Control of multicoloured LEDs - set a single state or step through several",
           "<ledspec> <sequence> [<reps> [<period>]] - type \"ledc ?\" for detailed help"
);
#endif
