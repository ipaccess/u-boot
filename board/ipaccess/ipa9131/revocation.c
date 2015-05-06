#include <revocation.h>
#include <common.h>
#include "ipa9131_fuse.h"




u64 get_board_revocation(void)
{
    return ipa9131_fuse_read_application_revocation();
}
