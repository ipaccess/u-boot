#ifndef IPAFSM92XX_TLMM_H_20131112
#define IPAFSM92XX_TLMM_H_20131112

/* TLMM Pull */
#define TLMM_PULL_NONE        (0)
#define TLMM_PULL_DOWN        (1)
#define TLMM_PULL_KEEPER      (2)
#define TLMM_PULL_UP          (3)

/* TLMM drive */
#define TLMM_DRIVE_2MA        (0)
#define TLMM_DRIVE_4MA        (1)
#define TLMM_DRIVE_6MA        (2)
#define TLMM_DRIVE_8MA        (3)
#define TLMM_DRIVE_10MA       (4)
#define TLMM_DRIVE_12MA       (5)
#define TLMM_DRIVE_14MA       (6)
#define TLMM_DRIVE_16MA       (7)

/* Output Enable */
#define TLMM_OE_DISABLE       (0)
#define TLMM_OE_ENABLE        (1)

int tlmm_out(
    int gpio,
    int value);

int tlmm_oe(
    int gpio,
    int enable);

int tlmm_drv(
    int gpio,
    int drv);

int tlmm_func(
    int gpio,
    int func);

int tlmm_pull(
    int gpio,
    int pull);

int tlmm_in(
    int gpio,
    int *value);

int
tlmm_gpio_cfg_get(
    int gpio,
    int *value);

int
tlmm_gpio_cfg_set(
    int gpio,
    int value);

#endif /* IPAFSM92XX_TLMM_H_20131112 */
