/*
 *  Open HR20
 *
 *  target:     ATmega169 @ 4 MHz in Honnywell Rondostat HR20E
 *
 *  compiler:   WinAVR-20071221
 *              avr-libc 1.6.0
 *              GCC 4.2.2
 *
 *  copyright:  2008 Jiri Dobry (jdobry-at-centrum-dot-cz)
 *
 *  license:    This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU Library General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later version.
 *
 *              This program is distributed in the hope that it will be useful,
 *              but WITHOUT ANY WARRANTY; without even the implied warranty of
 *              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *              GNU General Public License for more details.
 *
 *              You should have received a copy of the GNU General Public License
 *              along with this program. If not, see http:*www.gnu.org/licenses
 */

/*!
 * \file       eeprom.c
 * \brief      EEPROM storage
 * \author     Jiri Dobry <jdobry-at-centrum-dot-cz>
 * \date       $Date$
 * $Rev$
 */

#include "config.h"
#if !defined(MASTER_CONFIG_H)
#include "controller.h"
#endif
#include <avr/eeprom.h>

#define __EEPROM_C__
#include "eeprom.h"

#if !defined(EEWE) && defined(EEPE)
# define EEWE EEPE
#endif

#if !defined(EEMWE) && defined(EEMPE)
# define EEMWE EEMPE
#endif



// test for compilation
#if RTC_TIMERS_PER_DOW != 8
#error EEPROM layout is prepared for RTC_TIMERS_PER_DOW
#endif


/*!
 *******************************************************************************
 *  \note standard asm/eeprom.h is not used.
 *  Reason: eeprom_write_byte use only uint8_t address
 ******************************************************************************/

config_t config;

/*!
 *******************************************************************************
 *  generic EEPROM read
 *
 ******************************************************************************/
uint8_t EEPROM_read(uint16_t address)
{
	/* Wait for completion of previous write */
	while (EECR & (1 << EEWE))
	{
		;
	}
	EEAR = address;
	EECR |= (1 << EERE);
	return EEDR;
}

/*!
 *******************************************************************************
 *  config_read
 *	it is similar as EEPROM_read, but optimized for special usage
 ******************************************************************************/
uint8_t config_read(uint8_t cfg_address, uint8_t cfg_type)
{
	/* Wait for completion of previous write */
	while (EECR & (1 << EEWE))
	{
		;
	}
	EEAR = (((uint16_t)cfg_address) << 2) + cfg_type + (uint16_t)(&ee_config);
	EECR |= (1 << EERE);
	return EEDR;
}

/*!
 *******************************************************************************
 *  EEPROM_write
 *
 *  \note private function
 *  \note write to ee_config is limited
 ******************************************************************************/
#define config_write(cfg_address, data) (EEPROM_write((((uint16_t)cfg_address) << 2) + CONFIG_VALUE + (uint16_t)(&ee_config), data))

void EEPROM_write(uint16_t address, uint8_t data)
{
	/* Wait for completion of previous write */
	while (EECR & (1 << EEWE))
	{
		;
	}
	EEAR = address;
	EEDR = data;
	asm ("cli");
	EECR |= (1 << EEMWE);
	EECR |= (1 << EEWE);
	asm ("sei");
}


#if WINDOW_DETECTION_RUNTIME && !BOOST_CONTROLER_AFTER_CHANGE && !TEMP_COMPENSATE_OPTION
#define EE_LAYOUT_LEGACY_SOFTWARE 0x14
#define EE_LAYOUT_LEGACY_HARDWARE 0x15
#define EE_MIG_MAGIC0 0x57
#define EE_MIG_MAGIC1 0x4d
#define EE_MIG_OLD_FIRST 0x26
#define EE_MIG_OLD_COUNT 5

static uint8_t eeprom_old_config_value(uint8_t idx)
{
\treturn EEPROM_read((uint16_t)&ee_config + ((uint16_t)idx << 2) + CONFIG_VALUE);
}

