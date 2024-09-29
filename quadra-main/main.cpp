#include "quadrasoftware.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    quadrasoftware w;
    w.setStyleSheet("QMainWindow{background:#41444B;}");

    w.show();
    return a.exec();


}
