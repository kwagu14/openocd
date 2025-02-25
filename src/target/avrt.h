/* SPDX-License-Identifier: GPL-2.0-or-later */

/***************************************************************************
 *   Copyright (C) 2009 by Simon Qian                                      *
 *   SimonQian@SimonQian.com                                               *
 *                                                                         *
 *   Last modified: 01/2023 by Karley W.                                   *
 *	 kwagu14@lsu.edu                                                       *
 ***************************************************************************/

#ifndef OPENOCD_TARGET_AVRT_H
#define OPENOCD_TARGET_AVRT_H

#include <jtag/jtag.h>

struct avr_common {
	struct avrt_jtag jtag_info;
};

static inline struct avr_common *
target_to_avr(struct target *target)
{
	return (struct avr_common *)target->arch_info;
}

int mcu_execute_queue(void);
int avr_jtag_sendinstr(struct jtag_tap *tap, uint8_t *ir_in, uint8_t ir_out);
int avr_jtag_senddat(struct jtag_tap *tap, uint32_t *dr_in, uint32_t dr_out,
		int len);

#endif /* OPENOCD_TARGET_AVRT_H */
