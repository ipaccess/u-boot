#include "key.h"

#include <common.h>
#include <vsprintf.h>
#include <image.h>
#include <asm-generic/errno.h>


DECLARE_GLOBAL_DATA_PTR;




int require_key(const char * name, const char * type)
{
    void * fdt_blob;
    int sig_offset;
    char key_path[256];
    const char * required;

    if (!(fdt_blob = (void *)gd_fdt_blob()))
    {
        debug("%s: no valid OF_CONTROL tree present\n", __func__);
        return CMD_RET_FAILURE;
    }

    if (0 != fdt_check_header(fdt_blob))
    {
        debug("%s: invalid OF_CONTROL tree\n", __func__);
        return CMD_RET_FAILURE;
    }

    if (!name || !type)
    {
        return -EINVAL;
    }

    if (0 != strcmp("image", type) && 0 != strcmp("conf", type))
    {
        debug("%s: invalid requirement type '%s', must be one of 'image' or 'conf'\n", __func__, type);
        return -EINVAL;
    }

    snprintf(key_path, sizeof(key_path), "/" FIT_SIG_NODENAME "/key-%s", name);
    sig_offset = fdt_path_offset(fdt_blob, key_path);

    if ((required = (const char *)fdt_getprop(fdt_blob, sig_offset, "required", NULL)))
    {
        if (0 == strcmp(required, type))
        {
            debug("%s: no property value change required\n", __func__);
            return CMD_RET_SUCCESS;
        }

        if (0 != fdt_delprop(fdt_blob, sig_offset, "required"))
        {
            debug("%s: failed to delete existing property value\n", __func__);
            return CMD_RET_FAILURE;
        }
    }

    if (0 != fdt_setprop_string(fdt_blob, sig_offset, "required", type))
    {
        debug("%s: failed to set the 'required' property on node '%s' to '%s'\n", __func__, key_path, type);
        return CMD_RET_FAILURE;
    }

    return 0;
}



int unrequire_key(const char * name)
{
    void * fdt_blob;
    int sig_offset;
    char key_path[256];
    const char * required;

    if (!(fdt_blob = (void *)gd_fdt_blob()))
    {
        debug("%s: no valid OF_CONTROL tree present\n", __func__);
        return CMD_RET_FAILURE;
    }

    if (0 != fdt_check_header(fdt_blob))
    {
        debug("%s: invalid OF_CONTROL tree\n", __func__);
        return CMD_RET_FAILURE;
    }

    if (!name || !*name)
    {
        return -EINVAL;
    }

    snprintf(key_path, sizeof(key_path), "/" FIT_SIG_NODENAME "/key-%s", name);
    sig_offset = fdt_path_offset(fdt_blob, key_path);

    if ((required = (const char *)fdt_getprop(fdt_blob, sig_offset, "required", NULL)))
    {
        if (0 != fdt_delprop(fdt_blob, sig_offset, "required"))
        {
            debug("%s: failed to delete existing property value\n", __func__);
            return CMD_RET_FAILURE;
        }
    }

    return 0;
}




#if defined(CONFIG_CMD_KEY)
static int do_key(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    const char * keyname;
    u8 action;
    int sig_offset;

    if (0 != fdt_check_header(gd_fdt_blob()))
    {
        debug("%s: no valid FDT loaded\n", __func__);
        return CMD_RET_FAILURE;
    }

    sig_offset = fdt_path_offset(gd_fdt_blob(), "/signature");

    if (sig_offset < 0 && sig_offset != -FDT_ERR_NOTFOUND)
    {
        return CMD_RET_FAILURE;
    }

    if (argc == 2 && 0 == strcmp(argv[1], "list"))
    {
        int subnode_offet = fdt_first_subnode(gd_fdt_blob(), sig_offset);

        if (-FDT_ERR_NOTFOUND == subnode_offet)
        {
            printf("%s\n", "(no keys found)");
            return CMD_RET_SUCCESS;
        }

        printf("%s\n", "+----------+--------------------------------------------------------+");
        printf("%s\n", "| Required | Key Name                                               |");
        printf("%s\n", "+----------+--------------------------------------------------------+");

        while (-FDT_ERR_NOTFOUND != subnode_offet)
        {
            int len;
            const char * key_name_hint;

            if (NULL != (key_name_hint = (const char *)fdt_getprop(gd_fdt_blob(), subnode_offet, "key-name-hint", &len)))
            {
                const char * required = (const char *)fdt_getprop(gd_fdt_blob(), subnode_offet, "required", &len);
                printf("| %-5s    | %-54s |\n", (required ? required : "No"), key_name_hint);
                printf("%s\n", "+----------+--------------------------------------------------------+");
            }

            subnode_offet = fdt_next_subnode(gd_fdt_blob(), subnode_offet);
        }

        return CMD_RET_SUCCESS;
    }

    if (argc == 3)
    {
        if (0 != strcmp(argv[1], "unrequire"))
        {
            return CMD_RET_USAGE;
        }

        if (0 != unrequire_key(argv[2]))
        {
            return CMD_RET_FAILURE;
        }

        return CMD_RET_SUCCESS;
    }

    if (argc != 4)
    {
        return CMD_RET_USAGE;
    }

    if (0 != strcmp(argv[1], "require"))
    {
        return CMD_RET_USAGE;
    }

    if (0 != require_key(argv[2], argv[3]))
    {
        return CMD_RET_FAILURE;
    }

    return CMD_RET_SUCCESS;
}


U_BOOT_CMD(
    key, 4, 2, do_key,
    "list, require or unrequire security keys",
    "list -  list keys and their required flag values\n"
    "key require <keyname> <image|conf> - require a key, by name, for either image or configuration signing\n"
    "key unrequire <keyname> - do not require a key (by name)"
);
#endif
