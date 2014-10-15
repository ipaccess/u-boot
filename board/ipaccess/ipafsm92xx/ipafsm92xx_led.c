#include <common.h>
#include "tlmm.h"

#define GPIO_LED_INVAL 0  /* Special value, no line corresponds to this state */
#define GPIO_LED_RED_2 49
#define GPIO_LED_RED_3 51
#define GPIO_LED_RED_4 53

#define GPIO_LED_GRN_1 48
#define GPIO_LED_GRN_2 47
#define GPIO_LED_GRN_3 50
#define GPIO_LED_GRN_4 52

#define GPIO_HI 1
#define GPIO_LO 0

#define GPIO_LED_COUNT 4
#define MAX_STEPS 6    /* Number of allowed states in a cycle */

/* Min, mxx and default repetitions of a sequence */
#define MIN_REPS 1
#define MAX_REPS 100
#define DEF_REPS 3

/* Min, mxx and default period between steps in a sequence (ms) */
#define MIN_PERIOD 100   
#define MAX_PERIOD 2000
#define DEF_PERIOD 1000

/* Map of GPIO address and states */
typedef struct
{
    const int addr;
    int state;
} ledc_gpio;

typedef struct
{
    ledc_gpio red;    /* GPIO line to turn red */
    ledc_gpio green;  /* GPIO line to turn green */
} ledc_gpio_map;

ledc_gpio_map gpio_map[GPIO_LED_COUNT] = { { {GPIO_LED_INVAL, GPIO_LO}, {GPIO_LED_GRN_1, GPIO_LO} },
                                           { {GPIO_LED_RED_2, GPIO_LO}, {GPIO_LED_GRN_2, GPIO_LO} },
                                           { {GPIO_LED_RED_3, GPIO_LO}, {GPIO_LED_GRN_3, GPIO_LO} },
                                           { {GPIO_LED_RED_4, GPIO_LO}, {GPIO_LED_GRN_4, GPIO_LO} } };

                                           
/* Old function - sets the LEDs red, except LED 0 */
void __led_set (led_id_t mask, int state)
{
    int i;
    ledc_gpio *entry;
    int gpio_state;
    
    /* printf("%s mask:0x%x state:%d\n",__func__,mask,state); */
    
    for (i=0; i<GPIO_LED_COUNT; i++)
    {
        int bit = (0x1 << i);
        /* If this LED bit isn't set go around the loop again. */
        if (0 == (bit & mask))
            continue;
        
        /* Qualcomm oddity: led 0 - on means green, for others it's red */
        entry = (i == 0) ? &(gpio_map[i].green) : &(gpio_map[i].red);
        gpio_state = (STATUS_LED_ON == state) ? GPIO_HI : GPIO_LO;
        tlmm_out(entry->addr, gpio_state);
        entry->state = gpio_state;
    }
}


void __led_toggle (led_id_t mask)
{
    int i;
    ledc_gpio *entry;
    int gpio_state;
    
    //printf("%s mask:0x%x state:%d\n",__func__,mask);
    
    for (i=0; i<GPIO_LED_COUNT; i++)
    {
        int bit = (0x1 << i);
        if (0 == (bit & mask))
            continue;
        
        /* Qualcomm oddity: led 0 - on means green, for others it's red */
        entry = (i == 0) ? &(gpio_map[i].green) : &(gpio_map[i].red);        
        gpio_state = (entry->state == GPIO_LO) ? GPIO_HI : GPIO_LO;
        tlmm_out(entry->addr, gpio_state);
        entry->state = gpio_state;
    }
}


void __led_init (led_id_t mask, int state)
{
    int i;
    //printf("%s mask:0x%x state:%d\n",__func__,mask,state);

    for(i=0; i<GPIO_LED_COUNT; i++)
    {
        if (gpio_map[i].red.addr != GPIO_LED_INVAL)
        {
            tlmm_oe  (gpio_map[i].red.addr, TLMM_OE_ENABLE);
            tlmm_pull(gpio_map[i].red.addr, TLMM_PULL_NONE);
            tlmm_drv (gpio_map[i].red.addr, TLMM_DRIVE_2MA);
            tlmm_out (gpio_map[i].red.addr, gpio_map[i].red.state);
        }
        
        if (gpio_map[i].green.addr != GPIO_LED_INVAL)
        {
            tlmm_oe  (gpio_map[i].green.addr, TLMM_OE_ENABLE);
            tlmm_pull(gpio_map[i].green.addr, TLMM_PULL_NONE);
            tlmm_drv (gpio_map[i].green.addr, TLMM_DRIVE_2MA);
            tlmm_out (gpio_map[i].green.addr, gpio_map[i].green.state);
        }
    }
}


#if defined CONFIG_CMD_LEDC

