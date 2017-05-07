#ifndef OPENCARD_H
#define OPENCARD_H

#include <QDialog>
#include <QtGui>
#include <QtSql>
#include <QDataWidgetMapper>

namespace Ui {
class OpenCard;
}

class OpenCard : public QDialog
{
    Q_OBJECT

private slots:
    void submit();

public:
    explicit OpenCard(QWidget *parent = 0, int id = 0);
    ~OpenCard();

private:
    Ui::OpenCard *ui;
    QSqlRelationalTableModel *model;
    QDataWidgetMapper *mapper;
    int id;
};

#endif // OPENCARD_H
