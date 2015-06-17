/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>
#include <malloc.h>
#include "desc_constr.h"
#include "jobdesc.h"
#include "jr.h"
#include "desc.h"

static void inline_cnstr_jobdesc_rng_gen(uint32_t *desc,uint32_t rn_size, uint8_t *rn)
{
    dma_addr_t dma_addr_out;

    dma_addr_out = virt_to_phys((void *)rn);

    init_job_desc(desc, 0);
    /*LOAD Class 1 data size register with number of random bytes to be generated*/
    append_load_imm_u32(desc, rn_size, LDST_SRCDST_WORD_DATASZ_REG|CLASS_1);

    /*OPERATION command to generate random number*/
    append_operation(desc, OP_TYPE_CLASS1_ALG|OP_ALG_ALGSEL_RNG);

    /* Get data from the Output Fifo to our pointer */
    append_fifo_store(desc, dma_addr_out, rn_size, FIFOST_TYPE_MESSAGE_DATA);
    
}


int sec_generate_random_number(uint32_t len, uint8_t *dst_addr)
{
    int ret, i = 0;
    u32 *desc;

    desc = malloc(sizeof(int) * MAX_CAAM_DESCSIZE);
    if (!desc) {
        debug("Not enough memory for descriptor allocation\n");
        return -1;
    }

    inline_cnstr_jobdesc_rng_gen(desc,len,dst_addr);

    ret = run_descriptor_jr(desc);

    if (ret)
        printf("Error in Random number generation %d\n", ret);

    free(desc);
    return ret;
}


#if defined(CONFIG_CMD_IPA_RNG)
int do_rng(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    u8 *dst_addr;
    u32 len;
    
    if (argc != 3)
        return CMD_RET_USAGE;

    dst_addr = (u8 *)simple_strtoul(argv[1], NULL, 16);
    len = (u32) simple_strtoul(argv[2], NULL, 16);

    return sec_generate_random_number(len,dst_addr);
}

U_BOOT_CMD(ipa_rng,3,0,do_rng,
          "Excercise rng functions",
          "<dst_addr(hex_string)> <rand_num size in bytes(hex_string)>");
#endif


