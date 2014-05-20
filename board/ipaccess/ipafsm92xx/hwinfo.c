#include "hwinfo.h"

#include <common.h>
#include <fuse.h>

#define QFPROM_CORR_EUI64_ROW0_LSB 0x43E0
#define QFPROM_CORR_EUI64_ROW1_LSB 0x43E8

#define QFPROM_CORR_SPARE4_REG21_ROW0_LSB 0x4400
#define QFPROM_CORR_SPARE4_REG21_ROW0_MSB 0x4404




static const char * oscillator_text[] = {
    "030",
    "015",
    "000",
    "000"
};
static const int num_oscillators = sizeof(oscillator_text) / sizeof(oscillator_text[0]);



typedef struct variant_record_s {
    const char full[5];
    const char part[4];
    const char variant[2];
    u16 part_num;
    u8 variant_char;
} variant_record;

static const variant_record variant_lookup[] = {
    { "0000", "000", "0",   0, '0' },
    { "230A", "230", "A", 230, 'A' },
    { "230B", "230", "B", 230, 'B' }
};
static const int num_variants = sizeof(variant_lookup) / sizeof(variant_lookup[0]);



static const char * lookup_oscillator(u8 oscillator)
{
    if (oscillator >= num_oscillators) oscillator = (num_oscillators - 1);
    return oscillator_text[oscillator];
}



static const variant_record * lookup_variant(u16 hw_variant)
{
    if (hw_variant >= num_variants) hw_variant = 0;
    return &variant_lookup[hw_variant];
}




void print_ipa_hardware_info(void)
{
    u32 oui;
    u32 serial;
    u8 oscillator;
    u16 hw_variant;
    u16 pcb_assembly;

    if (0 != read_eid(&oui, &serial) ||
        0 != read_hardware_info(&oscillator, &hw_variant, &pcb_assembly))
    {
        return;
    }

    printf("Hardware Platform  : %s%s\n", lookup_variant(hw_variant)->full, lookup_oscillator(oscillator));
    printf("Serial Number      : %06X-%010u\n", oui, serial);
    printf("PCB Version        : %05u\n", pcb_assembly);
}




int read_eid(u32 * oui, u32 * serial)
{
    if (0 != fuse_read(QFPROM_CORR_EUI64_ROW1_LSB, 0, oui))
    {
        debug("%s: failed to read %s\n", __func__, "QFPROM_CORR_EUI64_ROW1_LSB");
        return -1;
    }

    *oui &= 0x00FFFFFF;

    if (0 != fuse_read(QFPROM_CORR_EUI64_ROW0_LSB, 0, serial))
    {
        debug("%s: failed to read %s\n", __func__, "QFPROM_CORR_EUI64_ROW0_LSB");
        return -1;
    }

    return 0;
}




int read_hardware_info(u8 * oscillator, u16 * hw_variant, u16 * pcb_assembly)
{
    u32 raw_hw_info;
    u16 hw_info;
    u32 raw_pcb_assembly;

    if (0 != fuse_read(QFPROM_CORR_SPARE4_REG21_ROW0_LSB, 0, &raw_hw_info))
    {
        debug("%s: failed to read %s\n", __func__, "QFPROM_CORR_SPARE4_REG21_ROW0_LSB");
        return -1;
    }

    if (0 != fuse_read(QFPROM_CORR_SPARE4_REG21_ROW0_MSB, 0, &raw_pcb_assembly))
    {
        debug("%s: failed to read %s\n", __func__, "QFPROM_CORR_SPARE4_REG21_ROW0_MSB");
        return -1;
    }

    hw_info = ((raw_hw_info & 0xFFFF0000) >> 16) | (raw_hw_info & 0x0000FFFF);

    *pcb_assembly = (((raw_pcb_assembly & 0xFFFF0000) >> 16) & 0x0000FFFF) | (raw_pcb_assembly & 0x0000FFFF);
    *oscillator = (hw_info & 0x0003);
    *hw_variant = ((hw_info & 0x0FFC) >> 2) & 0x03FF;
    return 0;
}
