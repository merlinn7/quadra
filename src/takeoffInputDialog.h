#pragma once

#include <QDialog>
#include "ui_takeoffInputDialog.h"
#include <mavsdk/mavsdk.h>

class takeoffInputDialog : public QDialog
{
    Q_OBJECT

public:
    takeoffInputDialog(QWidget* parent = Q_NULLPTR);
    ~takeoffInputDialog();

    int GetAltitude();

private:
    Ui::takeoffInputDialog ui;
};