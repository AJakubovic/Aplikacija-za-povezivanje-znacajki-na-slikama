#ifndef MATCHES_H
#define MATCHES_H

#include <QDialog>

namespace Ui {
class Matches;
}

class Matches : public QDialog
{
    Q_OBJECT

public:
    explicit Matches(QWidget *parent = 0, QString text = nullptr);
    ~Matches();

private:
    Ui::Matches *ui;
};

#endif // MATCHES_H
