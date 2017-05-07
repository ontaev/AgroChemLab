#ifndef TIPLIST_H
#define TIPLIST_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class TipList;
}

class TipList : public QDialog
{
    Q_OBJECT
    
public:
    explicit TipList(QWidget *parent = 0);
    ~TipList();
       
private slots:
    void on_addButton_clicked();

    void on_deleteButton_clicked();

private:
    Ui::TipList *ui;
    QSqlRelationalTableModel *model;
};

#endif // TIPLIST_H
