#pragma once

#include <QDialog>
#include "ui_connectDialog.h"
#include <mavsdk/mavsdk.h>

class connectDialog : public QDialog
{
    Q_OBJECT

public:
    connectDialog(QWidget* parent = Q_NULLPTR);
    ~connectDialog();

    QString getConnectionURL();

private:
    Ui::connectDialog ui;

private slots:
    void on_comboBox_currentTextChanged(QString text);
};