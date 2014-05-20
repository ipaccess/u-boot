/* linux/arch/arm/mach-msm/dma.c
 *
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2008-2010, 2012, 2013 The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "asm/io.h"
#include "k_dma.h"
#include "linux/list.h"
#include "common.h"
#include "k_dma_port.h"
#include "linux/compat.h"
#include "asm-generic/errno.h"

#define MODULE_NAME "msm_dmov"

#define MSM_DMOV_CHANNEL_COUNT 16
#define MSM_DMOV_CRCI_COUNT 16

enum {
	CLK_DIS,
	CLK_ENABLING,
	CLK_EN,
	CLK_TO_BE_DIS,
	CLK_DISABLING
};

struct msm_dmov_ci_conf {
	int start;
	int end;
	int burst;
};

struct msm_dmov_crci_conf {
	int sd;
	int blk_size;
};

struct msm_dmov_chan_conf {
	int sd;
	int block;
	int priority;
};

struct msm_dmov_conf {
	void *base;
	//struct msm_dmov_crci_conf *crci_conf;
	//struct msm_dmov_chan_conf *chan_conf;
	int channel_active;
	int sd;
	size_t sd_size;
	struct list_head staged_commands[MSM_DMOV_CHANNEL_COUNT];
	struct list_head ready_commands[MSM_DMOV_CHANNEL_COUNT];
	struct list_head active_commands[MSM_DMOV_CHANNEL_COUNT];
	//struct mutex clock_lock;
	//spinlock_t lock;
	unsigned int irq;
	struct clk *clk;
	struct clk *pclk;
	struct clk *ebiclk;
	unsigned int clk_ctl;
	//struct delayed_work work;
	struct workqueue_struct *cmd_wq;
};

static struct msm_dmov_conf dmov_conf[] = {
	{
//		.crci_conf = NULL,
//		.chan_conf = NULL,
//		.clock_lock = __MUTEX_INITIALIZER(dmov_conf[0].clock_lock),
//		.lock = __SPIN_LOCK_UNLOCKED(dmov_lock),
		.clk_ctl = CLK_DIS,
//		.work = __DELAYED_WORK_INITIALIZER(dmov_conf[0].work,
//				msm_dmov_clock_work),
	}
};

#define MSM_DMOV_ID_COUNT (MSM_DMOV_CHANNEL_COUNT * ARRAY_SIZE(dmov_conf))
#define DMOV_REG(name, adm)    ((name) + (dmov_conf[adm].base) +\
	(dmov_conf[adm].sd * dmov_conf[adm].sd_size))
#define DMOV_ID_TO_ADM(id)   ((id) / MSM_DMOV_CHANNEL_COUNT)
#define DMOV_ID_TO_CHAN(id)   ((id) % MSM_DMOV_CHANNEL_COUNT)
#define DMOV_CHAN_ADM_TO_ID(ch, adm) ((ch) + (adm) * MSM_DMOV_CHANNEL_COUNT)

#ifdef CONFIG_MSM_ADM3
#define DMOV_IRQ_TO_ADM(irq)   \
({ \
	typeof(irq) _irq = irq; \
	((_irq == INT_ADM1_MASTER) || (_irq == INT_ADM1_AARM)); \
})
#else
#define DMOV_IRQ_TO_ADM(irq) 0
#endif

enum {
	MSM_DMOV_PRINT_ERRORS = 1,
	MSM_DMOV_PRINT_IO = 2,
	MSM_DMOV_PRINT_FLOW = 4
};

unsigned int msm_dmov_print_mask = MSM_DMOV_PRINT_ERRORS | MSM_DMOV_PRINT_IO | MSM_DMOV_PRINT_FLOW;

#define MSM_DMOV_DPRINTF(mask, format, args...) \
	do { \
		if ((mask) & msm_dmov_print_mask) \
			printk(KERN_ERR format, args); \
	} while (0)
#define PRINT_ERROR(format, args...) \
	MSM_DMOV_DPRINTF(MSM_DMOV_PRINT_ERRORS, format, args);
#define PRINT_IO(format, args...) \
	MSM_DMOV_DPRINTF(MSM_DMOV_PRINT_IO, format, args);
#define PRINT_FLOW(format, args...) \
	MSM_DMOV_DPRINTF(MSM_DMOV_PRINT_FLOW, format, args);


enum {
	NOFLUSH = 0,
	GRACEFUL,
	NONGRACEFUL,
};

struct msm_dmov_exec_cmdptr_cmd {
	struct msm_dmov_cmd dmov_cmd;
//	struct completion complete;
	unsigned id;
	unsigned int result;
	struct msm_dmov_errdata err;
};

void print_flush_regs(void)
{
    uint32_t flush[6];
    int i;

    //Perform a flush
    writel_relaxed(0, DMOV_REG(DMOV_FLUSH0(DMOV_NAND_CHAN), 0));

    flush[0] = readl_relaxed(DMOV_REG(DMOV_FLUSH0(DMOV_NAND_CHAN), 0));
    flush[1] = readl_relaxed(DMOV_REG(DMOV_FLUSH1(DMOV_NAND_CHAN), 0));
    flush[2] = 0;
    flush[3] = readl_relaxed(DMOV_REG(DMOV_FLUSH3(DMOV_NAND_CHAN), 0));
    flush[4] = readl_relaxed(DMOV_REG(DMOV_FLUSH4(DMOV_NAND_CHAN), 0));
    flush[5] = readl_relaxed(DMOV_REG(DMOV_FLUSH5(DMOV_NAND_CHAN), 0));

    printf("flush regs: ");
    for(i=0; i<6; i++)
        printf("%d:0x%08x ",i,flush[i]);
    printf("\n");
}

void msm_dmov_enqueue_cmd_ext_atomic(unsigned id, struct msm_dmov_cmd *cmd)
{
    unsigned int status = 0;
    unsigned int try_count = 0;
    int adm = DMOV_ID_TO_ADM(id);
    int ch = DMOV_ID_TO_CHAN(id);
    unsigned int ch_result = 0;
    struct msm_dmov_exec_cmdptr_cmd *exec_cmd =
        container_of(cmd, struct msm_dmov_exec_cmdptr_cmd, dmov_cmd);

    //printf("%s cmdptr:0x%x\n",__func__,cmd->cmdptr);
    // Wait for space on the command queue
    try_count = 10000;
    while( (0 == (status & DMOV_STATUS_CMD_PTR_RDY)) && try_count )
    {
        status = readl_relaxed(DMOV_REG(DMOV_STATUS(ch), adm));
        //printf("%s try:%d status:0x%x\n",__func__,try_count,status);
        try_count--;
    }
    if(!try_count)
    {
        printf("%s Error full command queue\n",__func__);
        return;
    }
    //printf("%s id:%d start:%x exec_func:%p\n",__func__,id,status,cmd->exec_func);

    if (cmd->exec_func)
        cmd->exec_func(cmd);
    dmov_conf[adm].channel_active |= BIT(ch);
    writel_relaxed(cmd->cmdptr, DMOV_REG(DMOV_CMD_PTR(ch), adm));

    try_count = 10000;
    // Wait for status valid flag to be set.
    do
    {
        if(0 == try_count)
        {
            // Something is wrong, flush and print flush registers.
            printf("%s Error command not completing\n",__func__);
            print_flush_regs();
            return;
        }
        status = readl_relaxed(DMOV_REG(DMOV_STATUS(ch), adm));
        //printf("%s status:0x%08x\n",__func__,status);
        --try_count;
    }while(0 == (status & DMOV_STATUS_RSLT_VALID));

    // Read status, loop reading result until status result count is zero.
    // As this driver is now polled we should only ever have one result in the queue.
    status = readl_relaxed(DMOV_REG(DMOV_STATUS(ch), adm));
    try_count = 10000;
    while ( (DMOV_STATUS_RSLT_COUNT(status)) && try_count)
    {
        ch_result = readl_relaxed(DMOV_REG(DMOV_RSLT(ch), adm));
        status = readl_relaxed(DMOV_REG(DMOV_STATUS(ch), adm));
        try_count--;
    }

    // Set the result instead of calling complete_func() as done by ISR.
    exec_cmd->result = ch_result;
}

static void __msm_dmov_enqueue_cmd_ext(unsigned id, struct msm_dmov_cmd *cmd,
					int onstack)
{
	//int adm = DMOV_ID_TO_ADM(id);
	//int ch = DMOV_ID_TO_CHAN(id);

    //printf("%s cmd id:%d\n",__func__,id);
	cmd->id = id;
	cmd->toflush = 0;
    //printf("%s clk_ctl:%d\n",__func__,dmov_conf[adm].clk_ctl);

    msm_dmov_enqueue_cmd_ext_atomic(id, cmd);
}

static void
dmov_exec_cmdptr_complete_func(struct msm_dmov_cmd *_cmd,
			       unsigned int result,
			       struct msm_dmov_errdata *err)
{
	struct msm_dmov_exec_cmdptr_cmd *cmd =
		container_of(_cmd, struct msm_dmov_exec_cmdptr_cmd, dmov_cmd);
    printf("%s\n",__func__);
	cmd->result = result;
	if (result != 0x80000002 && err)
		memcpy(&cmd->err, err, sizeof(struct msm_dmov_errdata));
}

int msm_dmov_exec_cmd(unsigned id, unsigned int cmdptr)
{
	struct msm_dmov_exec_cmdptr_cmd cmd;

	//PRINT_FLOW("%s (%d, %x)\n",__func__, id, cmdptr);

	cmd.dmov_cmd.cmdptr = cmdptr;
	cmd.dmov_cmd.complete_func = dmov_exec_cmdptr_complete_func;
	cmd.dmov_cmd.exec_func = NULL;
	cmd.id = id;
	cmd.result = 0;

	__msm_dmov_enqueue_cmd_ext(id, &cmd.dmov_cmd, 1);

	if (cmd.result != 0x80000002) {
		PRINT_ERROR("%s (%d): ERROR, result: %x\n",__func__,
			id, cmd.result);
		PRINT_ERROR("%s (%d):  flush: %x %x %x %x\n",__func__,
			id, cmd.err.flush[0], cmd.err.flush[1],
			cmd.err.flush[2], cmd.err.flush[3]);
		return -EIO;
	}
	//PRINT_FLOW("%s (%d, %x) done\n",__func__, id, cmdptr);
	return 0;
}


static void config_datamover(int adm)
{
    unsigned conf;
    unsigned r;

    // Result must be produced on flush completion.
    // No interrupt on result
    // Forcing a result when last command completes.
    writel_relaxed(DMOV_RSLT_CONF_FORCE_FLUSH_RSLT | DMOV_RSLT_CONF_FORCE_TOP_PTR_RSLT,
                   DMOV_REG(DMOV_RSLT_CONF(DMOV_NAND_CHAN), adm));

    // Configure the NAND channel
    //conf = readl_relaxed(DMOV_REG(DMOV_CONF(DMOV_NAND_CHAN), adm));
    //conf &= ~DMOV_CONF_IRQ_EN;
    conf  = DMOV_CONF_FORCE_RSLT_EN;  //Doesn't help read issue.
    //conf |= DMOV_CONF_MPU_DISABLE;    //Ditto
    debug("%s dmov nand chan config reg 0x%x\n",__func__,conf);
    writel_relaxed(conf,DMOV_REG(DMOV_CONF(DMOV_NAND_CHAN), adm));

    r = readl_relaxed(DMOV_REG(DMOV_CRCI_CONF0,adm));
    debug("%s DMOV_CRCI_CONF0/HI0_CRCI_DBG 0x%x\n",__func__,r);
}


int msm_dmov_probe(void)
{
    int adm = 0;

    debug("%s\n",__func__);
    dmov_conf[adm].sd           =  3; // Security Domain
    dmov_conf[adm].sd_size      = 0x400;
    dmov_conf[adm].base         = (void*)0x94610000;

    config_datamover(adm);

    return 0;
}

