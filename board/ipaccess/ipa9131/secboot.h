#ifndef SECBOOT_H
#define SECBOOT_H

extern int load_security_requirements(void);

/* return 1 if silent mode, 0 otherwise. */
extern int silent_mode_enabled(void);

#endif
