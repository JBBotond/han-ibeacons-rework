# IAR Template Project for FRDM-MCXA153

## Project Structure

```
Template/
├── src/                    # Application source files
│   ├── main.c
│   ├── serial_communication.c
│   ├── serial.c
│   ├── lpuart2.c
│   └── fifo.c
│
├── inc/                    # Application headers
│   ├── serial_communication.h
│   ├── serial.h
│   ├── lpuart2.h
│   └── fifo.h
│
├── device/                 # Device-specific files
│   ├── MCXA153.h
│   ├── system_MCXA153.h
│   ├── system_MCXA153.c
│   ├── startup_MCXA153.s
│   └── periph/            # Peripheral headers (38 files)
│
├── drivers/                # SDK drivers (minimal set)
│   ├── fsl_common.h/c
│   ├── fsl_common_arm.h/c
│   ├── fsl_clock.h/c
│   ├── fsl_reset.h/c
│   ├── fsl_gpio.h/c
│   └── fsl_spc.h/c
│
├── CMSIS/                  # ARM CMSIS core
│   └── Include/           # Core headers (9 files)
│
└── iar/                    # IAR project files
    ├── IARTemplate.ewp    # Project file
    ├── IARTemplate.eww    # Workspace file
    └── MCXA153_flash.icf  # Linker script
```

## How to Use

1. Open IAR Embedded Workbench
2. Open workspace: `Template/iar/IARTemplate.eww`
3. Build project (F7)
4. Debug (Ctrl+D)

## Include Paths (Already configured)

- ../inc
- ../device
- ../device/periph
- ../drivers
- ../CMSIS/Include

## Preprocessor Defines (Already configured)

- CPU_MCXA153VLH
- __STARTUP_CLEAR_BSS

## Memory Configuration

- Flash: 0x00000000 - 0x0001FFFF (128 KB)
- RAM:   0x20000000 - 0x20005FFF (24 KB)
- SRAMX: 0x04000000 - 0x04001FFF (8 KB)
- Stack: 1 KB
- Heap:  1 KB

## Files Summary

Total files: ~65
- Application: 9 files (.c/.h)
- Device: 41 files (headers + startup + system)
- Drivers: 12 files (6 drivers)
- CMSIS: 9 files
- IAR: 3 files

## Notes

- All peripheral registers accessible via MCXA153.h
- UART0 (serial) and UART2 (lpuart2) configured
- FIFO buffers for interrupt-driven communication
- Clock configured for 96 MHz operation
- Minimal footprint - only essential SDK components
