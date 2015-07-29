#ifndef IPA9131_CHARACTERISATION_H_20150317
#define IPA9131_CHARACTERISATION_H_20150317

#include <common.h>
#define CHARACTERISATION_EID_LENGTH 17

extern int characterisation_init(void);
extern void print_characterisation(void);

extern int characterisation_is_production_mode(void);
extern int characterisation_is_test_mode(void);
extern int characterisation_is_development_mode(void);
extern int characterisation_is_specials_mode(void);
extern u8 characterisation_loader_revocation(void);
extern u8 characterisation_application_revocation(void);
extern int characterisation_read_eid(char *eid,size_t len);

#endif
