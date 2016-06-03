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

#include <common.h>
#include <asm/arch/atherosdrv.h>

void WriteAtherosDebugRegister(struct mii_bus * bus, u16 phy, u16 DebugRegister, u16 Value)
{
    //
    // Setup Debug register to target value
    //
    bus->write(bus, phy, 0x1D, DebugRegister);
    //
    // Debug register selected, write value to 0x1E to put value into that
    // debug register.
    //
    bus->write(bus, phy, 0x1E, Value);
}

u16 ReadAtherosDebugRegister(struct mii_bus * bus, u16 phy, u16 DebugRegister)
{
    //
    // Setup Debug register to target value
    //
    bus->write(bus,phy,0x1D,DebugRegister);
    //
    // Debug register selected, read value from 0x1E to get value from that
    // debug register.
    //
    return(bus->read(bus,phy,0x1E));
}

void DumpPhyRegisters(struct mii_bus * bus, u32 phy)
{
    u32 reg;
    u16 val;

	printf("\nPHY 0x%2.2X:\n",phy);
	for (reg=0; reg<=0xF; reg++)
	{
		// Read register
		val = bus->read(bus, phy, reg);
		// Print it
		printf(" 0x%4.4X",val);
		//mdiobb_write(bus,phy,reg,val);
	}
	printf("\n");
	for (; reg<=0x1F; reg++)
	{
		// Read register
		val = bus->read(bus, phy, reg);
		// Print it
		printf(" 0x%4.4X",val);
	}
    printf("\n");
}

void DumpPhyDebugRegisters(struct mii_bus * bus, u32 phy)
{
    u32 reg;
    u16 val;

	printf("\nPHY 0x%2.2X DEBUG:\n",phy);
	for (reg=0; reg<=0xF; reg++)
	{
		// Read register
		val = ReadAtherosDebugRegister(bus, phy, reg);
		// Print it
		printf(" 0x%4.4X",val);
	}
	printf("\n");
	for (; reg<=0x1F; reg++)
	{
		// Read register
		val = ReadAtherosDebugRegister(bus, phy, reg);
		// Print it
		printf(" 0x%4.4X",val);
	}
    printf("\n");
}


u32 SetRgmiiAtherosPhy(struct mii_bus * bus, u32 phy_addr, u32 delay_value)
{
    u16 val;
    u32 phy = phy_addr;
    //
    // For RGMII mode, need to change RGMII timing to delay TX (delay for RX defaulted OK)
    //
    // First set TX Value to default (4 values possible, see defines)
    //
    val = ReadAtherosDebugRegister(bus, phy_addr, 0xB);
    if((val & ATHEROS_RGMII_GTX_DLY_MASK) != delay_value)
    {
 //       printf("Changing Atheros RGMII delay from 0x%X to 0x%X\n", val & ATHEROS_RGMII_GTX_DLY_MASK, delay_value);

        val &= ~ATHEROS_RGMII_GTX_DLY_MASK;  // Clear bits
        val |= (delay_value & ATHEROS_RGMII_GTX_DLY_MASK);
        WriteAtherosDebugRegister(bus, phy_addr, 0xB, val);

        val = ReadAtherosDebugRegister(bus, phy, 5);    // Get current value from debug register 5
        val |= (1<<8);                                  // TX clock delay is enabled by setting bit 8
        WriteAtherosDebugRegister(bus, phy, 5, val);    // Write it back

        // Reset the PHY using standard MII register 0
        val = bus->read(bus, phy_addr, 0);
        val |= 0x8000;                                  // Set bit 15 high (self clearing bit)
        bus->write(bus, phy_addr, 0, val);

        // wait for link
        //do{
        //    val = bus->read(bus, phy, 0);
        //}while( (val & (1 << 5)) == 0);

        return 1;
    }
    return 0;
}

