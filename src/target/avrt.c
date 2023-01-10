// SPDX-License-Identifier: GPL-2.0-or-later

/***************************************************************************
 *   Copyright (C) 2009 by Simon Qian                                      *
 *   SimonQian@SimonQian.com                                               *
 *                                                                         *
 *   Last modified 01/2023 by Karley W.                                    *
 *	 kwagu14@lsu.edu                                                       *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "jtag/jtag.h"
#include "avrt.h"
#include "target.h"
#include "target_type.h"
#include "avrt_mem.h"
#include "avrt_jtag.h"
#include "avrt_regs.h"
#include "breakpoints.h"
#include "algorithm.h"
#include "register.h"

#define AVR_JTAG_INS_LEN	4

/* forward declarations */
static int avr_target_create(struct target *target, Jim_Interp *interp);
static int avr_init_target(struct command_context *cmd_ctx, struct target *target);

static int avr_arch_state(struct target *target);
static int avr_poll(struct target *target);
static int avr_halt(struct target *target);
static int avr_resume(struct target *target, int current, target_addr_t address,
		int handle_breakpoints, int debug_execution);
static int avr_step(struct target *target, int current, target_addr_t address,
		int handle_breakpoints);

static int avr_assert_reset(struct target *target);
static int avr_deassert_reset(struct target *target);

/* IR and DR functions */
static int mcu_write_ir(struct jtag_tap *tap, uint8_t *ir_in, uint8_t *ir_out, int ir_len, int rti);
static int mcu_write_dr(struct jtag_tap *tap, uint8_t *dr_in, uint8_t *dr_out, int dr_len, int rti);
static int mcu_write_ir_u8(struct jtag_tap *tap, uint8_t *ir_in, uint8_t ir_out, int ir_len, int rti);
static int mcu_write_dr_u32(struct jtag_tap *tap, uint32_t *ir_in, uint32_t ir_out, int dr_len, int rti);

struct target_type avr_target = {
	.name = "avr",

	.poll = avr_poll,
	.arch_state = avr_arch_state,

	.halt = avr_halt,
	.resume = avr_resume,
	.step = avr_step,

	.assert_reset = avr_assert_reset,
	.deassert_reset = avr_deassert_reset,
	.read_memory = avr_read_memory,
/*
	.get_gdb_reg_list = avr_get_gdb_reg_list,
	.write_memory = avr_write_memory,
	.bulk_write_memory = avr_bulk_write_memory,
	.checksum_memory = avr_checksum_memory,
	.blank_check_memory = avr_blank_check_memory,

	.run_algorithm = avr_run_algorithm,

	.add_breakpoint = avr_add_breakpoint,
	.remove_breakpoint = avr_remove_breakpoint,
	.add_watchpoint = avr_add_watchpoint,
	.remove_watchpoint = avr_remove_watchpoint,
*/
	.target_create = avr_target_create,
	.init_target = avr_init_target,
};

static int avr_target_create(struct target *target, Jim_Interp *interp)
{
	struct avr_common *avr = calloc(1, sizeof(struct avr_common));

	avr->jtag_info.tap = target->tap;
	target->arch_info = avr;
	
	//debugging code for ATMEGA32A
	if(avr->jtag_info.tap->ir_length != 4){
		LOG_ERROR("The jtag instruction length should be 4 bits long.");
		return ERROR_FAIL;
	}

	return ERROR_OK;
}

static int avr_init_target(struct command_context *cmd_ctx, struct target *target)
{
	struct avr_common *avr = target_to_avr(target);

	avr->jtag_info.tap = target->tap;
	return ERROR_OK;
}

static int avr_arch_state(struct target *target)
{
	LOG_DEBUG("%s", __func__);
	return ERROR_OK;
}

static int avr_poll(struct target *target)
{
	if ((target->state == TARGET_RUNNING) || (target->state == TARGET_DEBUG_RUNNING))
		target->state = TARGET_HALTED;

	LOG_DEBUG("%s", __func__);
	return ERROR_OK;
}

