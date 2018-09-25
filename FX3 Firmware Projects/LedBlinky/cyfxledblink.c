/*
## Cypress USB 3.0 Platform source file (cyfxledblink.c)
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

#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3error.h"
#include "cyu3gpio.h"
#include "cyu3uart.h"
#include "cyu3utils.h"
#include "cyfxledblink.h"

CyU3PThread gpioThread;                  /* Application thread object. */
CyU3PThread dbgThread;                   /* Debug thread object. */

static void
CyFxGpioInit (
        void)
{
    CyU3PGpioClock_t        gpioClock;
    CyU3PGpioSimpleConfig_t testConf = {CyFalse, CyTrue, CyTrue, CyFalse, CY_U3P_GPIO_NO_INTR};

    /* Initialize the GPIO block and set GPIO 54 as an output. */
    gpioClock.fastClkDiv = 2;
    gpioClock.slowClkDiv = 32;
    gpioClock.simpleDiv  = CY_U3P_GPIO_SIMPLE_DIV_BY_16;
    gpioClock.clkSrc     = CY_U3P_SYS_CLK_BY_2;
    gpioClock.halfDiv    = 0;
    CyU3PGpioInit (&gpioClock, NULL);

    /* Override GPIO54 as a simple GPIO. */
    CyU3PDeviceGpioOverride (54, CyTrue);

    CyU3PGpioSetSimpleConfig (54, &testConf);
}

/* Initialize the debug module with UART. */
CyU3PReturnStatus_t
CyFxDebugInit (
        void)
{
    CyU3PUartConfig_t uartConfig;
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    /* Initialize and configure the UART for logging. */
    status = CyU3PUartInit ();
    if (status != CY_U3P_SUCCESS)
    {
        return status;
    }

    CyU3PMemSet ((uint8_t *)&uartConfig, 0, sizeof (uartConfig));
    uartConfig.baudRate = CY_U3P_UART_BAUDRATE_115200;
    uartConfig.stopBit  = CY_U3P_UART_ONE_STOP_BIT;
    uartConfig.parity   = CY_U3P_UART_NO_PARITY;
    uartConfig.txEnable = CyTrue;
    uartConfig.rxEnable = CyFalse;
    uartConfig.flowCtrl = CyFalse;
    uartConfig.isDma    = CyTrue;
    status = CyU3PUartSetConfig (&uartConfig, NULL);
    if (status != CY_U3P_SUCCESS)
    {
        return status;
    }

    /* Set the dma for an infinity transfer */
    status = CyU3PUartTxSetBlockXfer (0xFFFFFFFF);
    if (status != CY_U3P_SUCCESS)
    {
        return status;
    }

    /* Start the debug module for printing log messages. */
    status = CyU3PDebugInit (CY_U3P_LPP_SOCKET_UART_CONS, 8);

    if (status == CY_U3P_SUCCESS)
        CyU3PDebugPreamble (CyFalse);

    return status;
}

void
DbgThread_Entry (
        uint32_t input)
{
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;
    uint16_t count = 0;

    /* Initialize the debug interface. */
    status = CyFxDebugInit ();
//    if (status != CY_U3P_SUCCESS)
//    {
//        CyU3PDebugPrint (4, "%x: Application failed to initialize. Error code: %d.\r\n", status);
//        while (1);
//    }

    /* Initialize the GPIO module and set GPIO 54 as the thread activity GPIO for this thread. */
//    CyFxGpioInit ();

//    if (CyU3PThreadSetActivityGpio (&dbgThread, 54) != CY_U3P_SUCCESS)
//        CyU3PDeviceReset (CyFalse);

    for (;;)
    {
        CyU3PThreadSleep (500);

        count++;
        if ((count & 0x0F) == 0)
            CyU3PDebugPrint (2, "Debug Thread: count=%d\r\r\n", count++);
//        if (count == 0)
//            CyU3PDeviceReset (CyFalse);

//        CyU3PBusyWait (60000);
    }
}

void
GpioThread_Entry (
        uint32_t input)
{
    uint32_t total, dbg, gpio, driver;

    for (;;)
    {
        CyU3PThreadSleep (2000);

        total  = CyU3PDeviceGetCpuLoad ();
        dbg    = CyU3PDeviceGetThreadLoad (&dbgThread);
        gpio   = CyU3PDeviceGetThreadLoad (&gpioThread);
        driver = CyU3PDeviceGetDriverLoad ();

        CyU3PDebugPrint (2, "CPU load=%d, dbg thread=%d, gpio thread=%d, drivers=%d\r\r\n",
                total, dbg, gpio, driver);
    }
}

