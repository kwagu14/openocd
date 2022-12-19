#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "target.h"
#include "jtag/jtag.h"
#include "avrt_jtag.h"


static int avr_jtag_mwa_read_data(struct mcu_jtag *jtag_info,uint8_t *pdata)
{

	struct scan_field fields[2];
	uint8_t data_buf[4];
	uint8_t busy_buf[4];
	int busy;

	do {
		memset(data_buf, 0, sizeof(data_buf));
		memset(busy_buf, 0, sizeof(busy_buf));

		fields[0].num_bits = 8;
		fields[0].out_value = NULL;
		fields[0].in_value = data_buf;


		fields[1].num_bits = 3;
		fields[1].in_value = busy_buf;
		fields[1].out_value = NULL;

		jtag_add_dr_scan(jtag_info->tap, 2, fields, TAP_IDLE);

		if (jtag_execute_queue() != ERROR_OK) {
			LOG_ERROR("%s: reading data  failed", __func__);
			return ERROR_FAIL;
		}

		busy = buf_get_u32(busy_buf, 0, 1);
	} while (busy);

	*pdata = buf_get_u32(data_buf, 0, 32);

	return ERROR_OK;
}