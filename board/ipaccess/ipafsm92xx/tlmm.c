#include "tlmm.h"

#include <asm/io.h>

#define TLMMGPIO_BASE (0x94040000)
#define TLMMGPIO_GPIO_OUT_0 (TLMMGPIO_BASE+0x0000)
#define TLMMGPIO_GPIO_OE_0  (TLMMGPIO_BASE+0x0020)
#define TLMMGPIO_GPIO_PAGE  (TLMMGPIO_BASE+0x0040)
#define TLMMGPIO_GPIO_CFG   (TLMMGPIO_BASE+0x0044)
#define TLMMGPIO_GPIO_IN_0  (TLMMGPIO_BASE+0x0048)

#define   GPIO_CFG__DRV_STRENGTH___M (0x000001C0)
#define   GPIO_CFG__DRV_STRENGTH___S (6)
#define   GPIO_CFG__FUNC_SEL___M (0x0000003C)
#define   GPIO_CFG__FUNC_SEL___S (2)
#define   GPIO_CFG__GPIO_PULL___M (0x00000003)
#define   GPIO_CFG__GPIO_PULL___S (0)

static int _set(uint32_t address, int bit);
static int _clear(uint32_t address, int bit);
static int _get(uint32_t address, int bit, int *value);
static int _out(int gpio, int value, uint32_t address);
static int _in(int gpio, int *value, uint32_t address);
static int _alt(int gpio, int value, uint32_t mask, uint32_t shift);
static int _map(int gpio, int *bit, uint32_t * offset);

/******************************************************************************
* Prototype:   int tlmm_out( int gpio , int value )
* Description: Set TLMM value
* Arguments:   gpio - TLMM output to change, 0 - 167
*              value - Value to set, 0 or 1
* Return:       0 - Success
*              -1 - Failure
* Detail:      Set or clear a TLMM value based on 1 or 0 passed to 'value'
******************************************************************************/
int tlmm_out(int gpio, int value)
{
    return _out(gpio, value, TLMMGPIO_GPIO_OUT_0);
}


/******************************************************************************
* Prototype:   int tlmm_oe( int gpio , int enable )
* Description: Enable TLMM output
* Arguments:   gpio - TLMM output to change, 0 - 167
*              enable - Disable( 0 ) or enable ( 1 )
* Return:       0 - Success
*              -1 - Failure
* Detail:      Enable a TLMM output.
******************************************************************************/
int tlmm_oe(int gpio, int enable)
{
    return _out(gpio, enable, TLMMGPIO_GPIO_OE_0);
}


/******************************************************************************
* Prototype:   int tlmm_drv( int gpio , int drv )
* Description: Set drive strength
* Arguments:   gpio - TLMM output to change, 0 - 167
*              drv - Drive strength, 0 - 7
* Return:       0 - Success
*              -1 - Failure
* Detail:      Set drive strength of TLMM output
******************************************************************************/
int tlmm_drv(int gpio, int drv)
{
    if (drv < 0 || drv > 7)
    {
        return -1;
    }
    return _alt(gpio, drv, GPIO_CFG__DRV_STRENGTH___M,
                GPIO_CFG__DRV_STRENGTH___S);
}


/******************************************************************************
* Prototype:   int tlmm_func( int gpio , int func )
* Description: Set TLMM function
* Arguments:   gpio - TLMM output to change, 0 - 167
*              drv - Function, 0 - 15 ( Not all values are valid! )
* Return:       0 - Success
*              -1 - Failure
* Detail:      Set the function of this TLMM pin
******************************************************************************/
int tlmm_func(int gpio, int func)
{
    if (func < 0 || func > 15)
    {
        return -1;
    }
    return _alt(gpio, func, GPIO_CFG__FUNC_SEL___M,
                GPIO_CFG__FUNC_SEL___S);
}


