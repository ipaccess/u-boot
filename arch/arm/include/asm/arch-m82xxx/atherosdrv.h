/*
 * Copyright(c) 2007-2014 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * The full GNU General Public License is included in this distribution 
 * in the file called LICENSE.GPL.
 *
 * Contact Information:
 * Intel Corporation
 */

//#include "sysdata.h"
#include <asm/arch/mdio_bitbang.h>

#define T2200_EVM_ATHEROS_SGMII_PHY_ADDRESS 6
#define T3300_EVM_ATHEROS_SGMII_PHY_ADDRESS 4

#ifdef ATHEROS_SWITCH_TEST_ENABLED
#define ATHEROS_SGMII_PHY_ADDRESS 6 // T2K EVMBOARD ATHEROS SGMII PHY ADDRESS (DIFFERENT FOR OTHER BOARDS)
#else
#define ATHEROS_SGMII_PHY_ADDRESS       4 // T2K CHECKOUT AND T3300 EVM BOARD ATHEROS SGMII PHY ADDRESS (DIFFERENT FOR OTHER BOARDS)
#endif
#define ATHEROS_RGMII_PHY_ADDRESS 5 // T2K CHECKOUT  BOARD ATHEROS RGMII PHY ADDRESS (DIFFERENT FOR OTHER BOARDS)
#define ATHEROS_SGMII_PHY_ADDRESS_SLAVE 6 // T3K EVM SLAVE ATHEROS SGMII PHY ADDRESS (DIFFERENT FOR OTHER BOARDS)
#define ATHEROS_RGMII_PHY_ADDRESS_SLAVE 7 // T3K EVM SLAVE ATHEROS RGMII PHY ADDRESS (DIFFERENT FOR OTHER BOARDS)

#define ATHEROS_RGMII_GTX_DLY_MASK         (3 << 5)
#define ATHEROS_RGMII_GTX_DLY_VAL_250_PS   (0 << 5)
#define ATHEROS_RGMII_GTX_DLY_VAL_1300_PS  (1 << 5)
#define ATHEROS_RGMII_GTX_DLY_VAL_2400_PS  (2 << 5)
#define ATHEROS_RGMII_GTX_DLY_VAL_3400_PS  (3 << 5)

#define ATHEROS_CONFIG_REG_TYPE_MASK                    (0xF << 0)
#define ATHEROS_CONFIG_TYPE_BASET_RGMII                 0       /* 1000 BASE-T, RGMII */
#define ATHEROS_CONFIG_TYPE_BASET_SGMII                 1       /* 1000 BASE-T, SGMII */
#define ATHEROS_CONFIG_TYPE_BX1000_RGMII_50             2       /* 1000 BASE-X, RGMII, 50 ohms */
#define ATHEROS_CONFIG_TYPE_BX1000_SGMII_75             3       /* 1000 BASE-X, SGMII, 75 ohms */
#define ATHEROS_CONFIG_TYPE_BX1000_CONV_1000_BASET_50   4       /* Converter mode between 1000 BASE-X  and 1000 BASE-T media, 50 ohm */
#define ATHEROS_CONFIG_TYPE_BX1000_CONV_1000_BASET_75   5       /* Converter mode between 1000 BASE-X  and 1000 BASE-T media, 75 ohm */
#define ATHEROS_CONFIG_TYPE_FX100_RGMII_50              6       /* 100 BASE-FX, RGMII 50 ohm */
#define ATHEROS_CONFIG_TYPE_FX100_CONV_100BASET_50      7       /* Converter mode between  100 BASE-FX and 1000 BASE-T media, 50 ohm */
#define ATHEROS_CONFIG_TYPE_RGMII_AUTO_COPPER_FIBER    11       /* RGMII, copper fiber auto-detection */
#define ATHEROS_CONFIG_TYPE_FX100_RGMII_75             14       /* 100 BASE-FX, RGMII mode, 75 ohm */
#define ATHEROS_CONFIG_TYPE_FX100_CONV_100_BASET_75    15       /* Converter mode between  100 BASE-FX and  100 BASE-T media, 75 ohm */

#define ATHEROS_CONFIG_TYPE_RG_AUTO_MDET        11


//#define T2K_EVM_RGMII_GTX_DLY_VAL   ATHEROS_RGMII_GTX_DLY_VAL_250_PS // Seeing abuot 45% CRC errors and gaps where no frames are seen at all
//#define T2K_EVM_RGMII_GTX_DLY_VAL   ATHEROS_RGMII_GTX_DLY_VAL_1300_PS // Still seeing CRC errors on board 9, but not many, about .0016 percent (still to high for actual system)
//#define T2K_EVM_RGMII_GTX_DLY_VAL   ATHEROS_RGMII_GTX_DLY_VAL_2400_PS // Still seeing CRC errors on board 9, about 12% on small packets
//#define T2K_EVM_RGMII_GTX_DLY_VAL   ATHEROS_RGMII_GTX_DLY_VAL_3400_PS // Seeing very few good frames (over 99% are CRC error at this delay value)




u32 SetRgmiiAtherosPhy(struct mii_bus * bus, u32 phy_addr, u32 delay_value);
u32 SetSgmiiAtherosPhy(struct mii_bus * bus, u32 phy_addr);


