#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSerialPort>


QT_BEGIN_NAMESPACE
namespace Ui {
class Dialog;
}
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void populatePorts();
    void connectSerial();
    void startStopScan();
    void onSerialData();

private:
    Ui::Dialog *ui;
    QSerialPort *serial;
    bool m_connected = false;
    bool m_scanning  = false;
    QString m_rxBuf;
};
#endif // DIALOG_H
