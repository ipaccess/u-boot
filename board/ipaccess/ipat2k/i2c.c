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
#include <asm/arch/hardware.h>


// Mapping of diagnostic types to standard C

#ifndef UINT32
#define UINT32 unsigned int
#endif

#ifndef PUINT32
#define PUINT32 unsigned int *
#endif

#ifndef VUINT32
#define VUINT32 volatile unsigned int
#endif

#ifndef UINT16
#define UINT16 unsigned short
#endif

#ifndef PUINT16
#define PUINT16 unsigned short *
#endif

#ifndef VUINT16
#define VUINT16 volatile unsigned short
#endif

#ifndef UINT8
#define UINT8 unsigned char
#endif

#ifndef PUINT8
#define PUINT8 unsigned char *
#endif


#ifndef SYSRC_SUCCESS
#define SYSRC_SUCCESS 0
#endif

unsigned int get_bus_clock(void);

#define EXPBUSTICKS_IN_1_US  (get_bus_clock())
#define EXPBUSTICKS_IN_1_MS ((get_bus_clock()) * 1000)

ulong get_timer (ulong base);
#define SysTime (get_timer(0))

#define I2C_WRITE		0
#define I2C_READ		1
#define START_MASTER		(1<<5)
#define I2C_MAX_TIMEOUT		10000   /* 10 milliseconds for timeout */

/*. STAT - Status codes */
#define I2C_BUS_ERROR			0x00	/* Bus error in master mode only */
#define I2C_START_TRANSMIT		0x08	/* Start condition transmitted */
#define I2C_REPEAT_START_TRANSMIT	0x10	/* Repeated Start condition transmited */
#define I2C_ADDRESS_W_ACK		0x18	/* Address + Write bit transmitted, ACK received */
#define I2C_ADDRESS_W_NACK		0x20	/* Address + Write bit transmitted, NACK received */
#define I2C_DATA_TRANSMIT_ACK		0x28	/* Data byte transmitted in master mode , ACK received */
#define I2C_DATA_TRANSMIT_NACK		0x30	/* Data byte transmitted in master mode , NACK received */
#define I2C_ARBIT_LOST			0x38	/* Arbitration lost in address or data byte */
#define I2C_ADDRESS_R_ACK		0x40	/* Address + Read bit transmitted, ACK received  */
#define I2C_ADDRESS_R_NACK		0x48	/* Address + Read bit transmitted, NACK received  */
#define I2C_DATA_RECEIVE_ACK		0x50	/* Data byte received in master mode, ACK transmitted  */
#define I2C_DATA_RECEIVE_NACK		0x58	/* Data byte received in master mode, NACK transmitted*/
#define I2C_ARBIT_LOST_ADDRESS		0x68	/* Arbitration lost in address  */
#define I2C_GENERAL_CALL		0x70	/* General Call, ACK transmitted */
#define I2C_NO_RELEVANT_INFO		0xF8	/* No relevant status information, IFLF=0 */

#define I2C_RC_TIMEOUT			-1UL
#define I2C_RC_NACK			-2UL
#define I2C_RC_HAVE_NOT_DEVICE		-3UL
#define I2C_RC_SET_MASTER_ERROR		-4UL

#define abs(val) ((int)(val) < 0) ? -(val) : (val)

#if 0
#define DEBUG printf
#else
#define DEBUG(msg,...)
#endif

// Forward declarations:
void I2CDrvResetDevice(void);

// Functions:

static u8 I2C_IS_IFLG_SET(void) {
	u32 timeout = 0;

	while (!(REG32(I2C_CNTR_REG) & I2C_IFLG)) {
		timeout++;
		if (timeout >= I2C_MAX_TIMEOUT)
			return 0;
		udelay (1);
	}
	return 1;
}

/* Calc Foscl = Fclk / ((2 << N) * (M + 1) * 10) */
static int i2c_calc_baudrate( int speed )
{
	unsigned int n, m, freq, delta, power;
	unsigned int actualFreq, actualN=0, actualM=0;
	unsigned int deltamin = -1;

	for(n = 0 ; n < 8 ; n++) {
		for(m = 0 ; m < 16 ; m++) {
			power = 2 << n;
			freq = AXICLK_HZ / (10 * (m + 1) * power);
			delta = abs(speed - freq);
			if(delta < deltamin)
			{
				deltamin    = delta;
				actualFreq  = freq;
				actualN	    = n;
				actualM	    = m;
			}
		}
	}

	return (actualM << 4) | actualN;
}