/* Application define function which creates the application threads. */
void
CyFxApplicationDefine (
        void)
{
    void *ptr = NULL;
    uint32_t retThrdCreate = CY_U3P_ERROR_MEMORY_ERROR;

    /* Allocate the memory for the threads and create threads */
    ptr = CyU3PMemAlloc (THREAD_STACK);
    if (ptr != NULL)
    {
        retThrdCreate = CyU3PThreadCreate (
                &dbgThread,                            /* Thread structure. */
                "21:DebugThread",                      /* Thread ID and name. */        
                DbgThread_Entry,                       /* Thread entry function. */
                0,                                     /* Thread input parameter. */
                ptr,                                   /* Pointer to the allocated thread stack. */
                THREAD_STACK,                          /* Allocated thread stack size. */
                THREAD_PRIORITY,                       /* Thread priority. */
                THREAD_PRIORITY,                       /* Thread pre-emption threshold: No preemption. */
                CYU3P_NO_TIME_SLICE,                   /* No time slice. Thread will run until task is
                                                          completed or until the higher priority 
                                                          thread gets active. */
                CYU3P_AUTO_START                       /* Start the thread immediately. */
                );
    }

    /* Check the return code */
    if (retThrdCreate != 0)
    {
        /* Application cannot continue. Loop indefinitely */
        while(1);
    }
#if (0)
    /* Allocate the memory for the threads and create threads */
    ptr = CyU3PMemAlloc (THREAD_STACK);
    if (ptr != NULL)
    {
        retThrdCreate = CyU3PThreadCreate (
                &gpioThread,                           /* Thread structure. */
                "22:GpioThread",                       /* Thread ID and name. */        
                GpioThread_Entry,                      /* Thread entry function. */
                0,                                     /* Thread input parameter. */
                ptr,                                   /* Pointer to the allocated thread stack. */
                THREAD_STACK,                          /* Allocated thread stack size. */
                THREAD_PRIORITY,                       /* Thread priority. */
                THREAD_PRIORITY,                       /* Thread pre-emption threshold: No preemption. */
                CYU3P_NO_TIME_SLICE,                   /* No time slice. Thread will run until task is
                                                          completed or until the higher priority 
                                                          thread gets active. */
                CYU3P_AUTO_START                       /* Start the thread immediately. */
                );
    }
#endif
    /* Check the return code */
    if (retThrdCreate != 0)
    {
        /* Application cannot continue. Loop indefinitely */
        while(1);
    }
}

/*
 * Main function
 */
int
main (void)
{
    CyU3PIoMatrixConfig_t io_cfg;
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    /* Initialize the device */
    status = CyU3PDeviceInit (NULL);
    if (status != CY_U3P_SUCCESS)
    {
        goto handle_fatal_error;
    }

    /* Initialize the caches. Enable both Instruction and Data caches. */
    status = CyU3PDeviceCacheControl (CyTrue, CyTrue, CyTrue);
    if (status != CY_U3P_SUCCESS)
    {
        goto handle_fatal_error;
    }

    /* Configure the IO matrix for the device. */
    CyU3PMemSet ((uint8_t *)&io_cfg, 0, sizeof(io_cfg));
    io_cfg.isDQ32Bit = CyFalse;
    io_cfg.s0Mode = CY_U3P_SPORT_INACTIVE;
    io_cfg.s1Mode = CY_U3P_SPORT_INACTIVE;
    io_cfg.useUart   = CyTrue;
    io_cfg.useI2C    = CyFalse;
    io_cfg.useI2S    = CyFalse;
    io_cfg.useSpi    = CyFalse;
    io_cfg.lppMode   = CY_U3P_IO_MATRIX_LPP_UART_ONLY;
    /* No GPIOs are enabled. */
    io_cfg.gpioSimpleEn[0]  = 0;
    io_cfg.gpioSimpleEn[1]  = 0;
    io_cfg.gpioComplexEn[0] = 0;
    io_cfg.gpioComplexEn[1] = 0;
    status = CyU3PDeviceConfigureIOMatrix (&io_cfg);
    if (status != CY_U3P_SUCCESS)
    {
        goto handle_fatal_error;
    }

    /* This is a non returnable call for initializing the RTOS kernel */
    CyU3PKernelEntry ();

    /* Dummy return to make the compiler happy */
    return 0;

handle_fatal_error:

    /* Cannot recover from this error. */
    while (1);
}

/* [] */