static int avr_halt(struct target *target)
{
	LOG_DEBUG("%s", __func__);
	return ERROR_OK;
}

static int avr_resume(struct target *target, int current, target_addr_t address,
		int handle_breakpoints, int debug_execution)
{
	LOG_DEBUG("%s", __func__);
	return ERROR_OK;
}

static int avr_step(struct target *target, int current, target_addr_t address, int handle_breakpoints)
{
	LOG_DEBUG("%s", __func__);
	return ERROR_OK;
}

static int avr_assert_reset(struct target *target)
{
	target->state = TARGET_RESET;

	LOG_DEBUG("%s", __func__);
	return ERROR_OK;
}

static int avr_deassert_reset(struct target *target)
{
	target->state = TARGET_RUNNING;

	LOG_DEBUG("%s", __func__);
	return ERROR_OK;
}

static int avr_read_memory(struct target *target, target_addr_t address, uint32_t count, uint8_t *buffer){
	struct avr_common *avr = target_to_avr(target);
	
	LOG_DEBUG("address: 0x%8.8" TARGET_PRIxADDR ", size: 0x%8.8" PRIx32 ", count: 0x%8.8" PRIx32 "",
		address,
		size,
		count);

	if ((count == 0) || !(buffer)){
		return ERROR_COMMAND_SYNTAX_ERROR;
	}

	return avr_jtag_read_memory8(&avr_common->jtag_info, address, count, buffer);

}


int avr_jtag_senddat(struct jtag_tap *tap, uint32_t *dr_in, uint32_t dr_out,
		int len)
{
	return mcu_write_dr_u32(tap, dr_in, dr_out, len, 1);
}

int avr_jtag_sendinstr(struct jtag_tap *tap, uint8_t *ir_in, uint8_t ir_out)
{
	return mcu_write_ir_u8(tap, ir_in, ir_out, AVR_JTAG_INS_LEN, 1);
}

/* IR and DR functions */
static int mcu_write_ir(struct jtag_tap *tap, uint8_t *ir_in, uint8_t *ir_out,
		int ir_len, int rti)
{
	if (!tap) {
		LOG_ERROR("invalid tap");
		return ERROR_FAIL;
	}
	if (ir_len != tap->ir_length) {
		LOG_ERROR("invalid ir_len");
		return ERROR_FAIL;
	}

	{
		jtag_add_plain_ir_scan(tap->ir_length, ir_out, ir_in, TAP_IDLE);
	}

	return ERROR_OK;
}

static int mcu_write_dr(struct jtag_tap *tap, uint8_t *dr_in, uint8_t *dr_out,
		int dr_len, int rti)
{
	if (!tap) {
		LOG_ERROR("invalid tap");
		return ERROR_FAIL;
	}

	{
		jtag_add_plain_dr_scan(dr_len, dr_out, dr_in, TAP_IDLE);
	}

	return ERROR_OK;
}

static int mcu_write_ir_u8(struct jtag_tap *tap, uint8_t *ir_in,
		uint8_t ir_out, int ir_len, int rti)
{
	if (ir_len > 8) {
		LOG_ERROR("ir_len overflow, maximum is 8");
		return ERROR_FAIL;
	}

	mcu_write_ir(tap, ir_in, &ir_out, ir_len, rti);

	return ERROR_OK;
}

static int mcu_write_dr_u32(struct jtag_tap *tap, uint32_t *dr_in,
		uint32_t dr_out, int dr_len, int rti)
{
	if (dr_len > 32) {
		LOG_ERROR("dr_len overflow, maximum is 32");
		return ERROR_FAIL;
	}

	mcu_write_dr(tap, (uint8_t *)dr_in, (uint8_t *)&dr_out, dr_len, rti);

	return ERROR_OK;
}

int mcu_execute_queue(void)
{
	return jtag_execute_queue();
}