void i2c_init(int speed, int slaveaddr)
{
	DEBUG("i2c_init: speed=%d, slaveaddr=0x%02X\n", speed, slaveaddr);

	REG32(I2C_DATA_REG)  = 0;
	REG32(I2C_ADDR_REG)  = 0;
	REG32(I2C_XADDR_REG) = 0;
	REG32(I2C_CNTR_REG)  = 0;
	REG32(I2C_CCRFS_REG) = i2c_calc_baudrate(speed);

	I2CDrvResetDevice();

	REG32(I2C_DATA_REG)  = 0;
	REG32(I2C_ADDR_REG)  = 0;
	REG32(I2C_XADDR_REG) = 0;
	REG32(I2C_CNTR_REG)  = 0;
	REG32(I2C_CCRFS_REG) = i2c_calc_baudrate(speed);
}


/*!
 * @brief i2c_read: U-boot standard API to read data from I2C device
 *
 *   @param[in] chip:    I2C chip address, range 0..127
 *   @param[in] addr:    Memory (register) address within the chip
 *   @param[in] alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *                       memories, 0 for register type devices with only one
 *                       register)
 *   @param[in] buffer:  Where to read/write the data
 *   @param[in] len:     How many bytes to read/write
 *
 *   @returns: 0 on success, not 0 on failure
 */
int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	u8  result;
	u16 i = 0;

	DEBUG("i2c_read: chip=0x%02X, alen=%d, offs=0x%04X, len=%d\n", chip, alen, addr, len);

#ifdef CONFIG_SYS_I2C_EEPROM_ADDR
	// If EEPROM, force to correct length
	if (chip == CONFIG_SYS_I2C_EEPROM_ADDR)
		alen =  CONFIG_SYS_I2C_EEPROM_ADDR_LEN;
#endif

	//put dev into master mode and transmit the start condition
	REG32(I2C_CNTR_REG) = I2C_STA;

	if ( !I2C_IS_IFLG_SET() ) {
		REG32(I2C_CNTR_REG) = START_MASTER;
		if (!I2C_IS_IFLG_SET()) {
			DEBUG("i2c_read: ERROR START_MASTER TIMEOUT\n");
			return I2C_RC_TIMEOUT;
		}
	}

	// Get result from start master operation
	result = REG32(I2C_STAT_REG);

	// Test if OK to start transmitting
	if (result == I2C_START_TRANSMIT ||  result== I2C_REPEAT_START_TRANSMIT)
	{
		// OK to start transmitting, issue I2C device (chip) address
		// and I2C write operation
		REG32(I2C_DATA_REG) = ((chip << 1) & 0xFE)|I2C_WRITE;

		//to send it, must clear the iflg (OK here to clear all flags)
                REG32(I2C_CNTR_REG) = 0;
	}

	// wait for TX to complete, return timeout error if failed
	if (!I2C_IS_IFLG_SET()) {
		DEBUG("i2c_read: ERROR Write address cycle TIMEOUT\n");
		return I2C_RC_TIMEOUT;
	}

	//check what's returned, if acked then continue with write
	result = REG32(I2C_STAT_REG);
	if (result != I2C_ADDRESS_W_ACK ) {
		DEBUG("i2c_read: Write address not ACK'ed\n");
#if 1
		I2CDrvResetDevice();  // Reset I2C back to known state
#endif
		return I2C_RC_NACK;
	}
	//
	// Device (chip) write completed OK, check data address length
	// If non-zero, write one address byte at a time, most
	// significant address byte first
	//
	while(alen) {
		// Address field specified, always write most significant
		// byte first
		alen--;
		REG32(I2C_DATA_REG) = (addr >> (((unsigned)alen) << 3)) & 0xFF;
		REG32(I2C_CNTR_REG) &= ~I2C_IFLG;
		if (!I2C_IS_IFLG_SET()) {
			DEBUG("i2c_read: Write data address %d TIMEOUT\n",alen);
			return I2C_RC_TIMEOUT;
		}
	}

	//---------------------------------------------
	// I2C device address and register address
	// have been sent.
	// Enter the sequential read
	//---------------------------------------------
	// Write cycle completed, setup I2C restart to do read operation,
	// send a new start condition

	REG32(I2C_CNTR_REG) = START_MASTER;
	if ( !I2C_IS_IFLG_SET() ) {
		// Start Master failed, try one more time...
		REG32(I2C_CNTR_REG) = START_MASTER;
		if (!I2C_IS_IFLG_SET()) {
			DEBUG("i2c_read: ERROR START_MASTER TIMEOUT\n");
			return I2C_RC_TIMEOUT;
		}
	}

	// now send the opcode and set to READ
	REG32(I2C_DATA_REG) = ((chip << 1) & 0xFE) | I2C_READ;

	// to send it, must clear the iflg, note its ok here only to reset all flags to zero
	REG32(I2C_CNTR_REG) = 0;
	if (!I2C_IS_IFLG_SET()) {
		DEBUG("i2c_read: Read address TIMEOUT\n");
		return I2C_RC_TIMEOUT;
	}

	// check if Read command was good, if not, exit
	result = REG32(I2C_STAT_REG);
	if (result != I2C_ADDRESS_R_ACK ) {
		DEBUG("i2c_read: Read address not ACK'ed\n");
#if 1
		I2CDrvResetDevice();  // Reset I2C back to known state
#endif
		return I2C_RC_NACK;
	}

	// depending on length we need to send an ack,
	// or if len=1, send no ack in prep for a stop
	if (len > 1)
		// len > 1
		// send an ACK
		REG32(I2C_CNTR_REG) = I2C_AAK;
	else if (len == 1)
		// len is 1
		// send no ACK when the last byte is recvd
		// to send it, must clear the iflg,
		// note its ok here only to reset all flags to zero
		REG32(I2C_CNTR_REG) = 0;
	else  {
		// len == 0
		// wait the for flag, then clear CNTR register
		if (!I2C_IS_IFLG_SET()) {
			DEBUG("i2c_read: Read len==0 TIMEOUT\n");
			return I2C_RC_TIMEOUT;
		}
		REG32(I2C_CNTR_REG) = 0;
	}

	for (i = 0; i < len; i++) {
		if (!I2C_IS_IFLG_SET()) {
			DEBUG("i2c_read: Read offset %d TIMEOUT\n",i);
			return I2C_RC_TIMEOUT;
		}
		// Get byte to send
		*(u8 *)(buffer + i) = (u8) (*(volatile u32*) I2C_DATA_REG);
		if (i == len-1)	{
			// Setup stop if last byte sent
			REG32(I2C_CNTR_REG) = I2C_STP;
			REG32(I2C_CNTR_REG) = 0;
		} else 	{
			// Not last byte, set ACK if second to last byte
			if (i < len-2)
				REG32(I2C_CNTR_REG) = I2C_AAK;
			else
				REG32(I2C_CNTR_REG) = 0;
		}
	}
	return 0;
}

