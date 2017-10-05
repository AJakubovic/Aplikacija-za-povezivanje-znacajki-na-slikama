#include "matches.h"
#include "ui_matches.h"

Matches::Matches(QWidget *parent, QString Text) :
    QDialog(parent),
    ui(new Ui::Matches)
{
    ui->setupUi(this);
    ui->plainTextEdit->setPlainText(Text);
    ui->plainTextEdit->setReadOnly(true);
}

Matches::~Matches()
{
    delete ui;
}
