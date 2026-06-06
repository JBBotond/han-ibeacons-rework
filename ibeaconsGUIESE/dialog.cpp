#include "dialog.h"
#include "ui_dialog.h"
#include <QSerialPortInfo>
#include <QVBoxLayout>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , serial(new QSerialPort(this))
{
    ui->setupUi(this);
    populatePorts();
    connect(ui->btnRefreshPorts, &QPushButton::clicked, this, &Dialog::populatePorts);
    connect(ui->btnConnect,      &QPushButton::clicked, this, &Dialog::connectSerial);
    connect(ui->btnScan,          &QPushButton::clicked, this, &Dialog::startStopScan);
    connect(serial, &QSerialPort::readyRead, this, &Dialog::onSerialData);
    ui->btnScan->setEnabled(false);

    // --- Chart setup ---
    m_chart = new QChart();
    m_chart->setTitle("RSSI per scan");
    m_chart->legend()->setVisible(true);

    m_axisX = new QValueAxis();
    m_axisX->setTitleText("Scan #");
    m_axisX->setLabelFormat("%d");
    m_axisX->setRange(0, MAX_POINTS);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    m_axisY = new QValueAxis();
    m_axisY->setTitleText("RSSI (dBm)");
    m_axisY->setRange(-100, -30);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(250);

    // Insert chart above the log
    qobject_cast<QVBoxLayout*>(layout())->insertWidget(1, m_chartView);
}


Dialog::~Dialog()
{
    if(serial->isOpen())
        serial->close();
    delete ui;
}

void Dialog::connectSerial()
{
    if(!m_connected)
    {
        QString port = ui->cboPorts->currentData().toString();
        if(port.isEmpty()) return;

        serial->setPortName(port);
        serial->setBaudRate(QSerialPort::Baud115200);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);

        if(serial->open(QIODevice::ReadWrite))
        {
            m_connected = true;
            ui->btnConnect->setText("Disconnect");
            ui->cboPorts->setEnabled(false);
            ui->btnRefreshPorts->setEnabled(false);
            ui->btnScan->setEnabled(true);
        }
    }
    else
    {
        serial->close();
        m_connected = false;
        m_scanning  = false;
        ui->btnConnect->setText("Connect");
        ui->btnScan->setText("Start Scan");
        ui->btnScan->setEnabled(false);
        ui->cboPorts->setEnabled(true);
        ui->btnRefreshPorts->setEnabled(true);
    }
}

void Dialog::startStopScan()
{
    if(!m_scanning)
    {
        serial->write("AT+DISI?");
        m_scanning = true;
        ui->btnScan->setText("Stop Scan");
        m_rxBuf.clear();
    }
    else
    {
        m_scanning = false;
        ui->btnScan->setText("Start Scan");
    }
}

void Dialog::onSerialData()
{
    m_rxBuf += QString::fromLatin1(serial->readAll());

    const QString DISC    = "OK+DISC:";
    const QString DISIS   = "OK+DISIS";
    const QString DISCE   = "OK+DISCE";
    const QString UUID    = "74278BDAB64445208F0C720EAF059935";
    const QString FACTORY = "4C000215";
    const int     DISC_LEN = 78;

    while(m_rxBuf.length() >= 8)
    {
        if(m_rxBuf.startsWith(DISIS))
        {
            ui->txtLog->appendPlainText(">> Scan started");
            m_rxBuf.remove(0, 8);
        }
        else if(m_rxBuf.startsWith(DISCE))
        {
            ui->txtLog->appendPlainText(">> Scan ended");
            m_scanning = false;
            ui->btnScan->setText("Start Scan");
            m_rxBuf.remove(0, 8);
        }
        else if(m_rxBuf.startsWith(DISC))
        {
            if(m_rxBuf.length() < DISC_LEN) break;
            QString record = m_rxBuf.left(DISC_LEN);
            m_rxBuf.remove(0, DISC_LEN);

            if(record.mid(8, 8) == FACTORY && record.mid(17, 32) == UUID)
            {
                uint16_t major   = record.mid(50, 4).toUShort(nullptr, 16);
                uint16_t minor   = record.mid(54, 4).toUShort(nullptr, 16);
                int8_t   txpower = (int8_t)record.mid(58, 2).toInt(nullptr, 16);
                int32_t  rssi    = record.mid(74).toInt();

                ui->txtLog->appendPlainText(
                    QString("iBeacon  Major:0x%1  Minor:0x%2  TxPow:%3dBm  RSSI:%4dBm")
                    .arg(major, 4, 16, QChar('0')).toUpper()
                    .arg(minor, 4, 16, QChar('0')).toUpper()
                    .arg(txpower)
                    .arg(rssi));

                QString label = QString("0x%1/0x%2")
                    .arg(major, 4, 16, QChar('0')).toUpper()
                    .arg(minor, 4, 16, QChar('0')).toUpper();
                addRssiPoint(label, rssi);
            }
        }
        else
        {
            m_rxBuf.remove(0, 1);
        }
    }
}

void Dialog::addRssiPoint(const QString &label, qreal rssi)
{
    if(!m_series.contains(label))
    {
        QLineSeries *s = new QLineSeries();
        s->setName(label);
        s->setPointsVisible(true);
        s->setPointLabelsVisible(false);
        QPen pen = s->pen();
        pen.setWidth(2);
        s->setPen(pen);
        m_chart->addSeries(s);
        s->attachAxis(m_axisX);
        s->attachAxis(m_axisY);
        m_series[label] = s;
    }

    QLineSeries *s = m_series[label];
    s->append(m_scanIndex, rssi);

    // Scroll X window
    if(m_scanIndex >= MAX_POINTS)
        m_axisX->setRange(m_scanIndex - MAX_POINTS, m_scanIndex);

    // Auto-fit Y
    qreal minY = 0, maxY = -150;
    for(auto *series : m_series)
        for(const QPointF &p : series->points())
        {
            if(p.y() < minY) minY = p.y();
            if(p.y() > maxY) maxY = p.y();
        }
    m_axisY->setRange(minY - 5, maxY + 5);

    m_scanIndex++;
}

void Dialog::populatePorts()
{
    ui->cboPorts->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports)
        ui->cboPorts->addItem(info.portName() + " - " + info.description(), info.portName());
}