/*!
 * @brief i2c_wrte: U-boot standard API to write data to I2C device
 *
 *   @param[in] chip:    I2C chip address, range 0..127
 *   @param[in] addr:    Memory (register) address within the chip
 *   @param[in] alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *                       memories, 0 for register type devices with only one
 *                       register)
 *   @param[in] buffer:  Where to read/write the data
 *   @param[in] len:     How many bytes to read/write
 *
 *   @returns: 0 on success, not 0 on failure
 */
int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	volatile u32 status;
	u32 i = 0;

	DEBUG("i2c_write: chip=0x%02X, alen=%d, offs=0x%04X, len=%d\n", chip, alen, addr, len);

#ifdef CONFIG_SYS_I2C_EEPROM_ADDR
	// If EEPROM, force to correct length
	if (chip == CONFIG_SYS_I2C_EEPROM_ADDR)
		alen =  CONFIG_SYS_I2C_EEPROM_ADDR_LEN;
#endif

	//put dev into master mode and transmit the start condition
	REG32(I2C_CNTR_REG) = I2C_STA;
	if ( !I2C_IS_IFLG_SET() ) {
		REG32(I2C_CNTR_REG) = START_MASTER;
		if (!I2C_IS_IFLG_SET()) {
			DEBUG("i2c_write: START_MASTER TIMEOUT\n");
			return I2C_RC_TIMEOUT;
		}
	}

	status = *(volatile u32*) I2C_STAT_REG;
	if (status == I2C_START_TRANSMIT        ||
	    status == I2C_REPEAT_START_TRANSMIT
	   )
	{
		REG32(I2C_DATA_REG) = ((chip << 1) & 0xFE) | I2C_WRITE;
		REG32(I2C_CNTR_REG) &= ~I2C_IFLG;
	}

	if (!I2C_IS_IFLG_SET()) {
		DEBUG("i2c_write: chip %d address write timeout\n",chip);
		return I2C_RC_TIMEOUT;
	}

	//check what's returned, if acked then continue with dummy write
	status = REG32(I2C_STAT_REG);
	if (status != I2C_ADDRESS_W_ACK) {
		DEBUG("i2c_write: chip %d address not ACK'ed\n",chip);
		//
		// Device address Ack failed, reset the device to go back
		// to idle state
		//
		I2CDrvResetDevice();  // Reset I2C back to known state
		return I2C_RC_SET_MASTER_ERROR;
	}

	//
	// Device (chip) write completed OK, check data address length
	// If non-zero, write one address byte at a time, most
	// significant address byte first
	//
	while(alen) {
		// Address field specified, always write most significant
		// byte first
		alen--;
		REG32(I2C_DATA_REG) = (addr >> (alen*8)) & 0xFF;
		REG32(I2C_CNTR_REG) &= ~I2C_IFLG;
		if (!I2C_IS_IFLG_SET()) {
			DEBUG("i2c_write: Write data address %d TIMEOUT\n",alen);
			return I2C_RC_TIMEOUT;
		}
		status = REG32(I2C_STAT_REG);
	}
	//
	// Address field (if any) written
	// Write data bytes (if any)
	//
	for (i=0; i < len; i++)
	{
		REG32(I2C_DATA_REG)  = (u8) buffer [i];
	     	REG32(I2C_CNTR_REG) &= ~I2C_IFLG;
		if (!I2C_IS_IFLG_SET()) {
			DEBUG("i2c_write: Write Data offset %d TIMEOUT\n",i);
			return I2C_RC_TIMEOUT;
		}
	}

	*(volatile u32*) I2C_CNTR_REG = I2C_STP;

	return 0;
}

