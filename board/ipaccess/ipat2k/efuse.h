/*
 *  * Copyright(c) 2007-2015 Intel Corporation. All rights reserved.
 *  *
 *  * This program is free software; you can redistribute it and/or modify
 *  * it under the terms of version 2 of the GNU General Public License as
 *  * published by the Free Software Foundation.
 *  *
 *  * This program is distributed in the hope that it will be useful, but
 *  * WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  * General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program; if not, write to the Free Software
 *  * Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *  * The full GNU General Public License is included in this distribution
 *  * in the file called LICENSE.GPL.
 *  *
 *  * Contact Information:
 *  * Intel Corporation
 *  */

#ifndef _EFUSE_H_
#define _EFUSE_H_

#include <asm/arch/transcede-2200.h>
#include <asm/arch/systypes.h>

/* EFUSE control registers*/
#define OTP_BASEADDR                    0xFE0F0000
#define EFUSE_BASEADDRESS               OTP_BASEADDR
#define EFUSE_CS                        (EFUSE_BASEADDRESS + 0x00)
#define EFUSE_ADDR                      (EFUSE_BASEADDRESS + 0x04)
#define EFUSE_SCK                       (EFUSE_BASEADDRESS + 0x08)
#define EFUSE_PROG                      (EFUSE_BASEADDRESS + 0x0C)
#define EFUSE_PRCHG                     (EFUSE_BASEADDRESS + 0x10)
#define EFUSE_SIGDEV                    (EFUSE_BASEADDRESS + 0x14)
#define EFUSE_FSET                      (EFUSE_BASEADDRESS + 0x18)
#define EFUSE_SDI                       (EFUSE_BASEADDRESS + 0x1C)
#define EFUSE_DOUT0                     (EFUSE_BASEADDRESS + 0x20)
#define EFUSE_DOUT1                     (EFUSE_BASEADDRESS + 0x24)
#define EFUSE_DOUT2                     (EFUSE_BASEADDRESS + 0x28)
#define EFUSE_DOUT3                     (EFUSE_BASEADDRESS + 0x2C)
#define EFUSE_INST_CNT                  (EFUSE_BASEADDRESS + 0x30)
#define EFUSE0_DOUT0                    (EFUSE_BASEADDRESS + 0x34)
#define EFUSE1_DOUT1                    (EFUSE_BASEADDRESS + 0x38)
#define EFUSE2_DOUT2                    (EFUSE_BASEADDRESS + 0x3C)
#define EFUSE3_DOUT3                    (EFUSE_BASEADDRESS + 0x40)
#define EFUSE_STATUS                    (EFUSE_BASEADDRESS + 0x44)


extern int efuse_write_instance (UINT32 inst_no, UINT8 *buff);
extern int efuse_read_instance (UINT32 inst_no, UINT8* buff);
#endif