u32 SetSgmiiAtherosPhy(struct mii_bus * bus, u32 phy_addr)
{
    u16 val;
    u32 phy = phy_addr;
    u32 ResetNeeded = 0;
    //
    // For SGMII mode, need to change mode register to SGMII (tests are showing this as not being set for some reason)
    //
    // Get Atheros Chip configure regsiter (register 0x1F)
    // Mask off current type and set to SGMII copper mode
    //
    val = bus->read(bus, phy, 0x1F);
    //printf("SGMII PHY 0x%02X read  0x1F: 0x%04X\n",phy,val);  // TEMP PRINT
    //
    // Test if already running SGMII, if so, nothing to do
    //
    if ((val & ATHEROS_CONFIG_REG_TYPE_MASK) == ATHEROS_CONFIG_TYPE_BASET_SGMII)
    {
    //    printf("SGMII PHY already running SGMII, leaving alone...\n");
        return 0;
    }
    else
    {
      //  printf("SGMII PHY (%d) is in wrong mode 0x%04X. Setting SGMII\n", phy, val);  // TEMP PRINT
        val &= ~(ATHEROS_CONFIG_REG_TYPE_MASK);   // (0xF << 0)
        val |= ATHEROS_CONFIG_TYPE_BASET_SGMII;   //  1
        bus->write(bus, phy, 0x1F, val);

        //val = bus->read(bus, phy, 0x1F);// TEMP READ BACK
        //printf("SGMII PHY 0x%02X read  0x1F: 0x%04X\n",phy,val);  // TEMP PRINT
        ResetNeeded = 1;
    }
#if 0
    //
    // FOR TEST, SHUTDOWN ENERGY EFFICIENT ETHERNET FEATURE
    // TO HOPEFULLY GET LINK MORE STABLE FOR IXIA TESTING
    //
    // Get current value of extended MDIO register 0x805D
    // and clear energy effecient ethernet bit
    //
    mdiobb_write(bus, phy, 0x0D, 0x0003); // Function=address, set device address to Request MMD3 exteneded GPIO data
    mdiobb_write(bus, phy, 0x0E, 0x805D); // Address with energy effecient Ethernet option
    mdiobb_write(bus, phy, 0x0D, 0x4003); // Function=data, keep the device address the same for MMD3 and previous set register
    val = mdiobb_read(bus, phy, 0x0E);    // Get data from extended register
    val &= ~(1<<8);                       // Turn off bit 8 to disable energy efficient ethernet
    mdiobb_write(bus, phy, 0x0E, val);    // Write back data with bit disabled
    //ResetNeeded = 1;  // Request reset (lpi_en bit is retained in software reset)
#endif
    if (ResetNeeded)
    {
        //
        // Reset the PHY using standard MII register 0
        // Set bit 15 high (self clearing bit)
        //
        val = bus->read(bus, phy, 0);
        val |= 0x8000; // Set reset bit
        bus->write(bus, phy, 0, val);
        return 1;
    }
    return 0;
}


