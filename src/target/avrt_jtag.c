/************************************************
* Author: Karley W.								* 
* 												*
* File Purpose: uses low-level JTAG programming	*
* commands to accomplish typical debugging 		*
* tasks											*
* 												*
* ATMEGA32 chips have an on-chip debug system, 	*
* but the JTAG instructions to use it are 		*
* proprietary. Instead, the programming 		*
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

