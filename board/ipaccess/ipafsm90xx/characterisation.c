#include <common.h>
#include <malloc.h>
#include <linux/compiler.h>
#include <errno.h>

#include "characterisation.h"



int characterisation_is_production_mode(void)
{
    return 0;
}


int characterisation_is_test_mode(void)
{
    return 0;
}


int characterisation_is_development_mode(void)
{
    return 1;
}


int characterisation_is_specials_mode(void)
{
    return 0;
}

int is_secure_boot(void)
{
    return 0;
}
