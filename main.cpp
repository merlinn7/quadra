#include "quadrasoftware.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    quadrasoftware w;
    w.setStyleSheet("QMainWindow{border-radius: 12px; border: 1px solid #686D76; background-color:#808080 ;}");
    w.setMinimumSize(QSize(500, 500));
    w.resize(1500, 900);
    w.show();
    return a.exec();
}
