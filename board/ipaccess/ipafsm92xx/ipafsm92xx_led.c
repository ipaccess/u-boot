#include <common.h>
#include "tlmm.h"

#define LED_PWR_GREEN_INDEX      0
#define LED_PWR_GREEN_GPIO      48

#define LED_LNK_RED_INDEX        1
#define LED_LNK_RED_GPIO        49

#define LED_LNK_GREEN_INDEX      2
#define LED_LNK_GREEN_GPIO      47

#define LED_SVC_RED_INDEX        3
#define LED_SVC_RED_GPIO        51

#define LED_SVC_GREEN_INDEX      4
#define LED_SVC_GREEN_GPIO      50

#define LED_GPS_RED_INDEX        5
#define LED_GPS_RED_GPIO        53

#define LED_GPS_GREEN_INDEX      6
#define LED_GPS_GREEN_GPIO      52

/* Use this to limit array access below */
#define NUM_GPIO_LINES           7

#define GPIO_HI 1
#define GPIO_LO 0

#define MAX_STEPS 6    /* Number of allowed states in a cycle */

/* Min, mxx and default repetitions of a sequence */
#define MIN_REPS 1
#define MAX_REPS 100
#define DEF_REPS 3

/* Min, mxx and default period between steps in a sequence (ms) */
#define MIN_PERIOD 100   
#define MAX_PERIOD 2000
#define DEF_PERIOD 1000

typedef enum { LED_OFF, LED_GREEN, LED_RED, LED_YELLOW, NUM_COLOURS } led_colour;
typedef enum { LED_PWR, LED_LNK, LED_SVC, LED_GPS, NUM_LEDS } led_index;


/* Map of GPIO address and states */
typedef struct
{
    const int addr;
    int state;
} ledc_gpio;


/* Table which keeps a record of GPIO states - note that the indexes in the
 * comments correspond to the _INDEX values defined above, so DON'T CHANGE!
 */
static ledc_gpio gpio_info[NUM_GPIO_LINES] = 
{
    {LED_PWR_GREEN_GPIO,   GPIO_LO},  /* 0  (red when off) */
    {LED_LNK_RED_GPIO,     GPIO_LO},  /* 1 */
    {LED_LNK_GREEN_GPIO,   GPIO_LO},  /* 2 */
    {LED_SVC_RED_GPIO,     GPIO_LO},  /* 3 */
    {LED_SVC_GREEN_GPIO,   GPIO_LO},  /* 4 */
    {LED_GPS_RED_GPIO,     GPIO_LO},  /* 5 */
    {LED_GPS_GREEN_GPIO,   GPIO_LO}   /* 6 */
};

/* Set maximum GPIO lines associated with one physical LED 
 * Only the Service LED actually has this many 
 */
#define GPIO_LINES_PER_LED 2
#define GPIO_NA -1

/* Look up table for GPIO line addresses for each LED */
typedef int led_lines[GPIO_LINES_PER_LED];

static const led_lines led_addr_table[NUM_LEDS] = 
{
    { LED_PWR_GREEN_INDEX,   GPIO_NA,           }, /* PWR  */
    { LED_LNK_GREEN_INDEX,   LED_LNK_RED_INDEX  }, /* LNK  */
    { LED_SVC_GREEN_INDEX,   LED_SVC_RED_INDEX  }, /* SVC  */
    { LED_GPS_GREEN_INDEX,   LED_GPS_RED_INDEX  }  /* GPS  */
};

/* MASTER LED LOOK-UP TABLE
 *  This table contains bitmaps indicating the GPIO states required to obtain a given colour for any given LED
 *  Bit 0 indicates the first line state and bit 1 the second
 *  If the colour is not available for that LED, the entry is 0xFF.
 */
typedef unsigned char led_settings[NUM_COLOURS];  /* How to get each colour for a given LED */

static const led_settings led_lookup[NUM_LEDS] =
{   /*  Off     Green   Red     Yellow */ 
    {   0xFF,   0x01,   0x02,   0xFF  }, /* PWR */
    {   0x00,   0x01,   0x02,   0x03  }, /* NWK  */
    {   0x00,   0x01,   0x02,   0x03  }, /* SVC  */
    {   0x00,   0x01,   0x02,   0x03  }  /* GPS  */
};


static void set_gpio(int index, int state)
{
    if (gpio_info[index].state != state)
    {
        tlmm_out(gpio_info[index].addr, state);
        gpio_info[index].state = state;
    }
};


