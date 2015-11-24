#include <common.h>
#include <compiler.h>
#include <fuse.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include "ipa9131_fuse.h"
#include "sec.h"
#include "image.h"

#define fuse_in_be32(x)	in_be32((const volatile unsigned __iomem *)(x))
#define fuse_out_be32(x,y) out_be32((volatile unsigned __iomem *)(x),(y))

#define GUTS_PMUXCR1_ADDR   (void *)0xff7e0060
#define GPIO41_DIR_ADDR     (void *)0xff70f100	
#define GPIO41_DATA_ADDR    (void *)0xff70f108

void ipa9131_fuse_enable_blowing()
{

    /*Enable gpio function of the pin*/
    setbits_be32(GUTS_PMUXCR1_ADDR,0x01000000);
    setbits_be32(GPIO41_DIR_ADDR,0x00400000);
    /*pull down gpio41 to enable fuse circuit power*/
    clrbits_be32(GPIO41_DATA_ADDR,0x00400000);
    udelay(10000);

}

void ipa9131_fuse_disable_blowing()
{
    setbits_be32(GPIO41_DATA_ADDR,0x00400000);
    clrbits_be32(GPIO41_DIR_ADDR,0x00400000);
    /*Disable gpio function of the pin*/
    clrbits_be32(GUTS_PMUXCR1_ADDR,0x01000000);
    udelay(10000);

}

int ipa9131_fuse_init(void)
{
	fuse_out_be32(SFP_INGR_ADDRESS, 0x0);
	fuse_out_be32(SFP_INGR_ADDRESS, 0x1);
	/*Let sfp initialize the shadow register*/
	udelay(10000);
	fuse_out_be32(SFP_INGR_ADDRESS, 0x0);
	return 0;
}


int ipa9131_fuse_read_eid(u8 oui_arr[3],u32 *serial)
{
	u32 r0;
	u32 r1;

	if (!oui_arr || !serial)
		return -EINVAL;

	r0 = fuse_in_be32(SFP_DCVR0_ADDRESS);
	r1 = fuse_in_be32(SFP_DCVR1_ADDRESS);

	oui_arr[0] = (r0 >> 24) & 0xff;
	oui_arr[1] = (r0 >> 16) & 0xff;
	oui_arr[2] = (r0 >> 8) & 0xff;
	*serial = r1 & 0x0fffffff;
	return 0;
}


int ipa9131_fuse_read_security_profile(u8 * p, u8 * d, u8 * s)
{
	u32 r;

	if (!p || !d || !s)
		return -EINVAL;

	r = fuse_in_be32(SFP_OUIDR_ADDRESS);

	*p = (r & 0x01) ? 1 : 0;
	*d = (r & 0x02) ? 1 : 0;
	*s = (r & 0x04) ? 1 : 0;
	return 0;
}


int ipa9131_fuses_are_write_protected(void)
{
	return (fuse_in_be32(SFP_OSPR_ADDRESS) & 0x01) ? 1 : 0;
}


int ipa9131_fuse_should_be_silent(void)
{
	if (!ipa9131_fuses_are_write_protected())
		return 0;

	return (fuse_in_be32(SFP_OUIDR_ADDRESS) & 0x01) ? 1 : 0;
}

int ipa9131_fuse_its_blown(void)
{
	return (fuse_in_be32(SFP_OSPR_ADDRESS) & 0x04) ? 1 : 0;
}


int ipa9131_fuse_read_loader_revocation_value(u16 * r)
{
	u32 r0;

	if (!r)
		return -EINVAL;

	r0 = fuse_in_be32(SFP_OUIDR_ADDRESS);
	r0 >>= 4;
	*r = r0 & 0xfff;
	return 0;
}


