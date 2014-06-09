/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file gpio.h
* \brief Definitions for the picoxcell gpio block.
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

#ifndef __PICOXCELL_GPIO_H__
#define __PICOXCELL_GPIO_H__

/* Constants --------------------------------------------------------------- */

/*****************************************************************************/
/* Register Offset Addresses                                                 */
/*****************************************************************************/

#define GPIO_SW_PORT_A_DR_REG_OFFSET        (0x00)
#define GPIO_SW_PORT_A_DDR_REG_OFFSET	    (0x04)
#define GPIO_SW_PORT_A_CTL_REG_OFFSET       (0x08)
#define GPIO_SW_PORT_B_DR_REG_OFFSET        (0x0C)
#define GPIO_SW_PORT_B_DDR_REG_OFFSET	    (0x10)
#define GPIO_SW_PORT_B_CTL_REG_OFFSET       (0x14)
#define GPIO_SW_PORT_C_DR_REG_OFFSET        (0x18)
#define GPIO_SW_PORT_C_DDR_REG_OFFSET	    (0x1C)
#define GPIO_SW_PORT_C_CTL_REG_OFFSET       (0x20)
#define GPIO_SW_PORT_D_DR_REG_OFFSET        (0x24)
#define GPIO_SW_PORT_D_DDR_REG_OFFSET	    (0x28)
#define GPIO_SW_PORT_D_CTL_REG_OFFSET       (0x2C)

/* global GPIO registers */
#define GPIO_INT_EN_REG_OFFSET              (0x30)
#define GPIO_INT_MASK_REG_OFFSET            (0x34)
#define GPIO_INT_TYPE_LEVEL_REG_OFFSET 	    (0x38)
#define GPIO_INT_POLARITY_REG_OFFSET 	    (0x3c)

#define GPIO_INT_STATUS_REG_OFFSET	    (0x40)
#define GPIO_RAW_INT_STATUS_REG_OFFSET	    (0x44)

/* no debounce */
#define GPIO_PORT_A_EOI_REG_OFFSET          (0x4c)
#define GPIO_EXT_PORT_A_REG_OFFSET          (0x50)
#define GPIO_EXT_PORT_B_REG_OFFSET          (0x54)
#define GPIO_EXT_PORT_C_REG_OFFSET          (0x58)
#define GPIO_EXT_PORT_D_REG_OFFSET          (0x5C)

#define GPIO_LS_SYNC_REG_OFFSET		    (0x60)
#define GPIO_ID_CODE_REG_OFFSET		    (0x64)
#define GPIO_RESERVED_REG_OFFSET            (0x68)
#define GPIO_COMP_VERSION_REG_OFFSET	    (0x6c)

/* Macros ------------------------------------------------------------------ */

/* Useful bit definitions */
#define GPIO_BIT_7  (0x80)
#define GPIO_BIT_6  (0x40)
#define GPIO_BIT_5  (0x20)
#define GPIO_BIT_4  (0x10)
#define GPIO_BIT_3  (0x08)
#define GPIO_BIT_2  (0x04)
#define GPIO_BIT_1  (0x02)
#define GPIO_BIT_0  (0x01)

#endif /* __PICOXCELL_GPIO_H__ */
