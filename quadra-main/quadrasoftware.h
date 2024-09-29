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

private slots:
    void on_tabButton1_clicked();
    void on_tabButton2_clicked();
};
