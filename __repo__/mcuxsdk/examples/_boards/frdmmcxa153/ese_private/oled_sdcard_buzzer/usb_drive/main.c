/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      February 2026
 *
 * \copyright 2026 HAN University of Applied Sciences. All Rights Reserved.
 *            \n\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \n\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \n\n
 *            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *            OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *            HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *            WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *            FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *            OTHER DEALINGS IN THE SOFTWARE.
 *
 * \copyright Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 *            Copyright 2016 - 2017, 2019 NXP
 *            All rights reserved.
 *
 *            SPDX-License-Identifier: BSD-3-Clause
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_msc.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "main.h"

#include "board.h"
#include "buzzer.h"
#include "leds.h"
#include "lpspi_master.h"
#include "oled.h"
#include "serial.h"
#include "diskio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef DEBUG
#define TARGETSTR "Debug"
#else
#define TARGETSTR "Release"
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Inquiry data for MSC device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
usb_device_inquiry_data_fromat_struct_t g_InquiryInfo = {
    (USB_DEVICE_MSC_UFI_PERIPHERAL_QUALIFIER << USB_DEVICE_MSC_UFI_PERIPHERAL_QUALIFIER_SHIFT) |
        USB_DEVICE_MSC_UFI_PERIPHERAL_DEVICE_TYPE,
    (uint8_t)(USB_DEVICE_MSC_UFI_REMOVABLE_MEDIUM_BIT << USB_DEVICE_MSC_UFI_REMOVABLE_MEDIUM_BIT_SHIFT),
    USB_DEVICE_MSC_UFI_VERSIONS,
    0x02,
    USB_DEVICE_MSC_UFI_ADDITIONAL_LENGTH,
    {0x00, 0x00, 0x00},
    {'H', 'A', 'N', ' ', 'E', 'S', 'E', ' '},
    {'o', 'l', 'e', 'd', '_', 's', 'd', 'c', 'a', 'r', 'd', '_', 'b', 'u', 'z'},
    {'0', '0', '0', '1'}
};

/* Mode parameters header */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
usb_device_mode_parameters_header_struct_t g_ModeParametersHeader = {
    /*refer to ufi spec mode parameter header*/
    0x0000, /*!< Mode Data Length*/
    0x00,   /*!<Default medium type (current mounted medium type)*/
    0x00,   /*!MODE SENSE command, a Write Protected bit of zero
              indicates the medium is write enabled*/
    {0x00, 0x00, 0x00, 0x00} /*!<This bit should be set to zero*/
};

/* MSC device structure */
usb_msc_struct_t g_msc;

static uint8_t g_diskInitialized = 0;
static uint32_t g_sectorCount = 0U;
static uint16_t g_sectorSize = LENGTH_OF_EACH_LBA;

volatile uint32_t ms = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Initialize raw SDCARD access.
 *
 * This function initializes the SDCARD and queries capacity.
 *
 * @return kStatus_USB_Success or error.
 */
static usb_status_t USB_DeviceMscInitDisk(void)
{
    DSTATUS diskStatus = disk_initialize(0);

    if (diskStatus != 0)
    {
        printf("SDCARD initialization failed: %d\r\n", diskStatus);
        return kStatus_USB_Error;
    }

    if (disk_ioctl(0, GET_SECTOR_COUNT, &g_sectorCount) != RES_OK)
    {
        printf("Get sector count failed\r\n");
        return kStatus_USB_Error;
    }

    if (disk_ioctl(0, GET_SECTOR_SIZE, &g_sectorSize) != RES_OK)
    {
        printf("Get sector size failed\r\n");
        g_sectorSize = LENGTH_OF_EACH_LBA;
    }

    g_diskInitialized = 1U;
    printf("FatFS ready\r\n");
    printf("SDCARD ready: sectors=%lu size=%u\r\n", g_sectorCount, g_sectorSize);

    return kStatus_USB_Success;
}

/*!
 * @brief Read raw blocks from SDCARD into buffer.
 *
 * @param lbaOffset   LBA offset (sector number) to read from.
 * @param buffer      Destination buffer for read data.
 * @param bufferSize  Size of data to read in bytes.
 *
 * @return kStatus_USB_Success or error.
 */
