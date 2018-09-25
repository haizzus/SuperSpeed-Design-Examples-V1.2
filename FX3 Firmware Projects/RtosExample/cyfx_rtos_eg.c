/*
## Cypress USB 3.0 Platform source file (cyfx_rtos_eg.c)
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
#include "cyfx_rtos_eg.h"

CyU3PThread Thread1;                    /* Application thread number 1. */
CyU3PThread Thread2;                    /* Application thread number 2. */
CyU3PThread Thread3;                    /* Application thread number 3. */
CyU3PThread Thread4;                    /* Application thread number 4. */

CyU3PMutex      AppMutex;               /* Mutex used for testing. */
CyU3PSemaphore  AppSema;                /* Semaphore used for testing. */
CyU3PEvent      AppEvent;               /* Event group used for testing. */

#define APP_SEMA_COUNT                  (4)             /* Semaphore count. */
#define APP_EVENT_FLAG                  (1)             /* Event flag that the thread waits for. */

#define THREAD_ACTIVITY_GPIO            (54)            /* GPIO used for thread activity indication. */
#define MUTEX_ACTIVITY_GPIO             (53)            /* GPIO used for Mutex status indication. */
#define SEMAPHORE_ACTIVITY_GPIO         (52)            /* GPIO used for Semaphore status indication. */
#define EVENT_ACTIVITY_GPIO             (51)            /* GPIO used for Event Group status indication. */

/* Initialize the GPIO module and enable all indicator pins as output pins with the default value of 0. */
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

    /* Override and configure the thread status GPIO. */
    CyU3PDeviceGpioOverride (THREAD_ACTIVITY_GPIO, CyTrue);
    CyU3PGpioSetSimpleConfig (THREAD_ACTIVITY_GPIO, &testConf);

    /* Override and configure the mutex status GPIO. */
    CyU3PDeviceGpioOverride (MUTEX_ACTIVITY_GPIO, CyTrue);
    CyU3PGpioSetSimpleConfig (MUTEX_ACTIVITY_GPIO, &testConf);

    /* Override and configure the semaphore status GPIO. */
    CyU3PDeviceGpioOverride (SEMAPHORE_ACTIVITY_GPIO, CyTrue);
    CyU3PGpioSetSimpleConfig (SEMAPHORE_ACTIVITY_GPIO, &testConf);

    /* Override and configure the event status GPIO. */
    CyU3PDeviceGpioOverride (EVENT_ACTIVITY_GPIO, CyTrue);
    CyU3PGpioSetSimpleConfig (EVENT_ACTIVITY_GPIO, &testConf);
}

/* Initialize the logger module with UART. */
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

    /* Set the dma for an inifinity transfer */
    status = CyU3PUartTxSetBlockXfer (0xFFFFFFFF);
    if (status != CY_U3P_SUCCESS)
    {
        return status;
    }

    /* Start the debug module for printing log messages. */
    status = CyU3PDebugInit (CY_U3P_LPP_SOCKET_UART_CONS, 8);

    /* Disable the message context preamble as we will be using a standard serial console. */
    if (status == CY_U3P_SUCCESS)
        CyU3PDebugPreamble (CyFalse);

    return status;
}

/* Thread1: Keep suspending/resuming the thread. */
void
Thread1_Entry (
        uint32_t input)
{
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;
    uint16_t count = 0;

    /* Initialize the debug interface. */
    status = CyFxDebugInit ();
    if (status != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "%x: Application failed to initialize. Error code: %d.\r\n", status);
        while (1);
    }

    /* Initialize the GPIO module and register the status indicator GPIOs for each of the objects. */
    CyFxGpioInit ();

    /* Thread status GPIO registration. */
    status = CyU3PThreadSetActivityGpio (&Thread1, THREAD_ACTIVITY_GPIO);
    if (status != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (2, "Error: Failed to register thread monitoring GPIO, status = %d\r\r\n", status);
    }

    /* Mutex status GPIO registration. */
    status = CyU3PMutexSetActivityGpio (&AppMutex, MUTEX_ACTIVITY_GPIO);
    if (status != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (2, "Error: Failed to register mutex monitoring GPIO, status = %d\r\r\n", status);
    }

    /* Semaphore status GPIO registration. */
    status = CyU3PSemaphoreSetActivityGpio (&AppSema, SEMAPHORE_ACTIVITY_GPIO);
    if (status != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (2, "Error: Failed to register semaphore monitoring GPIO, status = %d\r\r\n", status);
    }

    /* Event group status GPIO registration. */
    status = CyU3PEventSetActivityGpio (&AppEvent, EVENT_ACTIVITY_GPIO);
    if (status != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (2, "Error: Failed to register event group monitoring GPIO, status = %d\r\r\n", status);
    }

    for (;;)
    {
        CyU3PThreadSleep (500);

        count++;
        if ((count & 0x0F) == 0)
            CyU3PDebugPrint (2, "Thread1: count=%d\r\r\n", count++);
        if (count == 0)
            CyU3PDeviceReset (CyFalse);

        CyU3PBusyWait (60000);
    }
}