int ipa9131_fuse_read_application_revocation_value(u32 * r)
{
	u32 r0;
	u32 r1;
	u32 r2;

	if (!r)
		return -EINVAL;

	r0 = fuse_in_be32(SFP_OUIDR_ADDRESS);
	r1 = fuse_in_be32(SFP_DCVR0_ADDRESS);
	r2 = fuse_in_be32(SFP_DCVR1_ADDRESS);
	*r = (((r0 >> 16) & 0x0000ffff) | ((r1 << 16) & 0x00ff0000) | ((r2 >>  4) & 0x0f000000)) & 0x0fffffff;
	return 0;
}


static u32 determine_fused_bit_count(u32 N, u32 M)
{
	u32 i;

	for (i = N; i > 0; --i)
	{
		if (M & (1U << (i - 1)))
		{
			return i;
		}
	}

	return 0;
}


u16 ipa9131_fuse_read_loader_revocation(void)
{
	u16 constructed;

	if (0 != ipa9131_fuse_read_loader_revocation_value(&constructed))
		return IPA9131_LOADER_REVOCATION_MAX;

	return determine_fused_bit_count(IPA9131_LOADER_REVOCATION_MAX, constructed);
}


u32 ipa9131_fuse_read_application_revocation(void)
{
	u32 constructed;

	if (0 != ipa9131_fuse_read_application_revocation_value(&constructed))
		return IPA9131_APPLICATION_REVOCATION_MAX;

	return determine_fused_bit_count(IPA9131_APPLICATION_REVOCATION_MAX, constructed);
}


int ipa9131_is_unfused(void)
{
	if (fuse_in_be32(SFP_OUIDR_ADDRESS) ||
	    fuse_in_be32(SFP_DCVR0_ADDRESS) ||
	    fuse_in_be32(SFP_DCVR1_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR0_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR1_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR2_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR3_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR4_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR5_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR6_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR7_ADDRESS))
		return 0;

	return 1;
}

u32 ipa9131_fuse_read_fsl_uid(void)
{

    return fuse_in_be32(SFP_FUIDR_ADDRESS);

}    

int ipa9131_fuse_write_in_range(u32 start_addr, u8 num_words, const u32* val)
{
	u16 i;
	u32 curr_val = 0;

	for ( i = 0; i < num_words; i++ )
	{
		if ( ((start_addr >= SFP_DPR_ADDRESS ) && (start_addr <= SFP_OSCR_ADDRESS) ) ||
				(start_addr == SFP_OSPR_ADDRESS)
		   )
		{
			
			if ( (start_addr >= SFP_DRVR0_ADDRESS) && (start_addr <= SFP_OTPMKR7_ADDRESS) )	
			{
				/*DRVR/OTPMK Returns 0 always and are not allowed to be read, raises sfp violation*/
				curr_val = *(val + i);
			}
			else
			{
				curr_val = fuse_in_be32(start_addr);
				curr_val |= *(val+i);
			}

			fuse_out_be32(start_addr,curr_val);
			start_addr += 4;
		}
		else
		{
			fprintf(stderr, "Invalid Address %8X: out of OEM SFP register range.\n",start_addr);
			return -EINVAL;
		}
	}
	return 0;

}

void ipa9131_fuse_read_in_range(u32 start_addr, u8 num_words, u32* val)
{
	int i;

        for (i = 0; i < num_words;i++)
        {
            if ( (start_addr < SFP_DRVR0_ADDRESS) || (start_addr > SFP_OTPMKR7_ADDRESS) )
                val[i] = fuse_in_be32(start_addr);
            else 
                /*Do not try reading DRVR/OTPMK, it raises a SFP violation*/
                val[i] = 0;

            start_addr += 4;
        }

}

void ipa9131_blow_fuse(void)
{

	ipa9131_fuse_enable_blowing();

	fuse_out_be32(SFP_INGR_ADDRESS,0x2);
	/*Give some time to sfp to blow fuses*/
	udelay(100000);
	/*Reset INGR to 0*/	
	fuse_out_be32(SFP_INGR_ADDRESS, 0x0);

	ipa9131_fuse_disable_blowing();
}

