#ifndef SECBOOT_H
#define SECBOOT_H

int load_security_requirements(void);

// return 1 if silent mode, 0 otherwise.
int silent_mode_enabled(void);

#endif