static usb_status_t USB_DeviceMscReadBlocks(uint32_t lbaOffset, uint8_t *buffer,
    uint32_t bufferSize)
{
    uint32_t sectorSize = g_sectorSize;
    uint32_t sectorCount;

    if (!g_diskInitialized)
    {
        printf("Disk not initialized\r\n");
        memset(buffer, 0, bufferSize);
        return kStatus_USB_Error;
    }

    if ((bufferSize == 0U) || ((bufferSize % sectorSize) != 0U))
    {
        printf("Read size not aligned: %lu\r\n", bufferSize);
        memset(buffer, 0, bufferSize);
        return kStatus_USB_Error;
    }

    sectorCount = bufferSize / sectorSize;
    if (disk_read(0, buffer, (LBA_t)lbaOffset, sectorCount) != RES_OK)
    {
        memset(buffer, 0, bufferSize);
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

/*!
 * @brief Write raw blocks from buffer to SDCARD.
 *
 * @param lbaOffset   LBA offset (sector number) to write to.
 * @param buffer      Source buffer with data to write.
 * @param bufferSize  Size of data to write in bytes.
 *
 * @return kStatus_USB_Success or error.
 */
static usb_status_t USB_DeviceMscWriteBlocks(uint32_t lbaOffset, const uint8_t *buffer,
    uint32_t bufferSize)
{
    uint32_t sectorSize = g_sectorSize;
    uint32_t sectorCount;

    if (!g_diskInitialized)
    {
        printf("Disk not initialized\r\n");
        return kStatus_USB_Error;
    }

    if ((bufferSize == 0U) || ((bufferSize % sectorSize) != 0U))
    {
        printf("Write size not aligned: %lu\r\n", bufferSize);
        return kStatus_USB_Error;
    }

    sectorCount = bufferSize / sectorSize;
    if (disk_write(0, buffer, (LBA_t)lbaOffset, sectorCount) != RES_OK)
    {
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

/*!
 * @brief MSC device callback function.
 *
 * This function handles the MSC class specific events.
 *
 * @param handle  The USB class handle.
 * @param event   The USB device event type.
 * @param param   The parameter of the class specific event.
 *
 * @return kStatus_USB_Success or error.
 */
static usb_status_t USB_DeviceMscCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Success;
    usb_device_lba_information_struct_t *lbaInformationStructure;
    usb_device_lba_app_struct_t *lbaData;
    usb_device_ufi_app_struct_t *ufi;
    usb_device_capacity_information_struct_t *capacityInformation;

    switch (event)
    {
        case kUSB_DeviceMscEventReadResponse:
            // Read operation completed
            lbaData = (usb_device_lba_app_struct_t *)param;
            break;

        case kUSB_DeviceMscEventWriteResponse:
            // Write completed - now save the data
            lbaData = (usb_device_lba_app_struct_t *)param;
            USB_DeviceMscWriteBlocks(lbaData->offset, g_msc.transferBuffer, lbaData->size);
            break;

        case kUSB_DeviceMscEventWriteRequest:
            // Host wants to write - provide empty buffer
            lbaData = (usb_device_lba_app_struct_t *)param;
            lbaData->buffer = g_msc.transferBuffer;
            break;

        case kUSB_DeviceMscEventReadRequest:
            // Host wants to read - load data first
            lbaData = (usb_device_lba_app_struct_t *)param;
            USB_DeviceMscReadBlocks(lbaData->offset, g_msc.transferBuffer, lbaData->size);
            lbaData->buffer = g_msc.transferBuffer;
            break;

        case kUSB_DeviceMscEventGetLbaInformation:
            lbaInformationStructure = (usb_device_lba_information_struct_t *)param;
            lbaInformationStructure->logicalUnitNumberSupported = LOGICAL_UNIT_SUPPORTED;
            lbaInformationStructure->logicalUnitInformations[0].lengthOfEachLba = g_sectorSize;
            lbaInformationStructure->logicalUnitInformations[0].totalLbaNumberSupports =
                (g_sectorCount != 0U) ? g_sectorCount : TOTAL_LOGICAL_ADDRESS_BLOCKS_NORMAL;
            lbaInformationStructure->logicalUnitInformations[0].bulkInBufferSize  =
                USB_MSC_BUFFER_SIZE;
            lbaInformationStructure->logicalUnitInformations[0].bulkOutBufferSize =
                USB_MSC_BUFFER_SIZE;
            break;

        case kUSB_DeviceMscEventTestUnitReady:
            // Change the test unit ready command's sense data if needed
            if (1U == g_msc.stop)
            {
                ufi = (usb_device_ufi_app_struct_t *)param;
                ufi->requestSense->senseKey            = USB_DEVICE_MSC_UFI_NOT_READY;
                ufi->requestSense->additionalSenseCode = USB_DEVICE_MSC_UFI_ASC_MEDIUM_NOT_PRESENT;
            }
            break;

        case kUSB_DeviceMscEventInquiry:
            ufi         = (usb_device_ufi_app_struct_t *)param;
            ufi->size   = sizeof(usb_device_inquiry_data_fromat_struct_t);
            ufi->buffer = (uint8_t *)&g_InquiryInfo;
            break;

        case kUSB_DeviceMscEventModeSense:
            ufi         = (usb_device_ufi_app_struct_t *)param;
            ufi->size   = sizeof(usb_device_mode_parameters_header_struct_t);
            ufi->buffer = (uint8_t *)&g_ModeParametersHeader;
            break;

        case kUSB_DeviceMscEventModeSelectResponse:
            ufi = (usb_device_ufi_app_struct_t *)param;
            break;

        case kUSB_DeviceMscEventModeSelect:
        case kUSB_DeviceMscEventFormatComplete:
        case kUSB_DeviceMscEventRemovalRequest:
            error = kStatus_USB_InvalidRequest;
            break;

        case kUSB_DeviceMscEventRequestSense:
            break;

        case kUSB_DeviceMscEventReadCapacity:
            capacityInformation = (usb_device_capacity_information_struct_t *)param;
            capacityInformation->lengthOfEachLba        = g_sectorSize;
            capacityInformation->totalLbaNumberSupports =
                (g_sectorCount != 0U) ? g_sectorCount : TOTAL_LOGICAL_ADDRESS_BLOCKS_NORMAL;
            break;

        case kUSB_DeviceMscEventReadFormatCapacity:
            capacityInformation = (usb_device_capacity_information_struct_t *)param;
            capacityInformation->lengthOfEachLba        = g_sectorSize;
            capacityInformation->totalLbaNumberSupports =
                (g_sectorCount != 0U) ? g_sectorCount : TOTAL_LOGICAL_ADDRESS_BLOCKS_NORMAL;
            break;

        case kUSB_DeviceMscEventStopEjectMedia:
            ufi = (usb_device_ufi_app_struct_t *)param;

            // Check start bit
            if (0x00U == (ufi->cbwcb[4] & 0x01U))
            {
                // Stop command
                g_msc.stop = 1U;
            }
            break;

        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }

    return error;
}

/*!
 * @brief USB device callback function.
 *
 * This function handles the USB standard events.
 *
 * @param handle  The USB device handle.
 * @param event   The USB device event type.
 * @param param   The parameter of the device specific request.
 *
 * @return kStatus_USB_Success or error.
 */
static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint16_t *temp16   = (uint16_t *)param;
    uint8_t *temp8     = (uint8_t *)param;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            g_msc.attach               = 0U;
            g_msc.currentConfiguration = 0U;
            g_msc.stop                 = 0U;
            error                      = kStatus_USB_Success;

#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
            // Get USB speed to configure the device, including max packet size and
            // interval of the endpoints.
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &g_msc.speed))
            {
                USB_DeviceSetSpeed(handle, g_msc.speed);
            }
#endif
        }
        break;

        // Does not seem to work on MCXA devices:
        // https://community.nxp.com/t5/MCX-Microcontrollers/Full-Speed-USB-FRDM-MCXA153-usb-device-not-detecting-detach/m-p/1923985
        case kUSB_DeviceEventDetach:
        {
            g_msc.attach               = 0U;
            g_msc.currentConfiguration = 0U;
            g_msc.stop                 = 0U;
            error                      = kStatus_USB_Success;

            printf("USB device disconnected\r\n");
        }
        break;

        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                g_msc.attach               = 0U;
                g_msc.currentConfiguration = 0U;
                error                      = kStatus_USB_Success;

                printf("USB device disconnected\r\n");
            }
            else if (USB_MSC_CONFIGURE_INDEX == (*temp8))
            {
                g_msc.attach               = 1U;
                g_msc.currentConfiguration = *temp8;
                error                      = kStatus_USB_Success;

                printf("USB device connected\r\n");
            }
            else
            {
                // no action, return kStatus_USB_InvalidRequest
            }
            break;

        case kUSB_DeviceEventSetInterface:
            if (0U != g_msc.attach)
            {
                uint8_t interface        = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);
                if (interface < USB_MSC_INTERFACE_COUNT)
                {
                    if (alternateSetting < USB_MSC_INTERFACE_ALTERNATE_COUNT)
                    {
                        g_msc.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        error                                             = kStatus_USB_Success;
                    }
                }
            }
            break;

        case kUSB_DeviceEventGetConfiguration:
            if (NULL != param)
            {
                *temp8 = g_msc.currentConfiguration;
                error  = kStatus_USB_Success;
            }
            break;

        case kUSB_DeviceEventGetInterface:
            if (NULL != param)
            {
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) |
                        g_msc.currentInterfaceAlternateSetting[interface];
                    error   = kStatus_USB_Success;
                }
            }
            break;

        case kUSB_DeviceEventGetDeviceDescriptor:
            if (NULL != param)
            {
                error = USB_DeviceGetDeviceDescriptor(handle,
                    (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;

        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (NULL != param)
            {
                error = USB_DeviceGetConfigurationDescriptor(handle,
                    (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;

#if (defined(USB_DEVICE_CONFIG_CV_TEST) && (USB_DEVICE_CONFIG_CV_TEST > 0U))
        case kUSB_DeviceEventGetDeviceQualifierDescriptor:
            if (param)
            {
                /* Get Qualifier descriptor request */
                error = USB_DeviceGetDeviceQualifierDescriptor(
                    handle, (usb_device_get_device_qualifier_descriptor_struct_t *)param);
            }
            break;
#endif

        case kUSB_DeviceEventGetStringDescriptor:
            if (NULL != param)
            {
                error = USB_DeviceGetStringDescriptor(handle,
                    (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;

        default:
            break;
    }

    return error;
}

// USB device class information
usb_device_class_config_struct_t msc_config[1] =
{
    {
        USB_DeviceMscCallback,
        0U,
        &g_UsbDeviceMscConfig,
    }
};

// USB device class configuration information
usb_device_class_config_list_struct_t msc_config_list =
{
    msc_config,
    USB_DeviceCallback,
    1U,
};

/*!
 * @brief Device application init function.
 *
 * This function initializes the USB stack and FatFS.
 *
 * @return None.
 */
static void USB_DeviceApplicationInit(void)
{
    USB_DeviceClockInit();

    // Initialize MSC structure
    memset(&g_msc, 0, sizeof(g_msc));
    g_msc.speed        = USB_SPEED_FULL;
    g_msc.attach       = 0U;
    g_msc.mscHandle    = (class_handle_t)NULL;
    g_msc.deviceHandle = NULL;

    // Initialize raw disk access
    if (kStatus_USB_Success != USB_DeviceMscInitDisk())
    {
        printf("Disk initialization failed\r\n");
    }

    // Initialize USB device
    if (kStatus_USB_Success != USB_DeviceClassInit(CONTROLLER_ID,
        &msc_config_list, &g_msc.deviceHandle))
    {
        printf("USB device init failed\r\n");
    }
    else
    {
        printf("USB device ready\r\n");
        g_msc.mscHandle = msc_config_list.config->classHandle;
    }

    USB_DeviceIsrEnable();

    // Add one delay here to make the DP pull down long enough to allow host to
    // detect the previous disconnection

    uint32_t current_ms = ms;
    while ((ms - current_ms) < 10)
    {}

    USB_DeviceRun(g_msc.deviceHandle);
}

#if (defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U))
void USB0_IRQHandler(void)
{
    USB_DeviceKhciIsrFunction(g_msc.deviceHandle);
}
#endif

void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    // Select FIRC clock as USB clock source (48 MHz)
    SCG0->FIRCCSR |= SCG_FIRCCSR_FIRC_SCLK_PERIPH_EN(1);
    MRCC0->MRCC_USB0_CLKSEL = MRCC_MRCC_USB0_CLKSEL_MUX(0x01U);

    // Reset USB0
    MRCC0->MRCC_GLB_RST0_CLR = MRCC_MRCC_GLB_RST0_USB0(1U);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_USB0(1U);

    // Enable USB0 clock
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_USB0(1U);
#endif
}

void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;

#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    uint8_t usbDeviceKhciIrq[] = USBFS_IRQS;
    irqNumber                  = usbDeviceKhciIrq[CONTROLLER_ID - kUSB_ControllerKhci0];
#endif

    // Install ISR, set priority, and enable IRQ
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}

int main(void)
{
    // 96 MHz FIRC clock selected
    SCG0->FIRCCFG = SCG_FIRCCFG_FREQ_SEL(0b101);

    // Systick interrupt every 1 ms
    SysTick_Config(96000);

    serial_init(115200);
    leds_init();
    lpspi_master_init();
    oled_init();
    buzzer_init();
    buzzer_set(0);

    printf("USB Mass Storage Device (MSC)\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);

    oled_setorientation(1);
    oled_drawbitmap(usb_drive);
    oled_update();

    // Give OLED some time to finish updating before starting USB stack
    while(ms < 200)
    {}

    USB_DeviceApplicationInit();

    printf("Connect USB cable to MCU-USB port (J8)\r\n");

    while (1)
    {
        // Do nothing here, make sure all processing is handled
        // by USB stack
    }
}

void SysTick_Handler(void)
{
    ms++;
}
