#include "connectDialog.h"
#include <windows.h>

connectDialog::connectDialog(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    QObject::connect(ui.okButton, &QPushButton::clicked, this, &QDialog::accept);
    QObject::connect(ui.cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void connectDialog::on_comboBox_currentTextChanged(QString text)
{
    if (text == "UDP")
        ui.lineEdit->setText("udp://:14550");
    else if (text == "TCP")
        ui.lineEdit->setText("tcp://:14550");
    else if (text == "Serial")
        ui.lineEdit->setText("serial://11:57600");
}

connectDialog::~connectDialog()
{
}

QString connectDialog::getConnectionURL()
{
    return ui.lineEdit->text();
}
