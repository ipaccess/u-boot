#include <common.h>
#include <asm/io.h>
#include <cli.h>
#include "iomap.h"
#include "led.h"
#include "debug.h"
#include "gpio.h"

/* These LED control procedures are a little more convoluted than actually required for
 * the S60 board, as they are based on more generalised functions for LEDs which
 * support more than 3 colours, but for which not all LEDs offer the same colours.
 * NWK, SVC and PWR all have red and green options, and MODE LED has Green and yellow options, 
   the last being formed * (a little unconvincingly) when both red and green segments are lit together.
 */

/* Values for GPIO lines */
#define GPIO_HI 1
#define GPIO_LO 0

/*
 *  LED CONFIGURATION LOGIC
 *  The INDEX values are used for a quick look-up into the gpio_info table below
 *  DO NOT CHANGE THESE!
 */

/* LED_NWK_*/
#define LED_NWK_GREEN_INDEX         0
#define LED_NWK_GREEN_GPIO          20 
#define LED_NWK_RED_INDEX           1
#define LED_NWK_RED_GPIO            21

/* LED_SVC */
#define LED_SVC_GREEN_INDEX         2
#define LED_SVC_GREEN_GPIO          22
#define LED_SVC_RED_INDEX           3
#define LED_SVC_RED_GPIO            23 
#define LED_SVC_RED_GPIO_509        24 /*India gate*/

/*LED POWER*/
#define LED_PWR_GREEN_INDEX         4
#define LED_PWR_GREEN_GPIO          18
#define LED_PWR_RED_INDEX           5
#define LED_PWR_RED_GPIO            19

/* Use this to limit array access below */
#define NUM_GPIO_LINES              6

/* For ledc sequences */
#define MAX_STEPS 6    /* Number of allowed states in a cycle */

/* Min, mxx and default repetitions of a sequence */
#define MIN_REPS 1
#define MAX_REPS 100
#define DEF_REPS 3

/* Min, max and default period between steps in a sequence (ms) */
#define MIN_PERIOD 100
#define MAX_PERIOD 2000
#define DEF_PERIOD 1000


/* Map of GPIO address and states */
typedef struct
{
    int addr;
    int state;
} ledc_gpio;

/* Table which keeps a record of GPIO states - note that the indexes in the
 * comments correspond to the _INDEX values defined above, so DON'T CHANGE!
 */
static ledc_gpio gpio_info[NUM_GPIO_LINES] =
{
    {LED_NWK_GREEN_GPIO,    GPIO_LO},    /* 0 */
    {LED_NWK_RED_GPIO,      GPIO_LO},    /* 1 */
    {LED_SVC_GREEN_GPIO,    GPIO_LO},    /* 2 */
    {LED_SVC_RED_GPIO,      GPIO_LO},    /* 3 */
    {LED_PWR_GREEN_GPIO,    GPIO_LO},    /* 4 */
    {LED_PWR_RED_GPIO,      GPIO_LO},    /* 5 */
};

/* Set maximum GPIO lines associated with one physical LED */
#define GPIO_LINES_PER_LED 2

/* Look up table for GPIO line indexes for each LED */
typedef int led_lines[GPIO_LINES_PER_LED];

static const led_lines led_addr_table[NUM_LEDS] =
{
    { LED_NWK_GREEN_INDEX,   LED_NWK_RED_INDEX }, /* NWK  */
    { LED_SVC_GREEN_INDEX,   LED_SVC_RED_INDEX }, /* SVC  */
    { LED_PWR_GREEN_INDEX,   LED_PWR_RED_INDEX }, /* PWR  */
};

/* Acceptable strings for LED colours
 * It is vital that these states appear in the order shown as the value is
 * used as a bit masek to determine which LED segments to drive to obtain the
 * required colour, hence 0->off, 1->green, 2->red, 3->red+green=yellow
 */
// for now: green-> white red-> orange yellow -> pink
static const char* colour_strings[NUM_COLOURS] =
{ "off", "green", "red", "yellow" };

static int led_init=0;


static void set_gpio(int index, int state)
{
    if (gpio_info[index].state != state)
    {
        simple_set_gpio(gpio_info[index].addr, state);
        gpio_info[index].state = state;
    }
}


int setLED(led_index index, led_colour colour)
{
    unsigned char setting;
    int i;

    if ( (index < NUM_LEDS) && (colour < NUM_COLOURS) )
    {
        /* This is a short cut, saving a table lookup, because all LEDs in ipas60
         * have the same colours and work the same way - it relies on:
         * Off = 0; Green = 1; Red = 2; Yellow = 3
         * - so don't you dare change the led_colour enum!
         */
        setting = (unsigned char)colour;

        for (i = 0; i < GPIO_LINES_PER_LED; ++i)
        {
            int gpio_index = led_addr_table[index][i];
            set_gpio(gpio_index, (setting & 0x1) ? GPIO_HI : GPIO_LO);

            setting >>= 1;
        }
    }
    else
    {
        printf("setLED out of range error for index %d and colour %d\n", index, colour);
        return 1;
    }
    return 0;
}

/*Initialise LEDs on the basis product variants*/
void initLED()
{
    /*already initialised do nothing*/
    if (led_init)
        return;

    char *part_num = getenv("board_variant_part");
    if ( part_num && (0 == strncmp(part_num, "509", 3)))
    {
        gpio_info[LED_SVC_RED_INDEX].addr = LED_SVC_RED_GPIO_509;
    }

    led_init = 1;

}

#if defined(CONFIG_CMD_LEDSET)
/* Acceptable strings for LED identification (used in setled only)
*/
static const char* led_strings[NUM_LEDS] =
{ "nwk", "svc", "pwr"};

