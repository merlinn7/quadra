#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_quadrasoftware.h"

class quadrasoftware : public QMainWindow
{
    Q_OBJECT

public:
    quadrasoftware(QWidget *parent = nullptr);
    ~quadrasoftware();

private:
    Ui::quadrasoftwareClass ui;
};
