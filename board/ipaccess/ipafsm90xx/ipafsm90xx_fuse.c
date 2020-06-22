/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2020 ipaccess, Inc.
 All Rights Reserved.
----------------------------------------------------------------------
*********************************************************************/
#include <common.h>
#include <asm-generic/errno.h>

#include "ipafsm90xx_fuse.h"


#define QFPROM_FUSE_WR_BIT_ADDRESS_REG_OFFSET        0x204
#define QFPROM_FUSE_WR_PERFORM_REG_OFFSET            0x208
#define QFPROM_FUSE_WR_PERFORM                       0x66757365 /* "fuse" */
#define QFPROM_FUSE_WRITE_PAD_EN_REG_OFFSET          0x20c
#define QFPROM_FUSE_WRITE_PAD_EN_VALUE               0x656e626c /* "enbl" */
#define QFPROM_FUSE_WRITE_PAD_REG_OFFSET             0x210
#define QFPROM_FUSE_WRITE_PAD_VALUE                  0x56444451 /* "VDDQ" */
#define QFPROM_MAX_FUSE_INDEX                        4096
#define QFPROM_GLOBAL_LAST_TIME_PROGRAM_FUSE         0x3E1

#define IOWRITE32(vALUE, aDDRESS)                       *((volatile u32 *)(aDDRESS)) = (u32) (vALUE)
#define IOREAD32(aDDRESS)                               (*((volatile u32 *)(aDDRESS)))

u32 read_fuse(unsigned int index)
{
    volatile u32 * address;
    u32 val;
    
    address = (volatile u32 *)(index);
    val = *address;
    
    return val;
}

int read_fuse_in_range(unsigned int offset, u32 *buffer, unsigned int *buffer_len)
{
    u32  val = 0;
    val = read_fuse(offset);
    *buffer = val;
    return 0;
}

uint8_t ipafsm90xx_is_board_fused()
{
    unsigned int offset;
    unsigned int val;    
    uint8_t ret = 0;
    int i =0;

	for(i=0;i<8;i++)
	{
	    offset = QFPROM_CORR_FUSE_PK_HASH_ROW0_LSB + 4*i; 
    	val = read_fuse(offset); 
		if(val != 0)    
		{
		    ret = 1;
		    break;    
		}    
	}
    return ret;
}

int read_secure_boot_fuse(void)
{
    return read_fuse(QFPROM_CORR_FUSE_SECURE_BOOT_ROW0_LSB);
}

