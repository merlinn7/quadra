#include "quadrasoftware.h"
#include <windows.h>

quadrasoftware::quadrasoftware(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

quadrasoftware::~quadrasoftware()
{}

void quadrasoftware::on_tabButton1_clicked()
{
    ui.stackedWidget->setCurrentIndex(0);
}


void quadrasoftware::on_tabButton2_clicked()
{
    ui.stackedWidget->setCurrentIndex(1);
}