/******************************************************************************
* Prototype:   int tlmm_pull( int gpio , int pull )
* Description: Set TLMM pull configuration
* Arguments:   gpio - TLMM output to change, 0 - 167
*              drv - Function, 0 - 3
* Return:       0 - Success
*              -1 - Failure
* Detail:      Set TLMM for pull up ( 3 ), pull down ( 1 ), no pull ( 0 ) or
*              keeper ( 2 ).
******************************************************************************/
int tlmm_pull(int gpio, int pull)
{
    if (pull < 0 || pull > 3)
    {
        return -1;
    }
    return _alt(gpio, pull, GPIO_CFG__GPIO_PULL___M,
                GPIO_CFG__GPIO_PULL___S);
}


/******************************************************************************
* Prototype:   int tlmm_in( int gpio , int * value )
* Description: Get TLMM value
* Arguments:   gpio - TLMM output to change, 0 - 167
*              *value - Value to read, 0 or 1
* Return:       0 - Success
*              -1 - Failure
* Detail:      Get the current value of a TLMM pin
******************************************************************************/
int tlmm_in(int gpio, int *value)
{
    return _in(gpio, value, TLMMGPIO_GPIO_IN_0);
}


/******************************************************************************
* Prototype:   int _set( uint32_t address , int bit )
* Description: Bit set
* Arguments:   address - Register address
*              bit - Bit to set, 0 - 31
* Return:      0 - Success
*             -1 - Failure
* Detail:      Set 'bit' bit at address 'address'.  This routine is very
*              generic and can be used for non-gpio functions.
******************************************************************************/
static int _set(uint32_t address, int bit)
{
    uint32_t value;
    if (bit < 0 || bit > 32)
    {
        return -1;
    }
    value = readl(address);
    value |= (1 << bit);
    writel(value, address);
    return 0;
}


/******************************************************************************
* Prototype:   int _clear( uint32_t address , int bit )
* Description: Clear set
* Arguments:   address - Register address
*              bit - Bit to clear, 0 - 31
* Return:      0 - Success
*             -1 - Failure
* Detail:      Clear 'bit' bit at address 'address'.  This routine is very
*              generic and can be used for non-gpio functions.
******************************************************************************/
static int _clear(uint32_t address, int bit)
{
    uint32_t value;
    if (bit < 0 || bit > 32)
    {
        return -1;
    }
    value = readl(address);
    value &= ~(1 << bit);
    writel(value, address);
    return 0;
}


/******************************************************************************
* Prototype:   int _get( uint32_t address , int bit , int * value )
* Description: Get bit
* Arguments:   address - Register address
*              bit - Bit to get, 0 - 31
*              *value - Value of bit
* Return:       0 - Success
*              -1 - Failure
* Detail:      Get the 'bit' bit value at address 'address' and
*              return in 'value'.  This routine is very generic and can be
*              used for non-gpio functions.
******************************************************************************/
static int _get(uint32_t address, int bit, int *value)
{
    uint32_t v;
    if (bit < 0 || bit > 32)
    {
        return -1;
    }
    v = readl(address);
    v >>= bit;
    *value = v & 0x00000001;
    return 0;
}


/******************************************************************************
* Prototype:   int _map( int gpio , int * bit , uint32_t * offset )
* Description: Map TLMM to it's register components
* Arguments:   gpio - TLMM pin, 0 - 167
*              *bit - TLMM bit inside register
*              *offset - Register offset
* Return:      0 - Success
*             -1 - Failure
* Detail:      Used to map a TLMM value into it's register componants for easy
*              manipulation of register bits.
******************************************************************************/
static int _map(int gpio, int *bit, uint32_t * offset)
{
    if (gpio >= 0 && gpio <= 31)
    {
        *bit = gpio;
        *offset = 0x00000000;
    }

    else if (gpio >= 32 && gpio <= 63)
    {
        *bit = gpio - 32;
        *offset = 0x00000004;
    }

    else if (gpio >= 64 && gpio <= 95)
    {
        *bit = gpio - 64;
        *offset = 0x00000008;
    }

    else if (gpio >= 96 && gpio <= 127)
    {
        *bit = gpio - 96;
        *offset = 0x0000000C;
    }

    else if (gpio >= 128 && gpio <= 159)
    {
        *bit = gpio - 128;
        *offset = 0x00000010;
    }

    else if (gpio >= 160 && gpio <= 167)
    {
        *bit = gpio - 160;
        *offset = 0x00000014;
    }

    else
    {
        return -1;
    }
    return 0;
}