static int set_one_led(led_index index, led_colour colour)
{
    unsigned char setting;
    int i;
    
    if ( (index < NUM_LEDS) && (colour < NUM_COLOURS) )
    {
        setting = led_lookup[index][colour];
        if (setting == 0xFF)
        {
            return 1; /* No can do - quietly ignore */
        }
        
        for (i = 0; i < GPIO_LINES_PER_LED; ++i)
        {
            int gpio_index = led_addr_table[index][i];
            if (gpio_index == GPIO_NA)
            {
                break; /* No more lines for this LED */
            }
            
            set_gpio(gpio_index, (setting & 0x1) ? GPIO_HI : GPIO_LO);
            
            setting >>= 1;
        }
    }
    else
    {
        printf("set_one_led out of range error for index %d and colour %d\n", index, colour);
        return 1;
    }
    return 0;
}
                                           
                                           
                                           
/* Old function - sets the LEDs red, except LED 0 */
void __led_set (led_id_t mask, int state)
{
    int i;
    
    for (i=0; i<NUM_LEDS; i++)
    {
        led_colour colour;
        int bit = (0x1 << i);
        
        /* If this LED bit isn't set go around the loop again. */
        if (0 == (bit & mask))
            continue;
        
        /* Qualcomm oddity: led 0 - on means green, for others it's red */
        if (state == STATUS_LED_ON)
        {
            colour = (i == 0) ? LED_GREEN : LED_RED;
        }
        else
        {
            colour = (i == 0) ? LED_RED : LED_OFF;
        }
        set_one_led(i, colour);
    }
}


void __led_toggle (led_id_t mask)
{
    int i;
    
    for (i=0; i<NUM_LEDS; i++)
    {
        led_colour colour;
        int line_index;
        int gpio_index;
        int new_state;
        int bit = (0x1 << i);
        if (0 == (bit & mask))
            continue;
        
        /* Awkward - read current state, green line for pwr, red for others */
        line_index = (i == 0) ? 0 : 1;
        gpio_index = led_addr_table[i][line_index];
        new_state = (1 == gpio_info[gpio_index].state) ? 0 : 1;
        
        /* Qualcomm oddity: led 0 - on means green, for others it's red */
        if (new_state == 1)
        {
            colour = (i == 0) ? LED_GREEN : LED_RED;
        }
        else
        {
            colour = (i == 0) ? LED_RED : LED_OFF;
        }
        set_one_led(i, colour);
    }
}


void __led_init (led_id_t mask, int state)
{
    int i;

    for(i=0; i<NUM_GPIO_LINES; i++)
    {
        tlmm_oe  (gpio_info[i].addr, TLMM_OE_ENABLE);
        tlmm_pull(gpio_info[i].addr, TLMM_PULL_NONE);
        tlmm_drv (gpio_info[i].addr, TLMM_DRIVE_2MA);
        tlmm_out (gpio_info[i].addr, gpio_info[i].state);
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
 *   See usage message below for more details   
 * 
 *   Note re. Qualcomm ipafsm92xx board - led 0 can only show red or green.
 *     If set to "off" or "yellow", it remains unchanged,
 */

/* Summary of above as a help text */
static const char detailed_help[] =
    "ledc <ledspec> <sequence> [<reps> [<period>]]\n"
    "  <ledspec> = <led_id> [<ledspec>]   (Between 1 and 4 LED ids)\n"
    "  <led_id> = pwr|lnk|svc|gps|all\n"
    "  <sequence> = <state> [<sequence>]  (Between 1 and 6 LED states)\n"
    "  <state> = off|red|green|yellow\n"
    "  1 <= <reps> <= 100                 (number of times to repeat the sequence)\n"
    "  100 <= <period> <= 2000            (time in ms between steps)\n\n"
    "   Examples:\n"
    "    ledc pwr red                     Turn power led red\n"
    "    ledc lnk svc yellow              Turn link and service leds yellow\n"
    "    ledc all green                   Turn all leds green\n"
    "    ledc svc gps nwk off             Turn service, gps and link leds off (order doesn't matter)\n"
    "    ledc lnk gps off red off green off yellow 10 200   Step link and gps leds through sequence\n\n";

    
/* Map of mask values for LED Ids */
typedef struct
{
    char* arg;
    int mask;
} led_mask_entry;

static const led_mask_entry led_mask_table[] =
{
    { "pwr",    0x01 },
    { "lnk",    0x02 },
    { "svc",    0x04 },
    { "gps",    0x08 },
    { "all",    0x0F },
    { "",       -1  }
};

static const char* colour_strings[NUM_COLOURS] =
{ "off", "green", "red", "yellow" };

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
            set_one_led((led_index)i, colour);
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
    
    /* All OK, go through the sequence the requisite number of times */
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