static void eeprom_config_record_write(uint8_t idx, uint8_t value, uint8_t def, uint8_t min, uint8_t max)
{
\tuint16_t base = (uint16_t)&ee_config + ((uint16_t)idx << 2);
\tEEPROM_write(base + CONFIG_VALUE, value);
\tEEPROM_write(base + CONFIG_DEFAULT, def);
\tEEPROM_write(base + CONFIG_MIN, min);
\tEEPROM_write(base + CONFIG_MAX, max);
}

void eeprom_layout_migrate(void)
{
\tuint8_t old_layout = EEPROM_read((uint16_t)&ee_layout);
\tuint8_t old_value[EE_MIG_OLD_COUNT];
\tuint8_t i;
\tbool staged;

\tif (old_layout == EE_LAYOUT)
\t{
\t\treturn;
\t}
\tif ((old_layout != EE_LAYOUT_LEGACY_SOFTWARE) && (old_layout != EE_LAYOUT_LEGACY_HARDWARE))
\t{
\t\treturn;
\t}

\tstaged = (EEPROM_read((uint16_t)&ee_reserved2_60[0]) == EE_MIG_MAGIC0)
\t\t && (EEPROM_read((uint16_t)&ee_reserved2_60[1]) == EE_MIG_MAGIC1)
\t\t && (EEPROM_read((uint16_t)&ee_reserved2_60[2]) == old_layout);

\tif (staged)
\t{
\t\tfor (i = 0; i < EE_MIG_OLD_COUNT; i++)
\t\t{
\t\t\told_value[i] = EEPROM_read((uint16_t)&ee_reserved2_60[3 + i]);
\t\t}
\t}
\telse
\t{
\t\t/* Invalidate the staging header before filling it. */
\t\tEEPROM_write((uint16_t)&ee_reserved2_60[0], 0);
\t\tEEPROM_write((uint16_t)&ee_reserved2_60[1], 0);
\t\tEEPROM_write((uint16_t)&ee_reserved2_60[2], old_layout);
\t\tfor (i = 0; i < EE_MIG_OLD_COUNT; i++)
\t\t{
\t\t\told_value[i] = eeprom_old_config_value(EE_MIG_OLD_FIRST + i);
\t\t\tEEPROM_write((uint16_t)&ee_reserved2_60[3 + i], old_value[i]);
\t\t}
\t\t/* Header becomes valid only after all legacy values are safely staged. */
\t\tEEPROM_write((uint16_t)&ee_reserved2_60[1], EE_MIG_MAGIC1);
\t\tEEPROM_write((uint16_t)&ee_reserved2_60[0], EE_MIG_MAGIC0);
\t}

\tif (old_layout == EE_LAYOUT_LEGACY_SOFTWARE)
\t{
\t\teeprom_config_record_write(0x26, WINDOW_DETECTION_SOFTWARE, WINDOW_DETECTION_SOFTWARE, WINDOW_DETECTION_OFF, WINDOW_DETECTION_HARDWARE);
\t\teeprom_config_record_write(0x27, old_value[0], 50, 7, 255);
\t\teeprom_config_record_write(0x28, old_value[1], 50, 7, 255);
\t\teeprom_config_record_write(0x29, old_value[2], 8, 1, AVGS_BUFFER_LEN);
\t\teeprom_config_record_write(0x2a, old_value[3], 8, 1, AVGS_BUFFER_LEN);
\t\teeprom_config_record_write(0x2b, old_value[4], 90, 2, 255);
\t\teeprom_config_record_write(0x2c, 5, 5, 0, 240);
\t\teeprom_config_record_write(0x2d, 5, 5, 0, 240);
\t}
\telse
\t{
\t\teeprom_config_record_write(0x26, old_value[0] ? WINDOW_DETECTION_HARDWARE : WINDOW_DETECTION_OFF,
\t\t                           WINDOW_DETECTION_SOFTWARE, WINDOW_DETECTION_OFF, WINDOW_DETECTION_HARDWARE);
\t\teeprom_config_record_write(0x27, 50, 50, 7, 255);
\t\teeprom_config_record_write(0x28, 50, 50, 7, 255);
\t\teeprom_config_record_write(0x29, 8, 8, 1, AVGS_BUFFER_LEN);
\t\teeprom_config_record_write(0x2a, 8, 8, 1, AVGS_BUFFER_LEN);
\t\teeprom_config_record_write(0x2b, 90, 90, 2, 255);
\t\teeprom_config_record_write(0x2c, old_value[1], 5, 0, 240);
\t\teeprom_config_record_write(0x2d, old_value[2], 5, 0, 240);
\t}

\t/* Commit marker last. If power fails earlier, the staged legacy values allow a safe retry. */
\tEEPROM_write((uint16_t)&ee_layout, EE_LAYOUT);
\tEEPROM_write((uint16_t)&ee_reserved2_60[0], 0);
\tEEPROM_write((uint16_t)&ee_reserved2_60[1], 0);
}
#else
void eeprom_layout_migrate(void)
{
}
#endif


