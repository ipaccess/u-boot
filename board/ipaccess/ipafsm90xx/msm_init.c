/* Copyright (c) 2013-2016, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "debug.h"
#include "iomap.h"
#include "irqs.h"
#include "partition_parser.h"
#include "lk_sdhci_msm.h"
#include <common.h>


#if MMC_SDHCI_SUPPORT
struct mmc_device *dev;


static uint32_t mmc_sdhci_base[] =
	{ MSM_SDC1_SDHCI_BASE };
static uint32_t mmc_sdc_pwrctl_irq[] =
	{ SDCC1_PWRCTL_IRQ };

static uint32_t mmc_sdc_base[] =
	{ MSM_SDC1_BASE };


void target_mmc_sdhci_init()
{
	static uint32_t mmc_clks[] = {MMC_CLK_200MHZ, MMC_CLK_96MHZ, MMC_CLK_50MHZ };

	struct mmc_config_data config;
	unsigned int i;
    uint8_t *buff=NULL;

	memset(&config, 0, sizeof config);
	config.bus_width = DATA_BUS_WIDTH_8BIT;

	/* Trying Slot 1*/
	config.slot = 1;
	config.sdhc_base = mmc_sdhci_base[config.slot - 1];
	config.pwrctl_base = mmc_sdc_base[config.slot - 1];
	config.pwr_irq     = mmc_sdc_pwrctl_irq[config.slot - 1];
	config.hs400_support = 0;

	for (i = 0; i < ARRAY_SIZE(mmc_clks); ++i) {
		config.max_clk_rate = mmc_clks[i];
		dprintf(INFO, "SDHC Running at %u MHz\n",
			config.max_clk_rate / 1000000);
		dev = lk_mmc_init(&config);
		if (dev /*&& partition_read_table() == 0*/)
        {
            lk_add_sdhci(&dev->host,200000000,50000000);
			return;
        }
	}

	if (dev == NULL)
		dprintf(CRITICAL, "mmc init failed!");
	else
		dprintf(CRITICAL, "Error reading the partition table info\n");
}

void *target_mmc_device()
{
	return (void *) dev;
}

#endif


void target_uninit(void)
{
#if MMC_SDHCI_SUPPORT
        mmc_put_card_to_sleep(dev);
        sdhci_mode_disable(&dev->host);
#else
        mmc_put_card_to_sleep();
#endif
        timer_deinit();
}
