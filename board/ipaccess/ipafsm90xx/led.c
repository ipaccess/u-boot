#include <common.h>

/* For ledc sequences */
#define MAX_STEPS 6    /* Number of allowed states in a cycle */

#if defined(CONFIG_CMD_LEDC)

/*
 *  ledc command for U-Boot
 *
 *   This command provides a flexible way to control a set of multi-coloured LEDs.
 *   The standard U-Boot LED command only handles two-state LEDs, and the existing extensions
 *   for colours are not really suitable for a bank of LEDs. This command is designed to
 *   deal with 3 colours, and is easily extensible to deal with more. It also permits
 *   a fixed sequence to be applied to one or more of the LEDs, though more complex patterns
 *   where not all LEDs behave the same will still need to be programmed manually.
 *
 *   General syntax - see help text below
 */

/* Summary of above as a help text */
static const char detailed_help[] =
"ledc <ledspec> <sequence> [<reps> [<period>]]\n"
"  <ledspec> = <led_id> [<ledspec>]   (Between 1 and 5 LED ids)\n"
"  <led_id> = nwk|svc|gps|all\n"
"  <sequence> = <state> [<sequence>]  (Between 1 and 6 LED states)\n"
"  <state> = off|red|green|yellow\n"
"  1 <= <reps> <= 100                 (number of times to repeat the sequence)\n"
"  100 <= <period> <= 2000            (time in ms between steps)\n\n"
"   Examples:\n"
"    ledc nwk red                                   Turn network led red\n"
"    ledc nwk svc yellow                            Turn network and svc leds yellow\n"
"    ledc all green                                 Turn all leds green\n"
"    ledc pwr svc off                               Turn gps and service leds off (order doesn't matter)\n"
"    ledc svc off red off green off yellow 10 200   Step service led through sequence\n\n";



int do_ledc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
    ledc, (7 + MAX_STEPS), 1, do_ledc,
           "Control of multicoloured LEDs - set a single state or step through several",
           "<ledspec> <sequence> [<reps> [<period>]] - type \"ledc ?\" for detailed help"
);
#endif /* defined CONFIG_CMD_LEDC */