int ipa9131_read_provisioning_status(u8 *otpmk_set,u8 *dbg_resp_set, u8 *apk_created )
{
	u32 r;

	if (!otpmk_set || !dbg_resp_set || !apk_created)
		return -EINVAL;

	r = fuse_in_be32(SFP_OSCR_ADDRESS);

	*otpmk_set = (r & 0x01) ? 1 : 0;
	*dbg_resp_set = (r & 0x02) ? 1 : 0;
	*apk_created = (r & 0x04) ? 1 : 0;
	return 0;


}

#if defined(CONFIG_CMD_IPA9131_FUSE)
int do_ipa9131_fuse(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	if (0 == strcmp(argv[1], "init")) {
		if (0 != ipa9131_fuse_init()) {
			return CMD_RET_FAILURE;
		}

		printf("init OK\n");
		return CMD_RET_SUCCESS;
	}

	if (0 == strcmp(argv[1], "eid")) {
		u8 oui[3];
		u32 serial;

		if (0 != ipa9131_fuse_read_eid(oui,&serial)) {
			return CMD_RET_FAILURE;
		}

		printf("EID: %02X%02X%02X-%010u\n", oui[0], oui[1], oui[2], serial);
		return CMD_RET_SUCCESS;
	}

	if (0 == strcmp(argv[1], "security")) {
		u8 p;
		u8 d;
		u8 s;

		if (0 != ipa9131_fuse_read_security_profile(&p, &d, &s)) {
			return CMD_RET_FAILURE;
		}

		printf("security profile: prod <%u>, dev <%u>, special <%u>\n", p, d, s);
		return CMD_RET_SUCCESS;
	}

	if (0 == strcmp(argv[1], "ldr_revo")) {
		printf("loader revocation: %u\n", ipa9131_fuse_read_loader_revocation());
		return CMD_RET_SUCCESS;
	}

	if (0 == strcmp(argv[1], "app_revo")) {
		printf("application revocation: %u\n", ipa9131_fuse_read_application_revocation());
		return CMD_RET_SUCCESS;
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(ipa9131_fuse, 2, 0, do_ipa9131_fuse,
	"Exercise ipa9131 fuse functions",
	" init|eid|security|ldr_revo|app_revo"
);
#endif

#if defined(CONFIG_CMD_IPA9131_GO_SECURE)

static int do_ipa9131_secure(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

    u32 secmon_hpsr,sfp_dessr;
    u32 optmk_minimal = IPA9131_MINIMAL_OTPMK_VALUE;
    u32 debug_resp_minimal = IPA9131_MINIMAL_JTAG_RESP_VALUE;
    u32 its_csff_value = 0x06, debug_prmsn_val = 0x2;
    int prompt_to_user = 1, i = 1;
    int fuse_dpr = 1;

    while( i < argc )
    {
        if ( 0 == strcmp(argv[i],"-no-prompt") )
            prompt_to_user = 0;
        else if ( 0 == strcmp(argv[i],"-full-dbg-permission") )
            fuse_dpr = 0;
	else
            return CMD_RET_USAGE; 

        i++;

    }


    ipa9131_fuse_init();

    if (0 != do_ipa9131_sec_boot_verify(NULL,0,0,NULL))
        goto error;

    if ( ipa9131_is_unfused() )
    {
        fprintf(stderr,"Board not characterised, fuse characterisation data first\n");
        goto error;
    }

    if (0 != verify_secboot_images_present())
    {
        fprintf(stderr,"Valid secure boot images not present on flash\n");
        goto error;
    }



    if ( (0 != ipa9131_fuse_write_in_range(SFP_OTPMKR0_ADDRESS,1,&optmk_minimal)) || 
            (0 != ipa9131_fuse_write_in_range(SFP_DRVR1_ADDRESS,1,&debug_resp_minimal)) )
    {
        fprintf(stderr,"Error in setting fuse values\n");
        goto error;
    }

    udelay(10000);

    secmon_hpsr = sec_in_be32(SECMON_HPSR);

    /*Sec mon hpsr almost immediately reflects error if OTPMK is not hamming protected*/
    if (secmon_hpsr & 0x09FF0000)
    {
        fprintf(stderr,"Wrong value in OTPMK0, can't blow fuses\n");
        goto error;
    }

    if ( prompt_to_user )
    {
        fprintf(stdout,"Do you wish to go ahead and make this as secure boot board? this will permanently blow fuses\n");
        fprintf(stdout, "Type 'Y' to proceed: ");


        if ('Y' != getc())
        {
            fprintf(stdout, "\nUser cancelled operation\n");
            goto error;
        }

        fprintf(stdout,"\nYou're sure? ");
        fprintf(stdout, "Type 'Y' to proceed: ");

        if ('Y' != getc())
        {
            fprintf(stdout,"\nUser cancelled operation\n");
            goto error;
        }

        fprintf(stdout,"\nAttempting to write the fuses\n");
    }

    ipa9131_blow_fuse();

    ipa9131_fuse_init();/*reinit this clears PROGFB bit of SFP_INGR, so that fuses can be blown again*/

    secmon_hpsr = sec_in_be32(SECMON_HPSR);
    sfp_dessr = fuse_in_be32(SFP_DESSR_ADDRESS);

    if ( (secmon_hpsr & 0x09FF0000) || (sfp_dessr & 0x0000007E))
    {
        fprintf(stderr,"OTPMK0/DRV1 fuses not blown properly, can't blow ITS and DBG permission fuses\n");        
        goto error;
    }

    if (0 != ipa9131_fuse_write_in_range(SFP_OSPR_ADDRESS,1,&its_csff_value))
    {
        fprintf(stderr,"Error setting ITS fuse\n");
        goto error;
    }

    if (fuse_dpr)
    {

        if (0 != ipa9131_fuse_write_in_range(SFP_DPR_ADDRESS,1,&debug_prmsn_val))
        {
            fprintf(stderr,"Error setting jtag dbg permission fuse values\n");
            goto error;
        }


    }


    ipa9131_blow_fuse();

    fprintf(stdout,"fuses blown successfully, Board is now a Secure Boot Board\n");

    return CMD_RET_SUCCESS;

error:
    return CMD_RET_FAILURE;

}

U_BOOT_CMD(ipa9131_go_secure, 3, 0, do_ipa9131_secure,
        "Prepare Board for secure boot",
        "ipa9131_go_secure <-no-prompt|-full-dbg-permission> "
        );

#endif

#if defined CONFIG_CMD_IPA9131_VERIFY_SEC_BOOT_CHIP
int do_ipa9131_sec_boot_verify(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
        u32 val = 0;

        ipa9131_fuse_init();
        val = fuse_in_be32(SFP_OSPR_ADDRESS);
        if (val & 0x00000008)
        {
            fprintf(stderr,"Sec_disabled: Secure boot not possible on this chip\n");
            goto error;
        }

        val = fuse_in_be32(SFP_FSWPR_ADDRESS);
        if (0x01 != (val & 0x00000003) )
        {
            fprintf(stderr,"SFP_FSWPR invalid: Secure boot not possible on this chip\n");
            goto error;

        }

        val = fuse_in_be32(SFP_ONSEC_ADDRESS);
        if (val & 0x00000001)
        {
            fprintf(stderr,"SFP_ONSEC Sec_disabled: Secure boot not possible on this chip\n");
            goto error;

        }

        val = sec_in_be32(SECMON_HPSVSR);
        if ( val & 0x09FF001E)
        {
            fprintf(stderr,"SECMON_HPSVSR Sec_violations: Secure boot not possible on this chip\n");
            goto error;

        }

        fprintf(stdout,"Secure Boot compatible chip\n");
        return CMD_RET_SUCCESS;

error:
        return CMD_RET_FAILURE;

}

U_BOOT_CMD(ipa9131_verify_sec_boot_chip, 1, 0, do_ipa9131_sec_boot_verify,
        "Verify board is meant for secure boot",
        "ipa9131_verify_sec_boot_chip"
        );
#endif
