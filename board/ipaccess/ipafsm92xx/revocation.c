#include <revocation.h>
#include <common.h>
#include <fuse.h>

#define QFPROM_CORR_ANTI_REPLAY (0x41E8)



/*
 * Anti-rollback fuses: (Region 6)
 *   LSB (offset 0x41E8):
 *    Bits 1:0   (2 bits)  Qualcomm use only, do not change
 *    Bits 11:2  (10 bits) Loader revocation count
 *    Bits 31:12 (20 bits) Lower bits of application revocation count
 *   MSB: (offset 0x41EC)
 *    Bits 31:0  (32 bits) Upper bits of application revocation count
 */
u64 get_board_revocation(void)
{
    u32 revocation_lsb_;
    u32 revocation_msb_;
    u64 revocation_lsb;
    u64 revocation_msb;

    if (0 != fuse_sense(QFPROM_CORR_ANTI_REPLAY, 0, &revocation_lsb_))
    {
        debug("%s: failed to read revocation LSB\n", __func__);
        return 0xFFFFFFFFFFFFFFFFULL;
    }

    if (0 != fuse_sense(QFPROM_CORR_ANTI_REPLAY, 1, &revocation_msb_))
    {
        debug("%s: failed to read revocation MSB\n", __func__);
        return 0xFFFFFFFFFFFFFFFFULL;
    }

    revocation_lsb = ((revocation_lsb_ & 0xFFFFF000) >> 12);
    revocation_msb = ((revocation_msb_ & 0xFFFFFFFF) >>  0);
    revocation_msb <<= 20;
    return ((revocation_lsb | revocation_msb) & 0x000FFFFFFFFFFFFF);
}