/******************************************************************************
* Prototype:   int _out( int gpio , int value , uint32_t address )
* Description: Set TLMM value
* Arguments:   gpio - TLMM output to change, 0 - 167
*              value - Value to set, 0 or 1
*              address - Address of TLMM registers
* Return:      0 - Success
*             -1 - Failure 
* Detail:      Set or clear a TLMM value based on 1 or 0 passed to 'value'
******************************************************************************/
int _out(int gpio, int value, uint32_t address)
{
    uint32_t offset;
    int bit;
    int r;
    r = _map(gpio, &bit, &offset);
    if (r != 0)
    {
        return r;
    }
    address += offset;
    if (value & 1)
    {
        return _set(address, bit);
    }

    else
    {
        return _clear(address, bit);
    }
}


/******************************************************************************
* Prototype:   int _in( int gpio , int * value , uint32_t address)
* Description: Get TLMM value
* Arguments:   gpio - TLMM output to change, 0 - 167
*              *value - Value read, 0 or 1
*              address - Address of TLMM registers
* Return:       0 - Success
*              -1 - Failure
* Detail:      Get the current value of a TLMM pin
******************************************************************************/
static int _in(int gpio, int *value, uint32_t address)
{
    uint32_t offset;
    int bit;
    int r;
    r = _map(gpio, &bit, &offset);
    if (r != 0)
    {
        return r;
    }
    address += offset;
    return _get(address, bit, value);
}


/******************************************************************************
* Prototype:   int tlmm_gpio_cfg_get( int gpio , int* value)
* Description: Get GPIO config register
* Arguments:   gpio - TLMM output to change, 0 - 167
*              value - Current Value
* Return:       0 - Success
*              -1 - Failure
* Detail:      Get the TLMM GPIO config register.
******************************************************************************/
int tlmm_gpio_cfg_get(int gpio, int *value)
{
    writel(gpio, TLMMGPIO_GPIO_PAGE);
    *value = readl(TLMMGPIO_GPIO_CFG);
    return 0;
}


/******************************************************************************
* Prototype:   int tlmm_gpio_cfg_set( int gpio , int* value)
* Description: Set GPIO config regiser
* Arguments:   gpio - TLMM output to change, 0 - 167
*              value - Value to set
* Return:       0 - Success
*              -1 - Failure
* Detail:      Set the TLMM config register.
******************************************************************************/
int tlmm_gpio_cfg_set(int gpio, int value)
{
    writel(gpio, TLMMGPIO_GPIO_PAGE);
    writel(value, TLMMGPIO_GPIO_CFG);
    return 0;
}


/******************************************************************************
* Prototype:   int _alt( int gpio , int value , uint32_t mask ,
                         uint32_t shift1 )
* Description: Set alternate TLMM values
* Arguments:   gpio - TLMM output to change, 0 - 167
*              value - Value to set
*              mask - Mask for TLMM registers
*              shift - Shift for TLMM registers
* Return:       0 - Success
*              -1 - Failure
* Detail:      Set the TLMM alternate funtion register.  These values include
*              function, drive and pull.
******************************************************************************/
static int _alt(int gpio, int value, uint32_t mask, uint32_t shift)
{
    uint32_t v;
    writel(gpio, TLMMGPIO_GPIO_PAGE);
    v = readl(TLMMGPIO_GPIO_CFG);
    v &= ~(mask);
    v |= (value << shift) & mask;
    writel(v, TLMMGPIO_GPIO_CFG);
    return 0;
}
