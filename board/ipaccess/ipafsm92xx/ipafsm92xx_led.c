#include <common.h>
#include "tlmm.h"

#define GPIO_LED_RED_1 48
#define GPIO_LED_RED_2 49
#define GPIO_LED_RED_3 51
#define GPIO_LED_RED_4 53

// #define GPIO_LED_GRN_1  connected to power ?
// #define GPIO_LED_GRN_2 47
// #define GPIO_LED_GRN_3 50
// #define GPIO_LED_GRN_4 52

#define GPIO_HI 1
#define GPIO_LO 0

#define GPIO_LED_COUNT 4

struct
{
    int gpio;
    int state;                                                      //Note: bit position must increment.
} gpio_info[GPIO_LED_COUNT] = {{GPIO_LED_RED_1, STATUS_LED_OFF},    //STATUS_LED_BIT  0x1
                               {GPIO_LED_RED_2, STATUS_LED_OFF},    //STATUS_LED_BIT1 0x2
                               {GPIO_LED_RED_3, STATUS_LED_OFF},    //STATUS_LED_BIT2 0x4
                               {GPIO_LED_RED_4, STATUS_LED_OFF}};   //STATUS_LED_BIT3 0x8

static int status_led_state = STATUS_LED_ON;

void __led_set (led_id_t mask, int state)
{
    int i;
    //printf("%s mask:0x%x state:%d\n",__func__,mask,state);

    for (i=0; i<GPIO_LED_COUNT; i++)
    {
        int bit = (0x1 << i);
        // If this LED bit isn't set go around the loop again.
        if (0 == (bit & mask))
            continue;

        if (STATUS_LED_ON == state)
            tlmm_out(gpio_info[i].gpio, GPIO_HI);
        else if (STATUS_LED_OFF == state)
            tlmm_out(gpio_info[i].gpio, GPIO_LO);

        gpio_info[i].state = state;
    }
}

void __led_toggle (led_id_t mask)
{
    int i;
    //printf("%s mask:0x%x\n",__func__,mask);

    for (i=0; i<GPIO_LED_COUNT; i++)
    {
        int bit = (0x1 << i);
        if (0 == (bit & mask))
            continue;

        if (gpio_info[i].state == STATUS_LED_ON)
            __led_set(bit, STATUS_LED_OFF);
        else
            __led_set(bit, STATUS_LED_ON);
    }
}

void __led_init (led_id_t mask, int state)
{
    int i;
    //printf("%s mask:0x%x state:%d\n",__func__,mask,state);

    for(i=0; i<GPIO_LED_COUNT; i++)
    {
        tlmm_oe  (gpio_info[i].gpio, TLMM_OE_ENABLE);
        tlmm_pull(gpio_info[i].gpio, TLMM_PULL_NONE);
        tlmm_drv (gpio_info[i].gpio, TLMM_DRIVE_2MA);

        if (STATUS_LED_ON == gpio_info[i].state)
            tlmm_out(gpio_info[i].gpio, GPIO_HI);
        else if (STATUS_LED_OFF == gpio_info[i].state)
            tlmm_out(gpio_info[i].gpio, GPIO_LO);

    }
}


