#ifndef CHARACTERISATION_H
#define CHARACTERISATION_H

#include <common.h>

extern int characterisation_is_production_mode(void);
extern int characterisation_is_test_mode(void);
extern int characterisation_is_development_mode(void);
extern int characterisation_is_specials_mode(void);
extern int is_secure_boot(void);
#endif
