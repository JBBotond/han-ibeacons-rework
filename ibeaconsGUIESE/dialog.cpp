#include "dialog.h"
#include "ui_dialog.h"
#include <QSerialPortInfo>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , serial(new QSerialPort(this))
{
    ui->setupUi(this);
    populatePorts();
    connect(ui->btnRefreshPorts, &QPushButton::clicked, this, &Dialog::populatePorts);
    connect(ui->btnConnect,      &QPushButton::clicked, this, &Dialog::connectSerial);
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
        }
    }
    else
    {
        serial->close();
        m_connected = false;
        ui->btnConnect->setText("Connect");
        ui->cboPorts->setEnabled(true);
        ui->btnRefreshPorts->setEnabled(true);
    }
}

void Dialog::populatePorts(){
    ui -> cboPorts-> clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports)
        ui -> cboPorts ->addItem(info.portName() + " - " + info.description(), info.portName());
}
