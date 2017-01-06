/*
 * Copyright(c) 2007-2015 Intel Corporation. All rights reserved.
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
#include "efuse.h"

#define EFUSE_INST_CNT_MASK         0x3F
#define OVERRIDE_AS_DISABLE         0x00020000
#define EFUSE_ADD_MASK              0x7
#define EFUSE_GPIO_SET_0            0x0
#define EFUSE_GPIO_SET_1            0x1
#define EFUSE_GPIO_SET_DEFAULT      0x2
//gpio_11,6,10 control Fsource 0, 1.2

static void efuse_set_Fsource(int enable, int inst_no)
{
    int gpio = 0;
    switch (inst_no % 3){
        case 0: gpio = 11; break;
        case 1: gpio = 6; break;
        case 2: gpio = 10;break;
    }

    if(enable == EFUSE_GPIO_SET_1){
        REG32(GPIO_OUTPUT_REG)  |= (1 << gpio);
        REG32(GPIO_OE_REG)      |= (1 << gpio);
    }else if(enable == EFUSE_GPIO_SET_0){
        REG32(GPIO_OUTPUT_REG)  &= ~(1<< gpio);
        REG32(GPIO_OE_REG)      |= (1 << gpio);
    }else if(enable == EFUSE_GPIO_SET_DEFAULT){
        REG32(GPIO_OUTPUT_REG)  &= ~(1<< gpio); //default value OP =  0
        REG32(GPIO_OE_REG)      &= ~(1 << gpio); //default value OE = 0
    }else
        return;
}
static void efuse_signal_set(int enable)
{
    if(enable){
        efuse_set_Fsource(EFUSE_GPIO_SET_0,0);
        efuse_set_Fsource(EFUSE_GPIO_SET_0,1);
        efuse_set_Fsource(EFUSE_GPIO_SET_0,2);
        REG32(EFUSE_CS)     = 0;
        REG32(EFUSE_SIGDEV) = 0;
        REG32(EFUSE_FSET)   = 0;
        REG32(EFUSE_PROG)   = 0;
        REG32(EFUSE_PRCHG)  = 0;

        REG32(EFUSE_SCK)    = 0;
        REG32(EFUSE_SDI)    = 0;
        REG32(EFUSE_ADDR)   = 0;
    }else{
        efuse_set_Fsource(EFUSE_GPIO_SET_DEFAULT,0);
        efuse_set_Fsource(EFUSE_GPIO_SET_DEFAULT,1);
        efuse_set_Fsource(EFUSE_GPIO_SET_DEFAULT,2);
        REG32(EFUSE_CS)     = 0;
        REG32(EFUSE_SIGDEV) = 0;
        REG32(EFUSE_FSET)   = 0;
        REG32(EFUSE_PROG)   = 0;
        REG32(EFUSE_PRCHG)  = 0;

        REG32(EFUSE_SCK)    = 0;
        REG32(EFUSE_SDI)    = 0;
        REG32(EFUSE_ADDR)   = 0;

    }
}

int efuse_read_instance(UINT32 inst_no, UINT8* buff)
{
    UINT32 i;
    UINT32  temp[4];
    while ( (REG32(EFUSE_STATUS) & 0x1) != 1 );
    REG32(BOOTSTRAP_OVERRIDE_REG ) |= (OVERRIDE_AS_DISABLE);

    efuse_signal_set(1);
    REG32(EFUSE_INST_CNT) = inst_no;

    REG32(EFUSE_CS) = 1;
    for ( i = 0 ; i < 8 ; i++ ) {
        REG32(EFUSE_ADDR)   = i;
        REG32(EFUSE_SIGDEV) = 1;
        REG32(EFUSE_PRCHG)  = 1;
        REG32(EFUSE_FSET)   = 1;
        REG32(EFUSE_SIGDEV) = 0;
        REG32(EFUSE_PRCHG)  = 0;
        REG32(EFUSE_FSET)   = 0;
    }
    REG32(EFUSE_ADDR)   = 0;
    REG32(EFUSE_CS)     = 0;
    SysDelayUs(5);

    temp[0] = REG32(EFUSE_DOUT0);
    temp[1] = REG32(EFUSE_DOUT1);
    temp[2] = REG32(EFUSE_DOUT2);
    temp[3] = REG32(EFUSE_DOUT3);
    memcpy(buff, (UINT8 *)temp, 16);

    efuse_signal_set(0);
    memset(temp,0,sizeof(temp));
    return 0;
}

int efuse_write_instance(UINT32 inst_no, UINT8 *buff)
{
/*Commented out deliberately so that fuses are not blown by mistake*/
    int i,j;
    int d_id, b_id, bit;
    UINT8 *data = (UINT8 *)buff;

    while ( (REG32(EFUSE_STATUS) & 0x1) != 1 );
    REG32(BOOTSTRAP_OVERRIDE_REG ) |= OVERRIDE_AS_DISABLE;

    efuse_signal_set(1);

    REG32(EFUSE_INST_CNT) = (inst_no & EFUSE_INST_CNT_MASK);
    REG32(EFUSE_CS) = 1;
    for(i = 0; i < 128; i++){
        REG32(EFUSE_SCK) = 1;
        REG32(EFUSE_SCK) = 0;
    }
    REG32(EFUSE_SDI) = 1;
    REG32(EFUSE_ADDR) = 6;
    for(i = 0; i < 64; i ++){
        REG32(EFUSE_ADDR) = 7;
        REG32(EFUSE_ADDR) = 6;
    }
    REG32(EFUSE_SDI) = 0;
    REG32(EFUSE_ADDR) = 0;

    efuse_set_Fsource(EFUSE_GPIO_SET_1, inst_no);
    SysDelayUs(1200);
    for(i = 0; i < 128; i++){
        d_id = i/8;
        b_id = i%8;
        bit = (*(data + d_id) >> b_id) & 1;

        if (i == 0){
            REG32(EFUSE_SDI) = 1;
            REG32(EFUSE_SCK) = 1;
            REG32(EFUSE_SCK) = 0;
            REG32(EFUSE_SDI) = 0;
        }else{
            REG32(EFUSE_SCK) = 1;
            REG32(EFUSE_SCK) = 0;
        }

        if(bit){
            for(j = 0; j < 10; j++)
                REG32(EFUSE_PROG) = 1;
        }else{
            for(j = 0; j < 10; j++)
                REG32(EFUSE_PROG) = 0;
        }
        REG32(EFUSE_PROG) = 0;

    }
    efuse_set_Fsource(EFUSE_GPIO_SET_0, inst_no);
    REG32(EFUSE_CS) = 0;
    efuse_signal_set(0);
    /*Let's put some delay before the next call to this func*/
    SysDelayUs(1200);
    return 0;
}
