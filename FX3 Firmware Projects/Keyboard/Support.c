/*
 * Support.c - this module contains useful routines
 *
 *  Created on: Feb 18, 2014
 *      Author: John
 */

#include "Application.h"

extern CyBool_t glDebugTxEnabled;

CyU3PEvent DisplayEvent;		// Used to display events in the background

// For Debug and education display the name of the Event
const char* EventName[] = {
// First 23 are from typedef enum CyU3PUsbEventType_t in cyu3usb.h
	"CONNECT",					// 0
	"DISCONNECT",				// 1
	"SUSPEND",					// 2
	"RESUME",					// 3
	"RESET",					// 4
	"SET_CONFIGURATION",		// 5
	"CHANGE_SPEED",				// 6
	"SET_INTERFACE",			// 7
	"SET_EXIT_LATENCY",			// 8
	"SOF_ITP",					// 9
	"USER_EP0_XFER_COMPLETE",	// 10
	"VBUS_VALID",				// 11
	"VBUS_REMOVED",				// 12
	"HOSTMODE_CONNECT",			// 13
	"HOSTMODE_DISCONNECT",		// 14
	"OTG_CHANGE",				// 15
	"OTG_VBUS_CHG",				// 16
	"OTG_SRP",					// 17
	"EP_UNDERRUN",				// 18
	"LINK_RECOVERY",			// 19
	"USB3_LINKFAIL",			// 20
	"SS_COMP_ENTRY",			// 21
	"SS_COMP_EXIT",				// 22
	"23",						// 23
	"24",						// 24
	"25",						// 25
	"26",						// 26
	"27",						// 27
	"Sent Report Descriptor",	// 28
	"Ack to Set Idle",			// 29
	"Set LEDs",					// 30
	"Out of Range"				// 31
};

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
			DebugPrint(7, "\r\n%s Successful", StringPtr);
			return;
		}
		// else hang here
		DebugPrint(4, "\r\n%s failed, %d = CY_U3P_ERROR_%s\r\n", StringPtr, Status, ErrorCodeText(Status));
		while (1)
		{
			DebugPrint(4, "?");
			CyU3PThreadSleep(1000);
		}
	}
}

uint32_t BitPosition(uint32_t Value)
{
	if (Value > 32) return 31;
	else return (1<<Value);
}

void BackgroundPrint(uint32_t TimeToWait)
{
	uint32_t ActualEvents, i, Count, Limit;
	CyU3PReturnStatus_t Status;
	if (TimeToWait > 10)
	{
		// Check more often so that the events are better displayed in the order that they appeared
		Limit = TimeToWait/10;
		TimeToWait = 10;
	}
	else Limit = 1;
	// Check to see if any Events posted and, if they did, display them
	for (Count = 0; Count < Limit; Count++)
		{
		ActualEvents = 0;
		Status = CyU3PEventGet(&DisplayEvent, 0xFFFF, TX_OR_CLEAR, &ActualEvents, TimeToWait);
		if (Status != TX_NO_EVENTS)
		{
			for (i = 0; i<32; i++) if (ActualEvents & (1<<i)) DebugPrint(4, "\r\nEvent(%d) received = '%s' ", i, EventName[i]);
		}
	}
}
