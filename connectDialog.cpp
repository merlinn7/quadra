#include "connectDialog.h"
#include <windows.h>

connectDialog::connectDialog(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    QObject::connect(ui.okButton, &QPushButton::clicked, this, &QDialog::accept);
    QObject::connect(ui.cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void connectDialog::on_buttonBox_accepted()
{

}

void connectDialog::on_buttonBox_rejected()
{

}

connectDialog::~connectDialog()
{
}

QString connectDialog::getConnectionURL()
{
    return ui.lineEdit->text();
}
