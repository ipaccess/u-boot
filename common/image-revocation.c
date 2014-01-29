#include <image.h>
#include <common.h>
#include <revocation.h>




int fit_config_check_revocation(const void * fit, int conf_noffset)
{
	u64 fit_revocation;
	u64 board_revocation;

	board_revocation = get_board_revocation();

	if (0 != fit_get_revocation(fit, conf_noffset, &fit_revocation)) {
		debug("%s: failed to get FIT revocation, using 0 as "
			"a default value.\n", __func__);
		fit_revocation = 0;
	}

	if (fit_revocation < board_revocation) {
		debug("%s: FIT image (revocation: %llu) has been revoked "
			"and cannot run on board (revocation: %llu)",
			__func__, fit_revocation, board_revocation);
		return -1;
	}

	return 0;
}