/*!
 *******************************************************************************
 *  Init configuration storage
 *
 *  \note
 ******************************************************************************/
void eeprom_config_init(bool restore_default)
{
	uint16_t i;
	uint8_t *config_ptr = config_raw;

#if (NANODE == 1 || JEENODE == 1)
	// set to allow erase and write in one operation
	EECR |= (EEPM1 | EEPM0);
#endif
	for (i = 0; i < CONFIG_RAW_SIZE; i++)
	{
		if (restore_default)
		{
			*config_ptr = config_default(i); // default value
		}
		else
		{
			*config_ptr = config_value(i);
			if ((*config_ptr < config_min(i))               //min
			    || (*config_ptr > config_max(i)))           //max
			{
				*config_ptr = config_default(i);        // default value
			}
		}
		eeprom_config_save(i);                                  // update if default value is restored
		config_ptr++;
	}
}


/*!
 *******************************************************************************
 *  Update configuration storage
 *
 *  \note
 ******************************************************************************/
void eeprom_config_save(uint8_t idx)
{
	if (idx < CONFIG_RAW_SIZE)
	{
		if (config_raw[idx] != config_value(idx))
		{
			if ((config_raw[idx] < config_min(idx))         //min
			    || (config_raw[idx] > config_max(idx)))     //max
			{
				config_raw[idx] = config_default(idx);  // default value
			}
			config_write(idx, config_raw[idx]);
		}
	}
}

#if !defined(MASTER_CONFIG_H)

uint8_t timers_patch_offset = 0xff;
uint16_t timers_patch_data;

/*!
 *******************************************************************************
 *  read timer from storage
 *
 *  \note
 ******************************************************************************/
uint16_t eeprom_timers_read_raw(uint8_t offset)
{
	if (offset != timers_patch_offset)
	{
		uint16_t eeaddr = (uint16_t)offset * (uint16_t)sizeof(ee_timers[0][0]) + (uint16_t)ee_timers;
		return (EEPROM_read(eeaddr + 1) << 8) + EEPROM_read(eeaddr); //litle endian
	}
	else
	{
		return timers_patch_data;
	}
}


/*!
 *******************************************************************************
 *  Update timer storage for dow and slot
 *
 *  \note
 ******************************************************************************/
void eeprom_timers_write_raw(uint8_t offset, uint16_t value)
{
	if (offset >= (uint8_t)(sizeof(ee_timers) / sizeof(ee_timers[0][0])))
	{
		return;                                                                 // EEPROM protection
	}
	uint16_t eeaddr = (uint16_t)offset * (uint16_t)sizeof(ee_timers[0][0]) + (uint16_t)ee_timers;
	EEPROM_write(eeaddr, value & 0xff);                                             //litle endian
	EEPROM_write(eeaddr + 1, (value >> 8));                                         //litle endian
}

#endif // !defined(MASTER_CONFIG_H)