/* Thread2: Print CPU load values, and perform mutex get/put operations. */
void
Thread2_Entry (
        uint32_t input)
{
    uint32_t total, thr1, thr2, driver;
    CyBool_t isGet = CyTrue;

    for (;;)
    {
        CyU3PThreadSleep (2000);

        total  = CyU3PDeviceGetCpuLoad ();
        thr1   = CyU3PDeviceGetThreadLoad (&Thread1);
        thr2   = CyU3PDeviceGetThreadLoad (&Thread2);
        driver = CyU3PDeviceGetDriverLoad ();

        CyU3PDebugPrint (2, "Thread2: CPU load=%d, thread1=%d, thread2=%d, drivers=%d\r\r\n",
                total, thr1, thr2, driver);

        /* Toggle the mutex status. */
        if (isGet)
        {
            CyU3PMutexGet (&AppMutex, CYU3P_WAIT_FOREVER);
            isGet = CyFalse;
        }
        else
        {
            CyU3PMutexPut (&AppMutex);
            isGet = CyTrue;
        }
    }
}

/* Thread 3: Perform Semaphore Get/Put operations. */
void
Thread3_Entry (
        uint32_t input)
{
    uint32_t count = APP_SEMA_COUNT;

    for (;;)
    {
        CyU3PThreadSleep (1000);

        if (count > 0)
        {
            count--;
            CyU3PSemaphoreGet (&AppSema, CYU3P_WAIT_FOREVER);
        }
        else
        {
            while (count < APP_SEMA_COUNT)
            {
                CyU3PSemaphorePut (&AppSema);
                count++;
            }

            CyU3PDebugPrint (2, "Thread3\r\r\n");
        }
    }
}

/* Thread 4: Perform event group set/get operations. */
void
Thread4_Entry (
        uint32_t input)
{
    uint32_t flag;
    CyBool_t isSet = CyTrue;

    for (;;)
    {
        CyU3PThreadSleep (3000);

        if (isSet)
        {
            CyU3PEventSet (&AppEvent, APP_EVENT_FLAG, CYU3P_EVENT_OR);
            isSet = CyFalse;
        }
        else
        {
            CyU3PEventGet (&AppEvent, APP_EVENT_FLAG, CYU3P_EVENT_AND_CLEAR, &flag, CYU3P_WAIT_FOREVER);
            CyU3PDebugPrint (2, "Thread4: flag=%d\r\r\n", flag);
            isSet = CyTrue;
        }
    }
}

