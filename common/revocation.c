#include <revocation.h>
#include <common.h>


u64 __get_board_revocation(void) { return 0; }
u64 get_board_revocation(void) __attribute__((weak, alias("__get_board_revocation")));
