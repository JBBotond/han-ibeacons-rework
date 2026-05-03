
## Step 1 — Port selector dropdown

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

---

## Step 2 — Connect/Disconnect button

### `dialog.ui`
- Added `QPushButton` → `btnConnect` ("Connect") to the port selector row, after `btnRefreshPorts`
- Row now: `[ COM Port: ] [ cboPorts ▼ ] [ Refresh ] [ Connect ]`

### `dialog.h`
- Added `void connectSerial()` private slot
- Added `bool m_connected = false` member

### `dialog.cpp`
- `btnConnect` connected to `connectSerial()` in constructor
- `connectSerial()` on Connect: opens port at 115200/8N1, sets button to "Disconnect", disables `cboPorts` + `btnRefreshPorts`
- `connectSerial()` on Disconnect: closes port, resets button to "Connect", re-enables `cboPorts` + `btnRefreshPorts`
- Destructor safely closes port if still open