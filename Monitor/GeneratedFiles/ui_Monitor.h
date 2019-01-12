/********************************************************************************
** Form generated from reading UI file 'Monitor.ui'
**
** Created by: Qt User Interface Compiler version 5.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MONITOR_H
#define UI_MONITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MonitorClass
{
public:

    void setupUi(QWidget *MonitorClass)
    {
        if (MonitorClass->objectName().isEmpty())
            MonitorClass->setObjectName(QString::fromUtf8("MonitorClass"));
        MonitorClass->resize(600, 400);

        retranslateUi(MonitorClass);

        QMetaObject::connectSlotsByName(MonitorClass);
    } // setupUi

    void retranslateUi(QWidget *MonitorClass)
    {
        MonitorClass->setWindowTitle(QApplication::translate("MonitorClass", "Monitor", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MonitorClass: public Ui_MonitorClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MONITOR_H
