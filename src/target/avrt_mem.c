/************************************************
* Author: Karley W.                             * 
*                                               *
* File Purpose: Handles higher-level read and   *
* write capabilities for 8-bit AVR chips.       *
*                                               *
* Note: this file was modeled after the 32-bit  *
* AVR driver, but reading and writing on 8-bit  *
* chips follows a very different process. Will  *
* probably need modification after jtag file is *
* created.                                      *
*                                               *
************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "target.h"
#include "jtag/jtag.h"
#include "avrt_jtag.h"
#include "avrt_mem.h"


int avr_jtag_read_memory8(struct mcu_jtag *jtag_info,
	uint32_t addr, int count, uint8_t *buffer)
{
	int i, retval; 
	uint8_t data;

	for (i = 0; i < count; i++) {
		retval = avr_jtag_mwa_read(jtag_info, SLAVE_HSB_UNCACHED,
				addr + i, &data);

		if (retval != ERROR_OK)
			return retval;

	}

	return ERROR_OK;
}
