/*
 * Support.c - this module contains useful routines
 *
 *  Created on: Feb 18, 2014
 *      Author: John
 */

#include "Application.h"
#include <stdarg.h>		// For argument processing
#include <string.h>

extern uint8_t* CyU3PDebugIntToStr (uint8_t* convertedString, uint32_t num, uint8_t base);
extern CyU3PReturnStatus_t I2C_DebugPrint(uint8_t Priority, char* Message, ...);

const char* ErrorCodeTextLookupTable[] = {
	"SUCCESS",				// 0
	"DELETED",				// 1
	"BAD_POOL",				// 2
	"BAD_POINTER",			// 3
	"INVALID_WAIT",			// 4
	"BAD_SIZE",				// 5
	"BAD_EVENT_GROUP",		// 6
	"NO_EVENTS",			// 7
	"BAD_OPTION",			// 8
	"BAD_QUEUE",			// 9
	"QUEUE_EMPTY",			// 10
	"QUEUE_FULL",			// 11
	"BAD_SEMAPHORE",		// 12
	"GET_SEMAPHORE_FAIL",	// 13
	"BAD_THREAD",			// 14
	"BAD_PRIORITY",			// 15
	"MEMORY_ERROR",			// 16
	"DELETE_FAILED",		// 17
	"RESUME_FAILED",		// 18
	"INVALID_CALLER",		// 19
	"SUSPEND_FAILED",		// 20
	"BAD_TIMER",			// 21
	"BAD_TICK",				// 22
	"ACTIVATE_FAILED",		// 23
	"BAD_THRESHOLD",		// 24
	"SUSPEND_LIFTED",		// 25
	"WAIT_ABORTED",			// 26
	"WAIT_ABORT_FAILED",	// 27
	"BAD_MUTEX",			// 28
	"MUTEX_FAILURE",		// 29
	"MUTEX_PUT_FAILED",		// 30
	"INHERIT_FAILED",		// 31
	"NOT_IDLE",				// 32
	0,0,0,0,0,0,0,0,		// 33-40
	0,0,0,0,0,0,0,0,		// 41-48
	0,0,0,0,0,0,0,0,		// 49-56
	0,0,0,0,0,0,0,			// 57-63
	"BAD_ARGUMENT",				// 64 = 0x40
	"NULL_POINTER",				// 65
	"NOT_STARTED",				// 66
	"ALREADY_STARTED",			// 67
	"NOT_CONFIGURED",			// 68
	"TIMEOUT",					// 69
	"NOT_SUPPORTED",			// 70
	"INVALID_SEQUENCE",			// 71
	"ABORTED",					// 72
	"DMA_FAILURE",				// 73
	"FAILURE",					// 74
	"BAD_INDEX",				// 75
	"BAD_ENUM_METHOD",			// 76
	"INVALID_CONFIGURATION",	// 77
	"CHANNEL_CREATE_FAILED",	// 78
	"CHANNEL_DESTROY_FAILED",	// 79
	"BAD_DESCRIPTOR_TYPE",		// 80
	"XFER_CANCELLED",			// 81
	"FEATURE_NOT_ENABLED",		// 82
	"STALLED",					// 83
	"BLOCK_FAILURE",			// 84
	"LOST_ARBITRATION",			// 85
	"STANDBY_FAILED",			// 86
	0,0,0,0,0,0,0,0,			// 87-94
	0,							// 95
	"INVALID_VOLTAGE_RANGE",	// 96 = 0x60
	"CARD_WRONG_RESPONSE",		// 97
	"UNSUPPORTED_CARD",			// 98
	"CARD_WRONG_STATE",			// 99
	"CMD_NOT_SUPPORTED",		// 100
	"CRC",						// 101
	"INVALID_ADDR",				// 102
	"INVALID_UNIT",				// 103
	"INVALID_DEV",				// 104
	"ALREADY_PARTITIONED",		// 105
	"NOT_PARTITIONED",			// 106
	"BAD_PARTITION",			// 107
	"READ_WRITE_ABORTED",		// 108
	"WRITE_PROTECTED",			// 109
	"SIB_INIT",					// 110
	"CARD_LOCKED",				// 111
	"CARD_LOCK_FAILURE",		// 112
	"CARD_FORCE_ERASE",			// 113
	"INVALID_BLOCKSIZE",		// 114
	"INVALID_FUNCTION",			// 115
	"TUPLE_NOT_FOUND",			// 116
	"IO_ABORTED",				// 117
	"IO_SUSPENDED",				// 118
	"ILLEGAL_CMD",				// 119
	"SDIO_UNKNOWN",				// 120
	"BAD_CMD_ARG",				// 121
	"UNINITIALIZED_FUNCTION",	// 122
	"CARD_NOT_ACTIVE",			// 123
	"DEVICE_BUSY",				// 124
	"NO_METADATA",				// 125
	"CARD_UNHEALTHY",			// 126
	"MEDIA_FAILURE",			// 127
};

