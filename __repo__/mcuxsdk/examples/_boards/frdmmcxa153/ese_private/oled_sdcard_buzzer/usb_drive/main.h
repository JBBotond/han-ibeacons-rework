/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.h
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

#ifndef __USB_DRIVE_H__
#define __USB_DRIVE_H__

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif
#endif

#define USB_DEVICE_INTERRUPT_PRIORITY (1U)

/* Length of Each Logical Address Block */
#define LENGTH_OF_EACH_LBA (512U)

/* Total number of logical blocks present
 * For FatFS on SD card, we report actual SD card capacity.
 * Reserve some blocks for file system overhead.
 */
#define TOTAL_LOGICAL_ADDRESS_BLOCKS_NORMAL (1024U)  /* 512KB virtual size */

/* Net Disk Size */
#define DISK_SIZE_NORMAL (TOTAL_LOGICAL_ADDRESS_BLOCKS_NORMAL * LENGTH_OF_EACH_LBA)

/* Application define logical unit number, if LOGICAL_UNIT_SUPPORTED > USB_DEVICE_MSC_MAX_LUN, update
 * USB_DEVICE_MSC_MAX_LUN in class driver usb_device_msc.h */
#define LOGICAL_UNIT_SUPPORTED (1U)

/* USB transfer buffer size - single 512B sector */
#define USB_MSC_BUFFER_SIZE (512U)

typedef struct _usb_msc_struct
{
    usb_device_handle deviceHandle;
    class_handle_t mscHandle;

    /* Single transfer buffer for USB operations */
    uint8_t transferBuffer[USB_MSC_BUFFER_SIZE];

    /* Disk status flags */
    uint8_t diskLock;
    uint8_t read_write_error;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_MSC_INTERFACE_COUNT];
    uint8_t speed;
    uint8_t attach;
    uint8_t stop; /* indicates this media keeps stop or not, 1: stop, 0: start */
} usb_msc_struct_t;

#endif /* __USB_DRIVE_H__ */
