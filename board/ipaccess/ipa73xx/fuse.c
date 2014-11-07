#include <common.h>
#include <fuse.h>
#include <asm-generic/errno.h>


/* IPA73XX - dummy routine for the time being */

int do_fuse_read(u32 bank, u32 word, u32 *val, const char * caller)
{
#if 0
    static const u32 FUSE_MEM_BASE = 0x80000000;
    static const u32 FEC_ESR_OFFSET = 0x60A8;

    volatile u16 * fec_esr_pointer = (volatile u16 *)(FUSE_MEM_BASE + FEC_ESR_OFFSET);

    if ((*fec_esr_pointer) & 0x003F != 0x0000)
    {
        u16 v = *fec_esr_pointer;
        *fec_esr_pointer = v;
        debug("one of FEC_ESR[5:0] was set and has been cleared\n");
    }

    volatile u32 * address = (volatile u32 *)(FUSE_MEM_BASE + bank + (sizeof(u32) * word));
    *val = *address;

    if ((*fec_esr_pointer) & 0x003F != 0x0000)
    {
        u16 v = *fec_esr_pointer;
        *fec_esr_pointer = v;
        printf("ipafsm92xx %s(): FEC error - 0x%04X\n", caller, v);
        return -EIO;
    }
#else
    *val = 0;
#endif

    return 0;
}




int fuse_read(u32 bank, u32 word, u32 *val)
{
    return do_fuse_read(bank, word, val, __func__);
}




int fuse_sense(u32 bank, u32 word, u32 *val)
{
    return do_fuse_read(bank, word, val, __func__);
}




int fuse_prog(u32 bank, u32 word, u32 val)
{
    return -ENOSYS;
}




int fuse_override(u32 bank, u32 word, u32 val)
{
    return -ENOSYS;
}
