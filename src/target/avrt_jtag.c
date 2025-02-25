/**************************************************
* Author: Karley W.                               * 
*                                                 *
* File Purpose: uses low-level JTAG programming   *
* commands to accomplish typical debugging        *
* tasks                                           *
*                                                 *
* ATMEGA32 chips have an on-chip debug system,    *
* but the JTAG instructions to use it are         *
* proprietary. Instead, the chip's programming    *
* interface will be used to implement read and    *
* write functions.                                *
*                                                 *
***************************************************/

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



static int avr_jtag_set_data_reg(struct avrt_jtag *jtag_info, uint8_t *data, int data_reg_len, tap_state_t end_state)
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
	field.num_bits = data_reg_len;
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
static int avr_jtag_read_data_reg(struct avrt_jtag *jtag_info, uint8_t *TDO_buffer, int data_reg_len, tap_state_t end_state){
	
	struct jtag_tap *tap;
	tap = jtag_info->tap;
	
	//check that TAP was correctly initialized
	if (!tap){
		LOG_ERROR("Error in initializing the tap structure");
		return ERROR_FAIL;
		
	}
	
	//create and initialize fields for the DR scan
	struct scan_field field;
	//will vary based on what register is chosen
	field.num_bits = data_reg_len; 
	field.in_value = TDO_buffer;
	
	jtag_add_dr_scan(tap, 1, &field, end_state);
	if (jtag_execute_queue() != ERROR_OK) {
		LOG_ERROR("%s: read data register operation failed", __func__);
		return ERROR_FAIL;
	}
	
}



//command should be the final bit sequence that gets passed to TDI (do bit operations before calling this function)
static int* avr_jtag_exec_prog_command(struct avrt_jtag *jtag_info, int* command, int command_len){
	
	uint16_t command_output[command_len] = {0};
	uint8_t TDI_buffer[sizeof(uint32_t)] = {0};
	
	for(int i = 0; i < command_len; i++){
		
		//put the next bit sequence in TDI_buffer; ea. sequence is 16 bits long
		buf_set_u32(TDI_buffer, 0, 32, command[i]);
		//set the command
		avr_jtag_set_data_reg(jtag_info, TDI_buffer, PROG_COMMAND_LEN, TAP_IDLE);
		//read command output
		avr_jtag_read_data_reg(jtag_info, command_output, PROG_COMMAND_LEN, TAP_IDLE);
		if (jtag_execute_queue() != ERROR_OK) {
			LOG_ERROR("%s: execute program command operation failed", __func__);	
		}
		
	}
	
	return command_output;
	
}


//function for entering JTAG programming mode for ATMEGA32A
static int avr_jtag_enter_prog_mode(struct avrt_jtag *jtag_info){
	//toggle reset
	avr_jtag_set_instr_reg(jtag_info, ATMEGA32_AVR_RESET);
	avr_jtag_set_data_reg(jtag_info, HOLD_RESET, RESET_LEN, TAP_IDLE)
	//toggle program enable
	avr_jtag_set_data_reg(jtag_info, ATMEGA32_PROG_ENABLE);
	avr_jtag_set_data_reg(jtag_info, PROG_ENABLE_SIG, PROG_ENABLE_LEN, TAP_IDLE);
	//call program command instruction
	avr_jtag_set_instr_reg(jtag_info, ATMEGA32_PROG_COMMANDS);
	
}



//function for exiting programming mode
static int avr_jtag_exit_prog_mode(struct avrt_jtag *jtag_info){
	//call program command instruction; execute nop command
	avr_jtag_set_instr_reg(jtag_info, ATMEGA32_PROG_COMMANDS);
	avr_jtag_set_data_reg(jtag_info, NO_OP, PROG_COMMAND_LEN, TAP_IDLE)
	//remove program enable signature
	avr_jtag_set_data_reg(jtag_info, ATMEGA32_PROG_ENABLE);
	avr_jtag_set_data_reg(jtag_info, PROG_DISABLE, PROG_ENABLE_LEN, TAP_IDLE);
	//come out of reset
	avr_jtag_set_instr_reg(jtag_info, ATMEGA32_AVR_RESET);
	avr_jtag_set_data_reg(jtag_info, RELEASE_RESET, RESET_LEN, TAP_IDLE)
	
	
}



//function for reading two bytes of flash; high byte and low byte
//all setup required (reset, prog enable) is handled internally
static int avr_jtag_read_internal_flash(){
	uint8_t TDI_buffer[sizeof(uint32_t)] = {0};
	
	
}



static int avr_jtag_write_internal_flash(){
	
	
	
}


