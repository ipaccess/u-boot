#include <common.h>
#include <asm-generic/errno.h>

#include "ipa73xx_fuse.h"

#define PICOXCELL_FUSE_BASE_ADDRESS                     0x80080000
#define PICOXCELL_FUSE_CTRL_REG_OFFSET                  0x200
#define PICOXCELL_FUSE_CTRL_WRITE_BUSY                  (1 << 0)
#define PICOXCELL_FUSE_CTRL_VDDQ_OE                     (1 << 1)
#define PICOXCELL_FUSE_CTRL_VDDQ                        (1 << 2)
#define PICOXCELL_FUSE_WR_BIT_ADDRESS_REG_OFFSET        0x204
#define PICOXCELL_FUSE_WR_PERFORM_REG_OFFSET            0x208
#define PICOXCELL_FUSE_WR_PERFORM                       0x66757365 /* "fuse" */
#define PICOXCELL_FUSE_WRITE_PAD_EN_REG_OFFSET          0x20c
#define PICOXCELL_FUSE_WRITE_PAD_EN_VALUE               0x656e626c /* "enbl" */
#define PICOXCELL_FUSE_WRITE_PAD_REG_OFFSET             0x210
#define PICOXCELL_FUSE_WRITE_PAD_VALUE                  0x56444451 /* "VDDQ" */
#define PICOXCELL_MAX_FUSE_INDEX                        4096
#define PICOXCELL_GLOBAL_LAST_TIME_PROGRAM_FUSE         0x3E1

#define IOWRITE32(vALUE, aDDRESS)                       *((volatile u32 *)(aDDRESS)) = (u32) (vALUE)
#define IOREAD32(aDDRESS)                               (*((volatile u32 *)(aDDRESS)))

u8 test = 0;

void set_fuse_test_mode(void)
{
    test = 1;
}

int blow_fuse(unsigned int index)
{
    if ( (index > PICOXCELL_MAX_FUSE_INDEX) || read_fuse(PICOXCELL_GLOBAL_LAST_TIME_PROGRAM_FUSE) )
    {
        printf("\ncannot blow fuse %d: out of range or last time program fuse is blown\n");
        return 1;
    }

    if (test)
    {
        u32 *p32 = (u32 *)(PICOXCELL_FUSE_BASE_ADDRESS) + (index >> 5);
        *p32 |= (1 << (index & 0x1F));
    }
    else
    {
        unsigned int control;

        /* before going ahead make sure write operation is not ongoing. */
        do {
            control = IOREAD32(PICOXCELL_FUSE_BASE_ADDRESS +
                    PICOXCELL_FUSE_CTRL_REG_OFFSET);
        } while (control & PICOXCELL_FUSE_CTRL_WRITE_BUSY);	

        IOWRITE32(index,PICOXCELL_FUSE_BASE_ADDRESS + PICOXCELL_FUSE_WR_BIT_ADDRESS_REG_OFFSET);
        IOWRITE32(PICOXCELL_FUSE_WRITE_PAD_EN_VALUE, PICOXCELL_FUSE_BASE_ADDRESS +
                PICOXCELL_FUSE_WRITE_PAD_EN_REG_OFFSET);
        IOWRITE32(PICOXCELL_FUSE_WRITE_PAD_VALUE, PICOXCELL_FUSE_BASE_ADDRESS +
                PICOXCELL_FUSE_WRITE_PAD_REG_OFFSET);

        /* Give the external circuitry chance to take effect. */
        udelay(2);

        /* Start the fuse blowing process. */
        IOWRITE32(PICOXCELL_FUSE_WR_PERFORM, PICOXCELL_FUSE_BASE_ADDRESS +
                PICOXCELL_FUSE_WR_PERFORM_REG_OFFSET);

        /* Wait for the operation to complete. */
        do {
            control = IOREAD32(PICOXCELL_FUSE_BASE_ADDRESS +
                    PICOXCELL_FUSE_CTRL_REG_OFFSET);
        } while (control & PICOXCELL_FUSE_CTRL_WRITE_BUSY);


        IOWRITE32(0, PICOXCELL_FUSE_BASE_ADDRESS +
                PICOXCELL_FUSE_WRITE_PAD_EN_REG_OFFSET);
        IOWRITE32(0, PICOXCELL_FUSE_BASE_ADDRESS +
                PICOXCELL_FUSE_WRITE_PAD_REG_OFFSET);
        IOWRITE32(0, PICOXCELL_FUSE_BASE_ADDRESS +
                PICOXCELL_FUSE_WR_PERFORM_REG_OFFSET);
        udelay(2);
    }

    return 0;

}

int blow_fuse_in_range(unsigned int offset,char *buffer,unsigned int *buffer_len)
{
    /*read the value index by index from the buffer and write the corresponding fuse*/
    unsigned int buff_index = 0, fuse_index = 0;
    unsigned int ret = 0;
    while(buff_index < *buffer_len)
    {

        fuse_index = offset + buff_index;
        if ( buffer[buff_index] && 0 != blow_fuse(fuse_index))
        {
            ret = 1;
            break;
        }
        ++buff_index;

    }

    if (ret == 1)
    {
        printf("\nBlowing in range failed at index %d\n",fuse_index);
    }	

    return ret;


}

int read_fuse(unsigned int index)
{
    unsigned int word_index = index >> 5, bit = (index & 0x1F);
    u32 val = 0;

    val = IOREAD32( (u32 *)PICOXCELL_FUSE_BASE_ADDRESS + word_index);

    return !!(val & (1 << bit));


}

int read_fuse_in_range(unsigned int offset, char *buffer, unsigned int *buffer_len)
{
    int i = 0;
    u8  val = 0;
    while(i < (*buffer_len))
    {
        val = read_fuse(offset+i);
        buffer[i] = val;
        ++i;
    }

    return 0;
}


/* Read fuses from offset, fill result from LSB upwards for number of fuses required
 * (max 32)
 */
unsigned int read_and_reverse_fuses(unsigned int offset, int n_fuses)
{
    int i;
    int j = (n_fuses <= 32) ? n_fuses : 32;
    unsigned int val = 0;
    unsigned int f = offset;
    
    for (i = 0; i < j; ++i, ++f)
    {
        val |= ((unsigned int)read_fuse(f) << i);
    }
    
    return val;
}