/*
 *  ledc command for U-Boot
 * 
 *   This command provides a flexible way to control a set of multi-coloured LEDs.
 *   The standard U-Boot LED command only handles two-state LEDs, and the existing extensions
 *   for colours are not really suitable for a bank of LEDs. This command is designed to
 *   deal with 3 colours, and is easily extensible to deal with more. It also permits
 *   a fixed sequence sto be applied to one or more of the LEDs, though more complex patterns
 *   where not all LEDs behave the same will still need to be programmed manually.
 * 
 *   Note that this command still relies on the __led_init function above associated with the
 *   original led command. The led command still functions as far as it can.
 * 
 *   General syntax:   
 * 
 *   ledc <ledspec> <sequence> [<reps> [<period>]]
 *      <ledspec> = <led_id> [<ledspec>]   (Between 1 and 4 LED ids)
 *      <led_id> = 0|1|2|3|all
 *      <sequence> = <state> [<sequence>]  (Between 1 and 6 LED states)
 *      <state> = off|red|green|yellow
 *      1 <= <reps> <= 100                 (number of times to repeat the sequence)
 *      100 <= <period> <= 2000            (time in ms between steps)
 * 
 *   Examples:
 * 
 *    ledc 1 red                                     Turn led 1 red
 *    ledc 2 3 yellow                                Turn leds 2 and 3 yellow
 *    ledc all green                                 Turn all leds green
 *    ledc 2 3 1 off                                 Turn leds 1 2 3 off (order doesn't matter)
 *    ledc 1 3 off red off green off yellow 10 200   Step leds 1 and 3 through sequence
 *                                                     10 times, 200ms between steps
 * 
 *   After running a sequence, leds are left in the final state of the sequence
 * 
 *   Note re. Qualcomm ipafsm92xx board - led 0 can only show red or green.
 *     If set to "off", it shows red,
 *     if set to "yellow" it shows green
 */

/* Summary of above as a help text */
static const char detailed_help[] =
    "ledc <ledspec> <sequence> [<reps> [<period>]]\n"
    "  <ledspec> = <led_id> [<ledspec>]   (Between 1 and 4 LED ids)\n"
    "  <led_id> = 0|1|2|3|all\n"
    "  <sequence> = <state> [<sequence>]  (Between 1 and 6 LED states)\n"
    "  <state> = off|red|green|yellow\n"
    "  1 <= <reps> <= 100                 (number of times to repeat the sequence)\n"
    "  100 <= <period> <= 2000            (time in ms between steps)\n\n"
    "   Examples:\n"
    "    ledc 1 red                                     Turn led 1 red\n"
    "    ledc 2 3 yellow                                Turn leds 2 and 3 yellow\n"
    "    ledc all green                                 Turn all leds green\n"
    "    ledc 2 3 1 off                                 Turn leds 1 2 3 off (order doesn't matter)\n"
    "    ledc 1 3 off red off green off yellow 10 200   Step leds 1 and 3 through sequence\n\n";

    
/* Map of mask values for LED Ids */
typedef struct
{
    char* arg;
    int mask;
} led_mask_entry;

static const led_mask_entry led_mask_table[] =
{
    { "0",      0x1 },
    { "1",      0x2 },
    { "2",      0x4 },
    { "3",      0x8 },
    { "all",    0xF },
    { "",       -1  }
};

/* Map of GPIO states for LED colours */
typedef struct
{
    char* colour;
    int rstate;
    int gstate;
} ledc_colour_entry;

static const ledc_colour_entry ledc_colour_table[] =
{
    { "off",     GPIO_LO, GPIO_LO }, /* Both low -> off */
    { "red",     GPIO_HI, GPIO_LO }, /* Red high, green low -> red */
    { "green",   GPIO_LO, GPIO_HI }, /* Red low, green high -> green */
    { "yellow",  GPIO_HI, GPIO_HI }, /* Both high -> yellow(ish) */
    { "",        -1,      -1      }
};

/* Table for cyling round colours */
int led_sequence[MAX_STEPS];         


/* Set a GPIO line if it's not already in the required state */
void gpio_ledc_set(ledc_gpio* gpio_entry, int new_state)
{
    if ( (gpio_entry->state != new_state)
      && (gpio_entry->addr != GPIO_LED_INVAL) )
    {
        tlmm_out(gpio_entry->addr, new_state);
        gpio_entry->state = new_state;
    }
}

/* Apply specified states to all LEDs specified by the mask */
void ledc_set_leds(int mask, int colour)
{
    int i, im;
    
    for (i = 0, im = 1; i < GPIO_LED_COUNT; i++, im <<= 1)
    {
        if ( (mask & im) != 0)
        {
            gpio_ledc_set(&(gpio_map[i].red),   ledc_colour_table[colour].rstate);
            gpio_ledc_set(&(gpio_map[i].green), ledc_colour_table[colour].gstate);
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
    if ( (argc < 3) || (argc > (7 + MAX_STEPS)) )
    {
        printf("Incorrect number of arguments\n");
        return CMD_RET_USAGE;
    }
    
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
    while ((ledc_colour_table[i].rstate >= 0) && (argi < argc) && (steps < MAX_STEPS))
    {
        if (strcmp(ledc_colour_table[i].colour, argv[argi]) == 0)
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