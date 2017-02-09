/* 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * T3300 helper function
 */

#include <common.h>
#include <command.h>

#define mdelay(n)   udelay((n)*1000)

DECLARE_GLOBAL_DATA_PTR;

int create_linux_tags(char *commandline);

#if defined (CONFIG_SETUP_MEMORY_TAGS) || \
    defined (CONFIG_CMDLINE_TAG) || \
    defined (CONFIG_INITRD_TAG) || \
    defined (CONFIG_SERIAL_TAG) || \
    defined (CONFIG_REVISION_TAG) || \
    defined (CONFIG_VFD) || \
    defined (CONFIG_LCD)
static void setup_start_tag (bd_t *bd);

# ifdef CONFIG_SETUP_MEMORY_TAGS
extern void setup_memory_tags (bd_t *bd);
# endif
extern void setup_commandline_tag (bd_t *bd, char *commandline);

# ifdef CONFIG_INITRD_TAG
extern void setup_initrd_tag (bd_t *bd, ulong initrd_start,
                              ulong initrd_end);
# endif
extern void setup_end_tag (bd_t *bd);

# if defined (CONFIG_VFD) || defined (CONFIG_LCD)
extern void setup_videolfb_tag (gd_t *gd);
# endif

static struct tag *params;
#endif /* CONFIG_SETUP_MEMORY_TAGS || CONFIG_CMDLINE_TAG || CONFIG_INITRD_TAG */


int do_t3300 (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	
	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			if (strncmp(argv[i], "slave-bootargs", 14) == 0) {
				create_linux_tags(getenv("bootargs_slave"));
				break;
			} else if (strncmp(argv[i], "slave-reset", 11) == 0) {
				printf("Resetting slave...");
				REG32(GPIO_OE_REG) |= GPIO_9;
				REG32(GPIO_OUTPUT_REG) &= ~GPIO_9;
				printf(" GPIO_9 = 0;");
				mdelay(200);
				REG32(GPIO_OUTPUT_REG) |= GPIO_9;
				printf(" GPIO_9 = 1;");
				printf(" done.\n");
				break;
			} else {
				printf("Unknown argument: '%s'\n", argv[i]);
				goto usage;
			}
		}
	} else {
		printf("Wrong number of arguments\n");
		goto usage;	
	}


	return 0;
usage:
	cmd_usage(cmdtp);
	return 1;
}

int create_linux_tags(char *commandline)
{
        bd_t    *bd = gd->bd;
        char    *s;
        int     machid = bd->bi_arch_number;

        s = getenv ("machid");
        if (s) {
                machid = simple_strtoul (s, NULL, 16);
                printf ("Using machid 0x%x from environment\n", machid);
        }

#if defined (CONFIG_SETUP_MEMORY_TAGS) || \
    defined (CONFIG_CMDLINE_TAG) || \
    defined (CONFIG_INITRD_TAG) || \
    defined (CONFIG_SERIAL_TAG) || \
    defined (CONFIG_REVISION_TAG) || \
    defined (CONFIG_LCD) || \
    defined (CONFIG_VFD)

        setup_start_tag (bd);
#ifdef CONFIG_SERIAL_TAG
        setup_serial_tag (&params);
#endif
#ifdef CONFIG_REVISION_TAG
        setup_revision_tag (&params);
#endif
#ifdef CONFIG_SETUP_MEMORY_TAGS
        setup_memory_tags (bd);
#endif
#ifdef CONFIG_CMDLINE_TAG
        setup_commandline_tag (bd, commandline);
#endif
#if defined (CONFIG_VFD) || defined (CONFIG_LCD)
        setup_videolfb_tag ((gd_t *) gd);
#endif
        setup_end_tag (bd);
#endif

        /* we assume that the kernel is in place */
        printf ("\nMachid should be:\t0x%x\n"
		"Tags are placed at:\t0x%x\n"
		"Following bootargs has been converted to tags: '%s'\n",
		machid,
		bd->bi_boot_params,
		commandline);

	return 0;
}



/***************************************************/


U_BOOT_CMD(
	t3300,	2,	1,	do_t3300,
	"T3300 helper function",
	" [operation]\n"
	" operations:\n"
	"\tslave-bootargs - creates tags fron bootargs.\n"
	"\tslave-reset - resets slave cluster.\n"
);
