#ifndef OPENOCD_TARGET_AVRT_MEM_H
#define OPENOCD_TARGET_AVRT_MEM_H

int avr_jtag_read_memory8(struct mcu_jtag *jtag_info,
	uint32_t addr, int count, uint8_t *buffer);

#endif