/* Application define function which creates the application threads. */
void
CyFxApplicationDefine (
        void)
{
    void *ptr    = NULL;
    uint32_t ret = CY_U3P_ERROR_MEMORY_ERROR;

    /* Create the mutex object. */
    ret = CyU3PMutexCreate (&AppMutex, CYU3P_NO_INHERIT);
    if (ret != 0)
    {
        goto InitFail;
    }

    /* Create the semaphore object. */
    ret = CyU3PSemaphoreCreate (&AppSema, APP_SEMA_COUNT);
    if (ret != 0)
    {
        goto InitFail;
    }

    /* Create the event group. */
    ret = CyU3PEventCreate (&AppEvent);
    if (ret != 0)
    {
        goto InitFail;
    }

    /* Create thread number 1. */
    ptr = CyU3PMemAlloc (THREAD_STACK);
    if (ptr != NULL)
    {
        ret = CyU3PThreadCreate (
                &Thread1,                              /* Thread structure. */
                "21:App Thread 1",                     /* Thread ID and name. */        
                Thread1_Entry,                         /* Thread entry function. */
                0,                                     /* Thread input parameter. */
                ptr,                                   /* Pointer to the allocated thread stack. */
                THREAD_STACK,                          /* Allocated thread stack size. */
                THREAD_PRIORITY,                       /* Thread priority. */
                THREAD_PRIORITY,                       /* Thread pre-emption threshold: No preemption. */
                CYU3P_NO_TIME_SLICE,                   /* No time slice. */
                CYU3P_AUTO_START                       /* Start the thread immediately. */
                );
    }
    else
    {
        ret = CY_U3P_ERROR_MEMORY_ERROR;
    }

    /* Check the return code */
    if (ret != 0)
    {
        goto InitFail;
    }

    /* Create thread number 2. */
    ptr = CyU3PMemAlloc (THREAD_STACK);
    if (ptr != NULL)
    {
        ret = CyU3PThreadCreate (
                &Thread2,                              /* Thread structure. */
                "22:App Thread 2",                     /* Thread ID and name. */        
                Thread2_Entry,                         /* Thread entry function. */
                0,                                     /* Thread input parameter. */
                ptr,                                   /* Pointer to the allocated thread stack. */
                THREAD_STACK,                          /* Allocated thread stack size. */
                THREAD_PRIORITY,                       /* Thread priority. */
                THREAD_PRIORITY,                       /* Thread pre-emption threshold: No preemption. */
                CYU3P_NO_TIME_SLICE,                   /* No time slice. */
                CYU3P_AUTO_START                       /* Start the thread immediately. */
                );
    }
    else
    {
        ret = CY_U3P_ERROR_MEMORY_ERROR;
    }

    /* Check the return code */
    if (ret != 0)
    {
        goto InitFail;
    }

    /* Create thread number 3. */
    ptr = CyU3PMemAlloc (THREAD_STACK);
    if (ptr != NULL)
    {
        ret = CyU3PThreadCreate (
                &Thread3,                              /* Thread structure. */
                "23:App Thread 3",                     /* Thread ID and name. */        
                Thread3_Entry,                         /* Thread entry function. */
                0,                                     /* Thread input parameter. */
                ptr,                                   /* Pointer to the allocated thread stack. */
                THREAD_STACK,                          /* Allocated thread stack size. */
                THREAD_PRIORITY,                       /* Thread priority. */
                THREAD_PRIORITY,                       /* Thread pre-emption threshold: No preemption. */
                CYU3P_NO_TIME_SLICE,                   /* No time slice. */
                CYU3P_AUTO_START                       /* Start the thread immediately. */
                );
    }
    else
    {
        ret = CY_U3P_ERROR_MEMORY_ERROR;
    }

    /* Check the return code */
    if (ret != 0)
    {
        goto InitFail;
    }

    /* Create thread number 4. */
    ptr = CyU3PMemAlloc (THREAD_STACK);
    if (ptr != NULL)
    {
        ret = CyU3PThreadCreate (
                &Thread4,                              /* Thread structure. */
                "24:App Thread 4",                     /* Thread ID and name. */        
                Thread4_Entry,                         /* Thread entry function. */
                0,                                     /* Thread input parameter. */
                ptr,                                   /* Pointer to the allocated thread stack. */
                THREAD_STACK,                          /* Allocated thread stack size. */
                THREAD_PRIORITY,                       /* Thread priority. */
                THREAD_PRIORITY,                       /* Thread pre-emption threshold: No preemption. */
                CYU3P_NO_TIME_SLICE,                   /* No time slice. */
                CYU3P_AUTO_START                       /* Start the thread immediately. */
                );
    }
    else
    {
        ret = CY_U3P_ERROR_MEMORY_ERROR;
    }

    /* Check the return code */
    if (ret != 0)
    {
        goto InitFail;
    }

    return;

InitFail:
    /* As the initialization failed, there is nothing much we can do. Just reset the device
     * so that we go back to the boot-loader. */
    CyU3PDeviceReset (CyFalse);
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