extern CyBool_t glDebugTxEnabled;

CyU3PThread ApplicationThread;			// Handle to my Application Thread
CyBool_t glIsApplicationActive;			// Set true once device is enumerated

char* ErrorCodeText(CyU3PReturnStatus_t Status)
{
	char* TextPtr;
	// Handle exceptions first
	if (Status == 0xFF) return "OPERATION_DISABLED";
	if (Status == 0xFE) return "NO_REENUM_REQUIRED";
	if (Status > 127) return "INVALID_ERROR_CODE";
	// This BIG text array is in Support.c
	TextPtr = (char*)ErrorCodeTextLookupTable[Status];
	if (TextPtr) return TextPtr;
	return "INVALID_ERROR_CODE";
}

void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status)
// In this initial debugging stage I stall on an un-successful system call, else I display progress
{
	if (glDebugTxEnabled)				// Need to wait until ConsoleOut is enabled
	{
		if (Status == CY_U3P_SUCCESS)
		{
			DebugPrint(8, "\r\n%s Successful", StringPtr);
			return;
		}
		// else hang here
		DebugPrint(4, "\r\n%s failed, %d = CY_U3P_ERROR_%s\r\n", StringPtr, Status, ErrorCodeText(Status));
#if 0
		while (1)
		{
			DebugPrint(4, "?");
			CyU3PThreadSleep(1000);
		}
#endif
	}
}


// Use a routine cut and paste from cyU3debug.c (it was declared static so was not callable)
CyU3PReturnStatus_t MyDebugSNPrint (
        uint8_t  *debugMsg,
        uint16_t *length,
        char     *message,
        va_list   argp)
{
    uint8_t  *string_p;
    uint8_t  *argStr = NULL;
    CyBool_t  copyReqd = CyFalse;
    uint16_t  i = 0, j, maxLength = *length;
    int32_t   intArg;
    uint32_t  uintArg;
    uint8_t   convertedString[11];

    if (debugMsg == 0)
        return CY_U3P_ERROR_BAD_ARGUMENT;

    /* Parse the string and copy into the buffer for sending out. */
    for (string_p = (uint8_t *)message; (*string_p != '\0'); string_p++)
    {
        if (i >= (maxLength - 2))
            return CY_U3P_ERROR_BAD_ARGUMENT;

        if (*string_p != '%')
        {
            debugMsg[i++] = *string_p;
            continue;
        }

        string_p++;
        switch (*string_p)
        {
        case '%' :
            {
                debugMsg[i++] = '%';
            }
            break;

        case 'c' :
            {
                debugMsg[i++] = (uint8_t)va_arg (argp, int32_t);
            }
            break;

        case 'd' :
            {
                intArg = va_arg (argp, int32_t);
                if (intArg < 0)
                {
                    debugMsg[i++] = '-';
                    intArg = -intArg;
                }

                argStr = CyU3PDebugIntToStr (convertedString, intArg, 10);
                copyReqd = CyTrue;
            }
            break;

        case 's':
            {
                argStr = va_arg (argp, uint8_t *);
                copyReqd = CyTrue;
            }
            break;

        case 'u':
            {
                uintArg = va_arg (argp, uint32_t);
                argStr = CyU3PDebugIntToStr (convertedString, uintArg, 10);
                copyReqd = CyTrue;
            }
            break;

        case 'X':
        case 'x':
            {
                uintArg = va_arg (argp, uint32_t);
                argStr = CyU3PDebugIntToStr (convertedString, uintArg, 16);
                copyReqd = CyTrue;
            }
            break;

        default:
            return CY_U3P_ERROR_BAD_ARGUMENT;
        }

        if (copyReqd)
        {
            j = (uint16_t)strlen ((char *)argStr);
            if (i >= (maxLength - j - 1))
                return CY_U3P_ERROR_BAD_ARGUMENT;
            strcpy ((char *)(debugMsg + i), (char *)argStr);
            i += j;
            copyReqd = CyFalse;
        }
    }

    /* NULL-terminate the string. There will always be space for this. */
    debugMsg[i] = '\0';
    *length     = i;

    return CY_U3P_SUCCESS;
}


