#include <common.h>
#include <asm/setup.h>
#include <malloc.h>

#define ATAG_MSM_PARTITION 0x4d534D70
#define NUM_NAND_PARTITIONS 18

struct msm_ptbl_entry {
    char name[16];
    u32 offset;
    u32 size;
    u32 flags;
};

struct msm_partition_table {
    struct tag_header hdr;
    struct msm_ptbl_entry entries[NUM_NAND_PARTITIONS];
};

/*
 * The size of the structure is... strange...
 *
 * The code in the kernel that reads this is as follows:
 *         count = (tag->hdr.size - 2) /
 *               (sizeof(struct msm_ptbl_entry) / sizeof(__u32));
 * So, we're setting 128, sizeof(struct msm_ptbl_entry) is 28
 * and sizeof(__u32) is 4, so this becomes:
 *  count = (128 - 2) / (28 / 4)
 * or...
 *  count = 126 / 7
 * or...
 *  count = 18
 * ... which is the number of elements!
 *
 * Interestingly, the "right" way to do this is to take
 * the size of the structure and shift it right by two,
 * which yields the correct calculation.
 *
 * The maximum number of NAND partitions we can pass to the
 * kernel is 18 - that's as many as it will read.
 */
static const struct msm_partition_table msm_ptable = {
    { sizeof(struct msm_partition_table) >> 2, ATAG_MSM_PARTITION },
    {
        { "APPS2EFS\0\0\0\0\0\0\0\0",        608, 1440, 0x0 },
        { "APPS_CAL\0\0\0\0\0\0\0\0",         59,   16, 0x0 },
        { "FULL_NAND\0\0\0\0\0\0\0",           0, 2048, 0x0 },
        { "DBL\0\0\0\0\0\0\0\0\0\0\0\0\0",     0,    1, 0x0 },
        { "BBT\0\0\0\0\0\0\0\0\0\0\0\0\0",     1,    1, 0x0 },
        { "PARTITION\0\0\0\0\0\0\0",           2,    2, 0x0 },
        { "SIM_SCURE\0\0\0\0\0\0\0",           4,    4, 0x0 },
        { "EFS\0\0\0\0\0\0\0\0\0\0\0\0\0",     8,   51, 0x0 },
        { "ODM\0\0\0\0\0\0\0\0\0\0\0\0\0",    75,   53, 0x0 },
        { "SBL0\0\0\0\0\0\0\0\0\0\0\0\0",    128,   16, 0x0 },
        { "ABL0\0\0\0\0\0\0\0\0\0\0\0\0",    144,   16, 0x0 },
        { "UBOOT0\0\0\0\0\0\0\0\0\0\0",      160,   32, 0x0 },
        { "SBL1\0\0\0\0\0\0\0\0\0\0\0\0",    192,   16, 0x0 },
        { "ABL1\0\0\0\0\0\0\0\0\0\0\0\0",    208,   16, 0x0 },
        { "UBOOT1\0\0\0\0\0\0\0\0\0\0",      224,   32, 0x0 },
        { "AMSS\0\0\0\0\0\0\0\0\0\0\0\0",    256,  256, 0x0 },
        { "HEXAGON\0\0\0\0\0\0\0\0\0",       512,   96, 0x0 },
        { "FS\0\0\0\0\0\0\0\0\0\0\0\0\0\0",  608, 1440, 0x0 }
    }
};


void setup_board_tags(struct tag **in_params)
{
    struct tag *params;
    params = *in_params;
    memcpy(params, &msm_ptable, sizeof(msm_ptable));
    params = tag_next(params);
    *in_params = params;
}
