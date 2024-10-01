#include "takeoffInputDialog.h"
#include <QIntValidator>

takeoffInputDialog::takeoffInputDialog(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    ui.lineEdit->setValidator(new QIntValidator(0, 100, this));
    ui.lineEdit->setFocus();

    QObject::connect(ui.okButton, &QPushButton::clicked, this, &QDialog::accept);
    QObject::connect(ui.cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

takeoffInputDialog::~takeoffInputDialog()
{
}

int takeoffInputDialog::GetAltitude()
{
    return ui.lineEdit->text().toInt();
}
