/****************************************************
 * Author: Karley W.								*
 * kwagu14@lsu.edu									*
 *													*
 * All codes were discovered from the ATMEGA32A		*
 * datasheet 										*
 ****************************************************/
 
 struct avrt_jtag {
	struct jtag_tap *tap;
};

/* 
	JTAG Instruction codes:
	
	These are instructions that can be put into the instruction register 
	through jtag_add_ir_scan()

*/

/**************** Programming Instructions *********************/
#define ATMEGA32_AVR_RESET 			0xC
#define ATMEGA32_PROG_ENABLE 		0x4
#define ATMEGA32_PROG_COMMANDS 		0x5
#define ATMEGA32_PROG_PAGELOAD 		0x6
#define ATMEGA32_PROG_PAGEREAD 		0x7



/* 
	Register values: 
	
	These are potential values that can go within the data registers using
	jtag_add_dr_scan()
*/

/*************** Programming Enable *********************/
#define PROG_ENABLE_SIG 			0xA370

/******************* Reset *************************/
#define HOLD_RESET					0x1
#define RELEASE_RESET				0x0			


/*************** Program Command *********************/
/*
	These hex codes represent bit sequences that should be shifted in 
	through TDI; the results of previous command sequences are shifted 
	out through TDO)
	
	Note: some sequences have to be OR'ed with input to get the final
	sequence that should be sent out through TDI; these are marked with
	comments

*/

const int CHIP_ERASE[] = {0x2380, 0x3180, 0x3380, 0x3380};
const int POLL_CHIP_ERASE = 0x3380;

const int ENTER_FLASH_WRITE = 0x2310;
//OR with high bits of address
const int LOAD_ADDR_BYTE_HIGH = 0x700;
//OR with low bits of address
const int LOAD_ADDR_BYTE_LOW = 0x300;
//OR with low bits of data
const int LOAD_DATA_BYTE_LOW = 0x1300;
//OR with high bits of data
const int LOAD_DATA_BYTE_HIGH = 0x1700;
const int LATCH_DATA[] = {0x3700, 0x7700, 0x3700};
const int WRITE_FLASH_PAGE[] = {0x3700, 0x3500, 0x3700, 0x3700};
const int POLL_WRITE_FLASH_PAGE = 0x3700;

const int ENTER_FLASH_READ = 0x2302;
//low byte output appears on TDO when shifting the second sequence
//high byte output appears when shifting the final sequence
const int READ_FLASH_DATA[] = {0x3200, 0x3600, 0x3700};

const int ENTER_EEPROM_WRITE = 0x2311;
//OR with data
const int LOAD_DATA_BYTE = 0x1300;
const int WRITE_EEPROM_PAGE[] = {0x3300, 0x3100, 0x3300, 0x3300};
const int POLL_EEPROM_WRITE_PAGE = 0x3300;

const int ENTER_EEPROM_READ = 0x2303;
//OR first bit sequence with low address byte; also:
//output will appear on TDO when shifting the last 8 bits of final sequence 
const int READ_EEPROM_BYTE[] = {0x3300, 0x3200, 0x3300};

const int ENTER_FUSE_WRITE = 0x2340;
//OR with data
const int LOAD_FUSE_WRITE_DATA_HIGH = 0x1300;
const int WRITE_FUSE_HIGH[] = {0x3700, 0x3500, 0x3700, 0x1B80};
const int POLL_WRITE_HIGH = 0x3700;
//OR with data
const int LOAD_FUSE_WRITE_DATA_LOW = 0x1300;
const int WRITE_FUSE_LOW[] = {0x3300, 0x3100, 0x3300, 0x3300};
const int POLL_WRITE_LOW = 0x3300;

const int ENTER_LOCK_BIT_WRITE = 0x2320;
//OR with 6-bit data
const int LOAD_LOCK_DATA_BYTE = 0x13C0;
const int WRITE_LOCK_BITS[] = {0x3300, 0x3100, 0x3300, 0x3300};
const int POLL_WRITE_LOCK_BITS = 0x3300; 

const int ENTER_FUSE_AND_LOCK_READ = 0x2304;
const int READ_FUSE_HIGH_BYTE[] = {0x3E00, 0x3F00};
const int READ_FUSE_LOW_BYTE[] = {0x3200, 0x3300}; 
const int READ_LOCK_BITS[] = {0x3600, 0x3700};
const int READ_FUSES_AND_LOCK_BITS[] = {0x3E00, 0x3200, 0x3600, 0x3700};

const int ENTER_SIG_BYTE_READ = 0x2308; 
//OR with low bits of addr
const int LOAD_ADDR_BYTE = 0x300;
const int READ_SIG_BYTE[] = {0x3200, 0x3300};

const int ENTER_CALIB_BYTE_READ = 0x2308;
const int READ_CALIB_BYTE[] = {0x3600, 0x3700};
const int NO_OP[] = {0x2300, 0x3300};