/*!
 * @brief Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */
int i2c_probe(uchar chip)
{
	uchar dummy[4];

	return i2c_read(chip, 0, 1, dummy, 0);
}

unsigned I2CDrvWriteBytes(unsigned char  DeviceAddress,
                          unsigned char  RegisterAddress,
                          unsigned char* pData,
                          unsigned short RequestedLength
                         )
{

    unsigned char result;
    unsigned short i=0; //note len cannot exceed 64 kbytes
    //PUINT8 ptarget_address = (PUINT8)target_address;

    //put dev into master mode and transmit the start condition
    REG32(I2C_CNTR_REG) = I2C_STA;

    if (!I2C_IS_IFLG_SET())
    {
        REG32(I2C_CNTR_REG) = START_MASTER;
        if (!I2C_IS_IFLG_SET())
		return I2C_RC_TIMEOUT;
    }


    //---------------------------------------------
    //enter dummy write mode
    //this assumes a one byte address i.e. small device, not an EEPROM
    //---------------------------------------------
    result = REG32(I2C_STAT_REG);

    if (result == I2C_START_TRANSMIT ||  result== I2C_REPEAT_START_TRANSMIT)
    {
        //now send the opcode and set to write
        REG32(I2C_DATA_REG) = (DeviceAddress&0xFE)|I2C_WRITE;

        //to send it, must clear the iflg, note its ok here only to reset all flags to zero
        REG32(I2C_CNTR_REG) = 0;
    }

    //wait for tx to complete- no error handling here
    if (!I2C_IS_IFLG_SET())
        return I2C_RC_TIMEOUT;

    //check what's returned, if acked then continue with dummy write
    result = REG32(I2C_STAT_REG);

    if (result !=I2C_ADDRESS_W_ACK )
    {
        return 0;
    }


    //write the Reqister address to start write operation from
    REG32(I2C_DATA_REG) = RegisterAddress;

    //clear the iflag, to send the register address
    REG32(I2C_CNTR_REG) = 0;

    // wait for tx to complete- no error handling here
    if (!I2C_IS_IFLG_SET())
        return I2C_RC_TIMEOUT;

    for (i=0; i<RequestedLength; i++)
    {
        // -----------------------
        // DATA WRITE OPERATION
        // -----------------------
        REG32(I2C_DATA_REG) = (unsigned char)pData[i];

        //clear the iflag, to send
        REG32(I2C_CNTR_REG) &= ~I2C_IFLG;

        //wait for tx to complete
		if (!I2C_IS_IFLG_SET())
			return I2C_RC_TIMEOUT;
    }

    REG32(I2C_CNTR_REG) = I2C_STP; // I2C_STP

    udelay(3);

    return ((unsigned)i);
}

void I2CDrvResetDevice(void)
{
	u32 i;

	REG32(I2C_CTRL) |= 1;

	// The algorithm of device resetting is:
	//   - Issue 9 START conditional
	//   - Send  9 data bits :  111111111
	//   - Send one START conditional
	//   - Send one STOP conditional

	//assume we have initialized the HW before
	// Issue 9 START conditional
	for (i = 0; i < 9; i++) {
		//put dev in master mode and tx a start condition
		REG32(I2C_CNTR_REG) = I2C_STA;
		I2C_IS_IFLG_SET(); //wait, but if no transition ignore
	}

	// Send  9 data bits :  111111111, 8+1 from the ack, i2c ip will clock 9 pulses for us. note bus is held hi
	REG32(I2C_DATA_REG) = 0xFF;
	// To send it, we must clear the iflg.
	REG32(I2C_CNTR_REG) = 0x0;
	I2C_IS_IFLG_SET(); //wait, but if no transition ignore

	// Send one START conditional
	REG32(I2C_CNTR_REG) = I2C_STA;
	I2C_IS_IFLG_SET(); //wait, but if no transition ignore

	//   - Send one STOP conditional
	REG32(I2C_CNTR_REG) = I2C_STP;
	REG32(I2C_CNTR_REG) = 0; //we are done!!!! clear the flag

	//undo the mux
	REG32(I2C_CTRL) &= ~1;
}

