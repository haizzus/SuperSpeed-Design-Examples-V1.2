
/* This file contains the externals used by the Keyboard application. */

#ifndef _INCLUDED_KEYBOARD_H_
#define _INCLUDED_KEYBOARD_H_

#include "cyu3types.h"
#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3gpio.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyu3uart.h"
#include "cyu3i2c.h"
#include "cyu3externcstart.h"

#define Elements(X)		(sizeof(X)/sizeof(X[0]))

#define APPLICATION_THREAD_STACK	(0x1000)
#define APPLICATION_THREAD_PRIORITY	(8)
#define CY_FX_USBI2C_I2C_BITRATE	(400000)
#define DeviceAddress				(0x56)
#define REGISTER_MODE				(0)
#define CY_FX_CPU_PRODUCER_SOCKET	(CY_U3P_CPU_SOCKET_PROD)
#define CY_FX_CPU_CONSUMER_SOCKET	(CY_U3P_CPU_SOCKET_CONS)

#define CPLD_RESET					(27)

#include "cyu3externcend.h"

#endif /* _INCLUDED_KEYBOARD_H_ */

