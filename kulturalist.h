#ifndef KULTURALIST_H
#define KULTURALIST_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class KulturaList;
}

class KulturaList : public QDialog
{
    Q_OBJECT
    
public:
    explicit KulturaList(QWidget *parent = 0);
    ~KulturaList();
    
private slots:
    void on_addButton_clicked();

    void on_deleteButton_clicked();

private:
    Ui::KulturaList *ui;
    QSqlRelationalTableModel *model;
};

#endif // KULTURALIST_H
