/*
 ## Cypress USB 3.0 Platform header file (cyfx_rtos_eg.h)
 ## ===========================
 ##
 ##  Copyright Cypress Semiconductor Corporation, 2010-2011,
 ##  All Rights Reserved
 ##  UNPUBLISHED, LICENSED SOFTWARE.
 ##
 ##  CONFIDENTIAL AND PROPRIETARY INFORMATION
 ##  WHICH IS THE PROPERTY OF CYPRESS.
 ##
 ##  Use of this file is governed
 ##  by the license agreement included in the file
 ##
 ##     <install>/license/license.txt
 ##
 ##  where <install> is the Cypress software
 ##  installation root directory path.
 ##
 ## ===========================
*/

/* This file contains the externants used by the flash programmer application. */

#ifndef _INCLUDED_CYFX_RTOS_EG_H_
#define _INCLUDED_CYFX_RTOS_EG_H_

#include "cyu3types.h"
#include "cyu3externcstart.h"

#define THREAD_STACK       (0x0200)    /* App thread stack size */
#define THREAD_PRIORITY    (8)         /* App thread priority */

#include "cyu3externcend.h"

#endif /* _INCLUDED_CYFX_RTOS_EG_H_ */

/*[]*/
