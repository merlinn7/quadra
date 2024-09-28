#include "quadrasoftware.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    quadrasoftware w;
    w.show();
    return a.exec();
}
