#ifndef IPAT2K_CHARACTERISATION_H_20160422
#define IPAT2K_CHARACTERISATION_H_20160422

#include <common.h>

#define RSM_A_PROD          0x8
#define RSM_A_DEV           0x4
#define RSM_A_SPECIALS      0x2
#define RSM_A_TEST          0x5

extern int characterisation_init(void);
extern void print_characterisation(void);

extern int characterisation_is_production_mode(void);
extern int characterisation_is_test_mode(void);
extern int characterisation_is_development_mode(void);
extern int characterisation_is_specials_mode(void);
extern u8 characterisation_loader_revocation(void);
extern u8 characterisation_application_revocation(void);

#endif
