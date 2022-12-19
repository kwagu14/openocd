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
    //should be changed to an array of 4 bytes ? 
	uint32_t data;

	for (i = 0; i < count; i++) {
		retval = avr_jtag_mwa_read(jtag_info, SLAVE_HSB_UNCACHED,
				addr + i, &data);

		if (retval != ERROR_OK)
			return retval;

	}

	return ERROR_OK;
}