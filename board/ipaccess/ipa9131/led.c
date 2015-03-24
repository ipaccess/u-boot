#include "led.h"

#include <common.h>




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
