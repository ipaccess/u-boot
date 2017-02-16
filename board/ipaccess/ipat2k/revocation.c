#include <revocation.h>
#include <common.h>
#include "ipat2k_fuse.h"




u64 get_board_revocation(void)
{
    return ( u64 ) ipat2k_fuse_read_application_revocation();
}

