#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSerialPort>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QMap>

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
    void addRssiPoint(const QString &label, qreal rssi);

    Ui::Dialog    *ui;
    QSerialPort   *serial;
    bool           m_connected = false;
    bool           m_scanning  = false;
    QString        m_rxBuf;

    QChart                    *m_chart;
    QChartView                *m_chartView;
    QValueAxis                *m_axisX;
    QValueAxis                *m_axisY;
    QMap<QString, QLineSeries*> m_series;
    int                        m_scanIndex = 0;
    static const int           MAX_POINTS  = 30;
};
#endif // DIALOG_H
