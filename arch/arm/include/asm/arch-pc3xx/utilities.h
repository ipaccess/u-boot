/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file utilities.h
* \brief Definitions for some useful PC3xx functions.
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/
#ifndef __PC3XX_UTILS_H__
#define __PC3XX_UTILS_H__

/* Includes ---------------------------------------------------------------- */
#include <common.h>

/* Prototypes--------------------------------------------------------------- */
/*!
 * \brief Read a 32 bit value from a register.
 *
 * \param The address to read from
 * \return The value read
 */
__inline unsigned int
pc302_read_from_register (const unsigned int address);

/*!
 * \brief Write a 32 bit value to a register
 *
 * \param The address to write to
 * \param The value to write.
 */
__inline void
pc302_write_to_register (const unsigned int address,
                         const unsigned int value);

/*!
 * \brief Read the device identification code from the axi2cfg block
 *
 * \return The value read
 */
unsigned int
pc302_read_device_id (void);

/*!
 * \brief Read the device revision from the axi2cfg block
 *
 * \return The value read
 */
unsigned int
pc302_read_device_revision (void);

/*!
 * \brief Read the die id number from the fuse block
 *
 * \param die_number Pointer to an array
 */
void
pc302_read_die_id_number (unsigned int * die_number);

/*!
 * \brief Return the state of the Reduced MII enabled bit.
 *
 * \return 0 - RMII not enabled
 *         1 - RMII enabled
 */
unsigned int
pc302_get_rmii_enabled (void);

/*!
 * \brief Read a number of 16 bit words from the PC3xx axi2cfg.
 *
 * \param caeid The CAEID of the AE to read from.
 * \param address The start address in the AE to begin reading from.
 * \param[out] data The buffer to store the data in.
 * \param count The number of 16 bit words to read.
 * \return Returns the number of words read on success, negative on failure.
 */
int
axi2cfg_config_read( u16 caeid,
                     u16 address,
                     u16 *data,
                     u16 count );

/*!
 * \brief Write a number of 16 bit words to the PC3xx axi2cfg.
 *
 * \param caeid The CAEID of the AE to write to.
 * \param address The start address in the AE to begin writing to.
 * \param[in] data The buffer to write from.
 * \param count The number of 16 bit words to write.
 * \return Returns the number of words written on success, negative on failure.
 */
int
axi2cfg_config_write( u16 caeid,
                      u16 address,
                      u16 *data,
                      u16 count );

/*!
 * \brief Read the system configuration register.
 *
 * \return Returns the value of the system configuration register.
 */
u32
syscfg_read(void);

/*!
 * \brief Update the system configuration register.
 *
 * \param mask Mask of the bits to update.
 * \param val The value to write.
 */
void
syscfg_update(u32 mask,
              u32 val);

/*!
 * \brief Are we running on a PC3x3 ?
 *
 * \return 1 if running on a PC3x3 device
 *         0 if not running on a PC3x3 device
 */
__inline int
is_pc3x3(void);

#endif /* __PC3XX_UTILS_H__ */
