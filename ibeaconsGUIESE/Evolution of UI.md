
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

---

## Step 3 — Start/Stop Scan button

### `dialog.ui`
- Added `QPushButton` → `btnScan` ("Start Scan") to the port selector row, after `btnConnect`
- Row now: `[ COM Port: ] [ cboPorts ▼ ] [ Refresh ] [ Connect ] [ Start Scan ]`

### `dialog.h`
- Added `void startStopScan()` private slot
- Added `bool m_scanning = false` member

### `dialog.cpp`
- `btnScan` starts disabled — enabled only after successful Connect
- `btnScan` connected to `startStopScan()` in constructor
- `startStopScan()` on Start: sends `"AT+DISI?"` directly to HM-10 via serial2lpuart, sets button to "Stop Scan", clears `m_rxBuf`
- `startStopScan()` on Stop: resets flag and label only (scan finishes naturally on `OK+DISCE`)
- On Disconnect: `btnScan` disabled, text reset to "Start Scan", `m_scanning` reset

---

## Step 4 — Serial log + iBeacon parser

### `dialog.ui`
- Replaced vertical spacer with `QPlainTextEdit` → `txtLog` (read-only, fills remaining window space)

### `dialog.h`
- Added `void onSerialData()` private slot
- Added `QString m_rxBuf` member — accumulates raw bytes across multiple `readyRead` calls

### `dialog.cpp`
- Added `#include <QVBoxLayout>`
- `readyRead` signal connected to `onSerialData()` in constructor
- `onSerialData()` — token-based parser (no CR/LF dependency):
  - Appends incoming bytes to `m_rxBuf`
  - `OK+DISIS` (8 bytes) → logs ">> Scan started"
  - `OK+DISCE` (8 bytes) → logs ">> Scan ended", resets button to "Start Scan"
  - `OK+DISC:` (78 bytes) → checks Factory ID `4C000215` + UUID `74278BDAB64445208F0C720EAF059935`
    - If match: extracts Major `[50:54]`, Minor `[54:58]`, TxPower `[58:60]`, RSSI `[74:]`
    - Logs: `iBeacon  Major:0xXXXX  Minor:0xXXXX  TxPow:XXdBm  RSSI:XXdBm`
  - Unknown byte at head → drops 1 byte and resyncs

---

## Step 5 — Live RSSI chart

### `ibeaconsGUIESE.pro`
- Added `charts` to `QT` modules

### `dialog.h`
- Added includes: `QChart`, `QChartView`, `QLineSeries`, `QValueAxis`, `QMap`
- Added members:
  - `QChart *m_chart`
  - `QChartView *m_chartView`
  - `QValueAxis *m_axisX` — "Scan #", scrolling window of 30 points
  - `QValueAxis *m_axisY` — "RSSI (dBm)", auto-fitted to actual data range
  - `QMap<QString, QLineSeries*> m_series` — one line per beacon label (`Major/Minor`)
  - `int m_scanIndex` — X counter, increments per RSSI point added
  - `static const int MAX_POINTS = 30`
- Added `void addRssiPoint(const QString &label, qreal rssi)` private helper

### `dialog.cpp`
- Constructor builds chart and inserts `m_chartView` above `txtLog` via `QVBoxLayout::insertWidget(1, ...)`
- X axis range: 0–30, scrolls after 30 points
- Y axis: auto-fits ±5 dBm around actual min/max across all series
- `onSerialData()` calls `addRssiPoint(label, rssi)` after each parsed iBeacon
- `addRssiPoint()`:
  - Creates new `QLineSeries` on first sight of a label
  - Series configured with `setPointsVisible(true)` and pen width 2 (single points visible)
  - Appends `(m_scanIndex, rssi)` to the series
  - Scrolls X axis and auto-fits Y axis
  - Increments `m_scanIndex`