void TestMdioBitBangBus(void)
{
	struct mdiobb_ctrl ctrl;
	struct mii_bus *     bus;
	int    phy;
	int    reg;
	u16    val;


#if(0)
    //
    // Get current value of Misc. Control register and make sure
    // MDIO is set to 2.5V
    //
    reg = REG32(GPIO_MISC_CTRL_REG);
    printf("Reading GPIO_MISC_CTRL_REG: 0x%8.8X\n",reg);
    reg &= ~(3<<20); // Clear current bits
//  reg |=  (0<<20); // Set as 3.3V
//  reg |=  (1<<20); // Set as 1.8V
    reg |=  (2<<20); // Set as 2.5V
//  reg |=  (3<<20); // Set as 3.0V
    printf("Setting GPIO_MISC_CTRL_REG: 0x%8.8X\n",reg);
    REG32(GPIO_MISC_CTRL_REG) = reg;

    //
    // TEMP RUN TEST TO GET VALUES OF PAD CONFIG REGISTER 5
    // FOR PATCHING LATER TO TRY AND IMPROVE RGMII SIGNALS
    //
    reg = REG32(PAD_CONF5_REG);
    printf("Reading PAD CONFIG REGISTER 5: 0x%8.8X = 0x%8.8X\n",PAD_CONF5_REG,reg);
    printf("Values slew/strength: REFCLK = %u:%u TXC = %u:%u TXD = %u:%u RXC = %u:%u RXD = %u:%u\n",
           (reg >>  2) & 1, // REFCLK SLEW
           (reg >>  0) & 3, // REFCLK STRENGTH
           (reg >>  6) & 1, // TXC    SLEW
           (reg >>  4) & 3, // TXC    STRENGTH
           (reg >> 10) & 1, // TXD    SLEW
           (reg >>  8) & 3, // TXD    STRENGTH
           (reg >> 14) & 1, // RXC    SLEW
           (reg >> 12) & 3, // RXC    STRENGTH
           (reg >> 18) & 1, // RXD    SLEW
           (reg >> 16) & 3  // RXC    STRENGTH
          );


    //
    // SET TO LOOK LIKE WHAT IT APPEARS SHOULD HAVE BEEN DEFAULTS,
    // SLOW SLEW AND DRIVE STRENGTH 2 (DRIVE STRENGTH RANGE 0-3)
    //
    //reg = 0x00066666;
    //
    // TRYING MAX DRIVE STRENGTH (3) AND FAST (0)
    //
    reg = 0x00033333;
    printf("Setting PAD CONFIG REGISTER 5: 0x%8.8X = 0x%8.8X\n",PAD_CONF5_REG,reg);
    REG32(PAD_CONF5_REG) = reg;

#endif

	bus = alloc_mdio_bitbang(&ctrl);
	if (bus==NULL)
	{
		printf("Bit Bang Bus Allocation Error\n");
		return;
	}


#ifdef ATHEROS_SWITCH_TEST_ENABLED
    //
    // TEMP FOR NOW, PUT ATHEROS SWITCH TEST CODE HERE (WILL MOVE TO PROPER PLACE LATER)
    //
    //
    // MDIO bus initialized, call Atheros initialization
    //
    printf("\nTesting Atheros Switch Initialization...\n");
    athrs17_init(bus);
    printf("\nAtheros Switch initialization complete\n");
    //
    // T2K EVM, ALSO TEST IF ABLE TO SETUP SGMII PHY FOR SGMII OPERATION
    //
    printf("\nSetting Atheros 8031 PHY for SGMII operation\n");
    //SetSgmiiAtherosPhyForT2K(bus);
#endif


#if 1
	//
	// Read and write all possible normal range PHY (0-7), extended MDIO addresses (0x8 - 0x1F, example, Atheros Switch) and normal range registers (0-0x1F)
	//
#ifdef ATHEROS_SWITCH_TEST_ENABLED
	for (phy = 0; phy<=0x1F; phy++)
	//for (phy = 4; phy<=5; phy++)  // TEMP ONLY TEST ATHEROS PHY
	{
        DumpPhyRegisters(bus,phy);
	}
    printf("\nRegister 1st read test complete, waiting 5 seconds\n");
#else
    DumpPhyRegisters(bus,4);
    DumpPhyDebugRegisters(bus,4);
    DumpPhyRegisters(bus,5);
    DumpPhyDebugRegisters(bus,5);
    printf("\nRegister 1st read test complete, setting Atheros PHY for RGMII and SGMII and waiting 5 seconds\n");
    //
    // Put PHY into proper mode for operation for RGMII and SGMII
    // (override defaults where defaults do not work)
    //
//    SetRgmiiAtherosPhyForT2K(bus);
//    SetSgmiiAtherosPhyForT2K(bus);
#endif

#endif


#if 1
    //
    // TEMP DELAY AND READ AGAIN
    //
    mdelay(5);
#ifdef ATHEROS_SWITCH_TEST_ENABLED
	for (phy = 0; phy<=0x1F; phy++)
	//for (phy = 4; phy<=5; phy++)  // TEMP ONLY TEST ATHEROS PHY
	{
        DumpPhyRegisters(bus,phy);
	}
#else
    DumpPhyRegisters(bus,4);
    DumpPhyDebugRegisters(bus,4);
    DumpPhyRegisters(bus,5);
    DumpPhyDebugRegisters(bus,5);
#endif
    printf("\nRegister 2nd read test complete\n");

#endif
    //
    // Test complete, shutdown bit bang driver
    //
	free_mdio_bitbang(bus);
}

#if(0)
void TempAtherosSwitchDumpStats(void)
{
	struct mdiobb_ctrl ctrl;
	struct mii_bus *     bus;
	int    phy;
	int    reg;
	u16    val;

	bus = alloc_mdio_bitbang(&ctrl);
	if (bus==NULL)
	{
//		printf("Bit Bang Bus Allocation Error\n");
		return;
	}


    //
    // Dump statistics for MAC port 6 (hooked up to CPU)
    //
    DumpAtherosStatistics(bus, 6);

    //
    // Test complete, shutdown bit bang driver
    //
	free_mdio_bitbang(bus);
}


#endif


