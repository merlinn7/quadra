#include "quadrasoftware.h"
#include <windows.h>

quadrasoftware::quadrasoftware(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    ui.horizontalFrame->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76;  background-color:#686D76 ;}");
    ui.gridFrame->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76;  background-color:#686D76 ;}");
    ui.frame->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76;  background-color:#686D76 ;}");
    ui.frame_2->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76;  background-color:#686D76 ;}");
    ui.frame_3->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76; background-color:#686D76 ;}");
    ui.stackedWidget->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76; background-color:#686D76 ;}");
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