int do_set_led(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int i;
    int result = 1;
    led_index  index;
    led_colour colour;

    if (argc == 3)
    {
        for (i = 0; i < NUM_LEDS; i++)
        {
            if (strcmp(argv[1], led_strings[i]) == 0)
            {
                index = (led_index)i;
                break;
            }
        }

        if (i < NUM_LEDS)
        {
            for (i = 0; i < NUM_COLOURS; i++)
            {
                if (strcmp(argv[2], colour_strings[i]) == 0)
                {
                    colour = (led_colour)i;
                    result = 0;
                    break;
                }
            }
        }

        if (result == 0)
        {
            if (setLED(index, colour))
            {
                printf("\"%s\" is not a valid setting for LED \"%s\"\n", argv[2], argv[1]);
            }
        }
    }
    if (result)
    {
        return CMD_RET_USAGE;
    }

    return CMD_RET_SUCCESS;
}
U_BOOT_CMD(
setled, 3, 1, do_set_led,
"setled - change colour of single LED",
" setled [nwk | svc | gps] [off | green | red | yellow]"
);
#endif

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


/* Map of mask values for LED Ids */
typedef struct
{
    char* arg;
    int mask;
} led_mask_entry;

static const led_mask_entry led_mask_table[] =
{
    { "nwk",    0x01 },
    { "svc",    0x02 },
    { "pwr",    0x04 },
    { "all",    0x07 },
    { "",       -1  }
};

/* Table for cycling round colours */
led_colour led_sequence[MAX_STEPS];


/* Apply specified states to all LEDs specified by the mask */
void ledc_set_leds(int mask, led_colour colour)
{
    int i, im;

    for (i = 0, im = 1; i < NUM_LEDS; i++, im <<= 1)
    {
        if ( (mask & im) != 0)
        {
            setLED((led_index)i, colour);
        }
    }
}
int do_ledc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int i;
    int argi;
    int mask;
    int steps;
    int reps = DEF_REPS;
    int period_us = (DEF_PERIOD * 1000);
    /* Check first for a detailed help request */
    if ( (argc > 0) && (strcmp("?", argv[1]) == 0) )
    {
        printf(detailed_help);
        return CMD_RET_SUCCESS;
    }
    /* Validate arguments */
    if ( (argc < 3) || (argc > (8 + MAX_STEPS)) )
    {
        printf("Incorrect number of arguments\n");
        return CMD_RET_USAGE;
    }

    initLED();

    /* Find mask for specified LED(s) - step through args until we reach a non-led value */
    i = 0; argi = 1; mask = 0;
    while ((led_mask_table[i].mask >= 0) && (argi < argc))
    {
        if (strcmp(led_mask_table[i].arg, argv[argi]) == 0)
        {
            mask |= led_mask_table[i].mask;
            ++argi; i = 0; /* Next arg... */
            continue;
        }
        ++i;
    }

    if (mask <= 0)
    {
        printf("No valid LED id specified\n");
        return CMD_RET_USAGE;
    }

    /* Step through next args until we run out or hit a non-colour */
    i = 0; steps = 0;
    while ( (i < NUM_COLOURS) && (argi < argc) && (steps < MAX_STEPS))
    {
        if (strcmp(colour_strings[i], argv[argi]) == 0)
        {
            led_sequence[steps] = i; /* Each sequence step contains the index of the colour table entry */
            i = 0; ++steps; ++argi;
            continue;
        }
        ++i;
    }

    /* If no colours detected, oops */
    if (steps == 0)
    {
        printf("Invalid LED id, or no valid LED colour/state specified\n");
        return CMD_RET_USAGE;
    }

    /* If we only have one colour, just set it - no sequence */
    if (steps == 1)
    {
        ledc_set_leds(mask, led_sequence[0]);
        return CMD_RET_SUCCESS;
    }

    /* Read reps if present */
    if (argi < argc)
    {
	int new_reps = simple_strtoul(argv[argi], NULL, 0);
        if ( (new_reps < MIN_REPS) || ( new_reps > MAX_REPS) )
        {
            printf("Number of sequence repetitions must be >= %d and <= %d\n", MIN_REPS, MAX_REPS);
            return CMD_RET_USAGE;
        }
        reps = new_reps;
        ++argi;
    }

    /* Read period if present */
    if (argi < argc)
    {
        int new_period_ms = simple_strtoul(argv[argi], NULL, 0);
        if ( (new_period_ms < MIN_PERIOD) || (new_period_ms > MAX_PERIOD) )
        {
            printf("Step period must be >= %dms and <= %dms\n", MIN_PERIOD, MAX_PERIOD);
            return CMD_RET_USAGE;
        }
        period_us = 1000 * new_period_ms;
        ++argi;
    }

    /* Should be no more args! */
    if (argi < argc)
    {
        printf("Extra arguments found\n");
        return CMD_RET_USAGE;
    }

    /* Now go through the sequence the requisite number of times */
    while (reps > 0)
    {
        for (i=0; i < steps; i++)
        {
            ledc_set_leds(mask, led_sequence[i]);
            udelay(period_us);
        }
        --reps;
    }


    return CMD_RET_SUCCESS;
}


U_BOOT_CMD(
    ledc, (7 + MAX_STEPS), 1, do_ledc,
           "Control of multicoloured LEDs - set a single state or step through several",
           "<ledspec> <sequence> [<reps> [<period>]] - type \"ledc ?\" for detailed help"
);
#endif /* defined CONFIG_CMD_LEDC */
