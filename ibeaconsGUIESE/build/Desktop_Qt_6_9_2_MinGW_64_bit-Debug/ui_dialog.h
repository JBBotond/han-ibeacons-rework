/********************************************************************************
** Form generated from reading UI file 'dialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_H
#define UI_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_Dialog
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *layoutPortSelector;
    QLabel *lblPort;
    QComboBox *cboPorts;
    QPushButton *btnRefreshPorts;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *verticalSpacer;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName("Dialog");
        Dialog->resize(800, 600);
        verticalLayout = new QVBoxLayout(Dialog);
        verticalLayout->setObjectName("verticalLayout");
        layoutPortSelector = new QHBoxLayout();
        layoutPortSelector->setObjectName("layoutPortSelector");
        lblPort = new QLabel(Dialog);
        lblPort->setObjectName("lblPort");

        layoutPortSelector->addWidget(lblPort);

        cboPorts = new QComboBox(Dialog);
        cboPorts->setObjectName("cboPorts");
        cboPorts->setMinimumWidth(300);

        layoutPortSelector->addWidget(cboPorts);

        btnRefreshPorts = new QPushButton(Dialog);
        btnRefreshPorts->setObjectName("btnRefreshPorts");

        layoutPortSelector->addWidget(btnRefreshPorts);

        horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        layoutPortSelector->addItem(horizontalSpacer);


        verticalLayout->addLayout(layoutPortSelector);

        verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QCoreApplication::translate("Dialog", "iBeacon GUI", nullptr));
        lblPort->setText(QCoreApplication::translate("Dialog", "COM Port:", nullptr));
        btnRefreshPorts->setText(QCoreApplication::translate("Dialog", "Refresh", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_H
