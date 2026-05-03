
## Changes summary

### `dialog.ui`
- Added a `QHBoxLayout` at the top with:
  - `QLabel` → "COM Port:"
  - `QComboBox` → `cboPorts` (300px min width)
  - `QPushButton` → `btnRefreshPorts` ("Refresh")
  - `QSpacerItem` → pushes widgets to the left

### `dialog.h`
- Added `#include <QSerialPort>`
- Added `QSerialPort *serial` member
- Added `void populatePorts()` private slot

### `dialog.cpp`
- Added `#include <QSerialPortInfo>`
- `serial` initialized in constructor
- `populatePorts()` called on startup — fills `cboPorts` with `portName + description`
- "Refresh" button connected to `populatePorts()`