#ifndef IPAT2K_CHARACTERISATION_H_20160422
#define IPAT2K_CHARACTERISATION_H_20160422

#include <common.h>

extern int characterisation_init(void);
extern void print_characterisation(void);

extern int characterisation_is_production_mode(void);
extern int characterisation_is_test_mode(void);
extern int characterisation_is_development_mode(void);
extern int characterisation_is_specials_mode(void);
extern u8 characterisation_loader_revocation(void);
extern u8 characterisation_application_revocation(void);

#endif
