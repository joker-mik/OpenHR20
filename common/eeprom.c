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
	uint8_t sreg;

	/* Wait for completion of previous write */
	while (EECR & (1 << EEWE))
	{
		;
	}
	EEAR = address;
	EEDR = data;
	sreg = SREG;
	cli();
	EECR |= (1 << EEMWE);
	EECR |= (1 << EEWE);
	SREG = sreg;
}


#if WINDOW_DETECTION_RUNTIME && !BOOST_CONTROLER_AFTER_CHANGE && !TEMP_COMPENSATE_OPTION
#define EE_LAYOUT_LEGACY_SOFTWARE 0x14
#define EE_LAYOUT_LEGACY_HARDWARE 0x15
#define EE_MIG_MAGIC0 0x57
#define EE_MIG_MAGIC1 0x4d
#define EE_MIG_OLD_FIRST ((uint8_t)OFFSETOF(config_t, window_detection_mode))
#define EE_MIG_OLD_COUNT 5
#define EE_CFG_IDX(field) ((uint8_t)OFFSETOF(config_t, field))

static uint8_t eeprom_old_config_value(uint8_t idx)
{
	return EEPROM_read((uint16_t)&ee_config + ((uint16_t)idx << 2) + CONFIG_VALUE);
}

static void eeprom_config_record_write(uint8_t idx, uint8_t value, uint8_t def, uint8_t min, uint8_t max)
{
	uint16_t base = (uint16_t)&ee_config + ((uint16_t)idx << 2);
	EEPROM_write(base + CONFIG_VALUE, value);
	EEPROM_write(base + CONFIG_DEFAULT, def);
	EEPROM_write(base + CONFIG_MIN, min);
	EEPROM_write(base + CONFIG_MAX, max);
}

void eeprom_layout_migrate(void)
{
	uint8_t old_layout = EEPROM_read((uint16_t)&ee_layout);
	uint8_t staged_layout = EEPROM_read((uint16_t)&ee_reserved2_60[2]);
	uint8_t old_value[EE_MIG_OLD_COUNT];
	uint8_t i;
	bool staged;

	staged = (EEPROM_read((uint16_t)&ee_reserved2_60[0]) == EE_MIG_MAGIC0)
		 && (EEPROM_read((uint16_t)&ee_reserved2_60[1]) == EE_MIG_MAGIC1)
		 && ((staged_layout == EE_LAYOUT_LEGACY_SOFTWARE) || (staged_layout == EE_LAYOUT_LEGACY_HARDWARE));

	if (old_layout == EE_LAYOUT)
	{
		/* A reset after committing ee_layout may leave the scratch header valid. */
		if (staged)
		{
			EEPROM_write((uint16_t)&ee_reserved2_60[0], 0);
			EEPROM_write((uint16_t)&ee_reserved2_60[1], 0);
		}
		return;
	}

	if (staged)
	{
		/* Staging is authoritative even if a power loss corrupted ee_layout mid-write. */
		old_layout = staged_layout;
		for (i = 0; i < EE_MIG_OLD_COUNT; i++)
		{
			old_value[i] = EEPROM_read((uint16_t)&ee_reserved2_60[3 + i]);
		}
	}
	else
	{
		uint8_t old_count;
		if ((old_layout != EE_LAYOUT_LEGACY_SOFTWARE) && (old_layout != EE_LAYOUT_LEGACY_HARDWARE))
		{
			return;
		}
		old_count = (old_layout == EE_LAYOUT_LEGACY_SOFTWARE) ? 5 : 3;

		/* Invalidate the staging header before filling it. */
		EEPROM_write((uint16_t)&ee_reserved2_60[0], 0);
		EEPROM_write((uint16_t)&ee_reserved2_60[1], 0);
		EEPROM_write((uint16_t)&ee_reserved2_60[2], old_layout);
		for (i = 0; i < EE_MIG_OLD_COUNT; i++)
		{
			old_value[i] = (i < old_count) ? eeprom_old_config_value(EE_MIG_OLD_FIRST + i) : 0xff;
			EEPROM_write((uint16_t)&ee_reserved2_60[3 + i], old_value[i]);
		}
		/* Header becomes valid only after all legacy values are safely staged. */
		EEPROM_write((uint16_t)&ee_reserved2_60[1], EE_MIG_MAGIC1);
		EEPROM_write((uint16_t)&ee_reserved2_60[0], EE_MIG_MAGIC0);
	}

	if (old_layout == EE_LAYOUT_LEGACY_SOFTWARE)
	{
		eeprom_config_record_write(EE_CFG_IDX(window_detection_mode), WINDOW_DETECTION_SOFTWARE, WINDOW_DETECTION_SOFTWARE, WINDOW_DETECTION_OFF, WINDOW_DETECTION_HARDWARE);
		eeprom_config_record_write(EE_CFG_IDX(window_open_detection_diff), old_value[0], 50, 7, 255);
		eeprom_config_record_write(EE_CFG_IDX(window_close_detection_diff), old_value[1], 50, 7, 255);
		eeprom_config_record_write(EE_CFG_IDX(window_open_detection_time), old_value[2], 8, 1, AVGS_BUFFER_LEN);
		eeprom_config_record_write(EE_CFG_IDX(window_close_detection_time), old_value[3], 8, 1, AVGS_BUFFER_LEN);
		eeprom_config_record_write(EE_CFG_IDX(window_open_timeout), old_value[4], 90, 2, 255);
		eeprom_config_record_write(EE_CFG_IDX(hw_window_open_detection_delay), 5, 5, 0, 240);
		eeprom_config_record_write(EE_CFG_IDX(hw_window_close_detection_delay), 5, 5, 0, 240);
	}
	else
	{
		eeprom_config_record_write(EE_CFG_IDX(window_detection_mode), old_value[0] ? WINDOW_DETECTION_HARDWARE : WINDOW_DETECTION_OFF,
		                           WINDOW_DETECTION_SOFTWARE, WINDOW_DETECTION_OFF, WINDOW_DETECTION_HARDWARE);
		eeprom_config_record_write(EE_CFG_IDX(window_open_detection_diff), 50, 50, 7, 255);
		eeprom_config_record_write(EE_CFG_IDX(window_close_detection_diff), 50, 50, 7, 255);
		eeprom_config_record_write(EE_CFG_IDX(window_open_detection_time), 8, 8, 1, AVGS_BUFFER_LEN);
		eeprom_config_record_write(EE_CFG_IDX(window_close_detection_time), 8, 8, 1, AVGS_BUFFER_LEN);
		eeprom_config_record_write(EE_CFG_IDX(window_open_timeout), 90, 90, 2, 255);
		eeprom_config_record_write(EE_CFG_IDX(hw_window_open_detection_delay), old_value[1], 5, 0, 240);
		eeprom_config_record_write(EE_CFG_IDX(hw_window_close_detection_delay), old_value[2], 5, 0, 240);
	}

	/* Commit marker last. If power fails earlier, the staged legacy values allow a safe retry. */
	EEPROM_write((uint16_t)&ee_layout, EE_LAYOUT);
	EEPROM_write((uint16_t)&ee_reserved2_60[0], 0);
	EEPROM_write((uint16_t)&ee_reserved2_60[1], 0);
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
	if (offset >= (uint8_t)(sizeof(ee_timers) / sizeof(ee_timers[0][0])))
	{
		return 0xffff;
	}
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