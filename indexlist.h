#ifndef INDEXLIST_H
#define INDEXLIST_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class IndexList;
}

class IndexList : public QDialog
{
    Q_OBJECT
    
public:
    explicit IndexList(QWidget *parent = 0);
    ~IndexList();
    
private slots:
    void on_addButton_clicked();

    void on_deleteButton_clicked();

private:
    Ui::IndexList *ui;
    QSqlRelationalTableModel *model;
};

#endif // INDEXLIST_H
