/************************************************
* Author: Karley W.								* 
* 												*
* File Purpose: uses low-level JTAG programming	*
* commands to accomplish typical debugging 		*
* tasks											*
* 												*
* ATMEGA32 chips have an on-chip debug system, 	*
* but the JTAG instructions to use it are 		*
* proprietary. Instead, the chip's programming 	*
* interface will be used to implement read and	*
* write functions.								*
*												*
************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "target.h"
#include "jtag/jtag.h"
#include "avrt_jtag.h"



static int avr_jtag_set_instr_reg(struct avrt_jtag *jtag_info, int new_instr)
{
	struct jtag_tap *tap;
	tap = jtag_info->tap;
	uint32_t current_instruction;
	uint8_t TDI_buffer[sizeof(uint32_t)] = {0};
	uint8_t TDO_buffer[sizeof(uint32_t)];
	
	//check that TAP was correctly initialized
	if (!tap){
		LOG_ERROR("Error in initializing the tap structure");
		return ERROR_FAIL;
		
	}
	
	current_instruction = buf_get_u32(tap->cur_instr, 0, tap->ir_length)

	//If the current instruction is already the one we want to set return
	if (current_instruction == (uint32_t)new_instr) {
		return;
	}
	
	//otherwise, create and initialize fields for the IR scan
	struct scan_field field;
	field.num_bits = tap->ir_length;
	field.out_value = TDI_buffer;
	buf_set_u32(TDI_buffer, 0, field.num_bits, new_instr);
	field.in_value = TDO;
	
	//set instruction through TAP controller
	jtag_add_ir_scan(tap, &field, TAP_IDLE);
	if (jtag_execute_queue() != ERROR_OK) {
		LOG_ERROR("%s: set instruction operation failed", __func__);
		return ERROR_FAIL;
	}


	return ERROR_OK;
}



static int avr_jtag_set_data_reg(struct avrt_jtag *jtag_info, uint8_t *data, tap_state_t end_state)
{
	struct jtag_tap *tap;
	tap = jtag_info->tap;
	uint8_t TDI_buffer[sizeof(uint32_t)] = {0};
	
	//check that TAP was correctly initialized
	if (!tap){
		LOG_ERROR("Error in initializing the tap structure");
		return ERROR_FAIL;
		
	}
	
	//create and initialize fields for the DR scan
	struct scan_field field;
	field.num_bits = tap->ir_length;
	field.out_value = TDI_buffer;
	buf_set_u32(TDI_buffer, 0, field.num_bits, data);
	
	//set current data register through TAP controller
	jtag_add_dr_scan(tap, 1, &field, end_state);
	if (jtag_execute_queue() != ERROR_OK) {
		LOG_ERROR("%s: set data register operation failed", __func__);
		return ERROR_FAIL;
	}


	return ERROR_OK;
}



//note: TDO buffer must have memory allocated to it so it can hold the output
static int avr_jtag_read_data_reg(struct avrt_jtag *jtag_info, uint8_t *TDO_buffer, tap_state_t end_state){
	
	struct jtag_tap *tap;
	tap = jtag_info->tap;
	
	//check that TAP was correctly initialized
	if (!tap){
		LOG_ERROR("Error in initializing the tap structure");
		return ERROR_FAIL;
		
	}
	
	//create and initialize fields for the DR scan
	struct scan_field field;
	field.num_bits = tap->ir_length; //should always be 4
	field.in_value = TDO_buffer;
	
	jtag_add_dr_scan(tap, 1, &field, end_state);
	if (jtag_execute_queue() != ERROR_OK) {
		LOG_ERROR("%s: read data register operation failed", __func__);
		return ERROR_FAIL;
	}
	
}



static int avr_jtag_read_internal_flash(){
	
	
	
}



static int avr_jtag_write_internal_flash(){
	
	
	
}


